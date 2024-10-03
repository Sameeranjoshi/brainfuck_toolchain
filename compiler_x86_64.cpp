#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <string>
#include <filesystem>
#include <stdexcept>
#include <cstdint>
#include <algorithm>
#include <map>
#include <chrono>

class Compiler {
private:
    std::vector<char> preprocessed;
    std::unordered_map<int, std::string> loop_labels;
    std::unordered_map<int, std::unordered_map<int, int>> dollar_to_offsetcount;
    bool optimize_simple_loops;
    bool optimize_memory_scans;

    bool is_simple_loop(size_t start, size_t end) { // [...] contains brackets
        std::unordered_map<int, int> offset_count;
        int pointer_movement = 0;
        int net_change = 0;
        for (size_t i = start + 1; i < end; ++i) {  // body of loop
            char command = preprocessed[i];
            switch (command) {
                case '>': pointer_movement++; break;
                case '<': pointer_movement--; break;
                case '+': offset_count[pointer_movement]++; break;
                case '-': offset_count[pointer_movement]--; break;
                // if [+$] or even [+0] this is not a simple loop
                case '[': case ']': case '.': case ',': case '$': case '0': return false; // Nested loops or I/O
            }
        }
        return pointer_movement == 0 && (offset_count[0] == 1 || offset_count[0] == -1);
    }

    std::unordered_map<int, int> getoffset_count_map(size_t start, size_t end) {
        std::unordered_map<int, int> offset_count;
        // special case for single command loops    // [+] or [-]
        // if (end - start == 2){
        //     offset_count[0] = 0;
        //     return offset_count;
        // }
        int pointer_movement = 0;
        for (size_t i = start + 1; i < end; ++i) {
            char command = preprocessed[i];
            switch (command) {
                case '>': pointer_movement++; break;
                case '<': pointer_movement--; break;
                case '+': offset_count[pointer_movement]++; break;
                case '-': offset_count[pointer_movement]--; break;
            }
        }
        return offset_count;
    }

    void rewrite_$_for_loop(size_t start, size_t end, std::unordered_map<int, int>& offset_count_map) {
        // use the formula start + offset * offset_count to optimize the loop and dump assembly
                                // print some debugging code to make sure everything is working
        
        // print start and end
        // std::cout << "start " << start << "end "   << end << "\n";
        // replace [++] -> $000
        for (size_t i = start; i <= end; i++)
        {
            if (i == start){
                preprocessed[i] = '$';
            } else {
                preprocessed[i] = '0';
            }
        }
        dollar_to_offsetcount[start] = offset_count_map;
    }

public:
    Compiler(bool optimize_simple_loops, bool optimize_memory_scans)
        : optimize_simple_loops(optimize_simple_loops), optimize_memory_scans(optimize_memory_scans) {}

    void assign_loop_label() {
        std::vector<int> stack;
        for (size_t idx = 0; idx < preprocessed.size(); ++idx) {
            char command = preprocessed[idx];
            if (command == '[') {
                stack.push_back(idx);
            } else if (command == ']') {
                if (stack.empty()) {
                    throw std::runtime_error("Unmatched ']' found");
                }
                int start = stack.back();
                stack.pop_back();
                loop_labels[start] = ".L" + std::to_string(start);
                loop_labels[idx] = ".L" + std::to_string(start);
            }
        }
        if (!stack.empty()) {
            throw std::runtime_error("Unmatched '[' found");
        }
    }

    void final_setup_assembly_structute(std::ofstream &assembly_file) {
        // Restore registers
        assembly_file << "\n\n\n# Epilogue\n";
        assembly_file << "\tpopq %r12                 # Restore r12\n";
        assembly_file << "\tpopq %rbx                 # Restore rbx\n";
        assembly_file << "\tmovq %rbp, %rsp           # Restore stack pointer\n";
        assembly_file << "\tpopq %rbp                 # Restore base pointer\n";
        assembly_file << "\tret                       # Return to the kernel\n";
    }

    void initial_setup_assembly_structure(std::ofstream &assembly_file) {
        // Text section
        assembly_file << ".section .text\n";
        assembly_file << ".globl main\n\n";

        assembly_file << "main:\n";

        assembly_file << "\t.extern malloc\n";
        assembly_file << "\t.extern memset\n";
        assembly_file << "\t.extern putchar\n";

        // Save frame pointer and link register onto stack
        assembly_file << "# Prologue\n";
        assembly_file << "\tpushq %rbp                # Save the base pointer\n";
        assembly_file << "\tmovq %rsp, %rbp          # Set base pointer to stack pointer\n";

        // Save rbx and r12 (used as general-purpose registers)
        assembly_file << "\tpushq %rbx                # Save rbx\n";
        assembly_file << "\tpushq %r12                # Save r12\n";

        // Allocate data array using malloc
        assembly_file << "# Memory allocation call\n";
        assembly_file << "\tmovq $30000, %rdi         # Allocate 30,000 bytes\n";
        assembly_file << "\tcall malloc                # Call malloc function\n";

        // Store data pointer for tape in r12
        assembly_file << "# Save the return pointer\n";
        assembly_file << "\tmovq %rax, %r12           # Store returned pointer in r12\n";
        // Store original pointer allocated in rbx
        assembly_file << "\tmovq %rax, %rbx           # Store original pointer in rbx\n";

        // zero the tape
        assembly_file << "# Zero out the allocated memory\n";
        assembly_file << "\tmovq %r12, %rdi           # Destination pointer\n";
        assembly_file << "\tmovq $0, %rsi              # Value to set (zero)\n";
        assembly_file << "\tmovq $30000, %rdx         # Number of bytes\n";
        assembly_file << "\tcall memset                # Call memset function\n";
    }

    void gen_assembly(std::ofstream &assembly_file, const std::string& filename) {

        for (int PC_index = 0; PC_index < preprocessed.size(); ++PC_index) {
            char instruction = preprocessed[PC_index];

            switch (instruction) {
                case '>':
                    assembly_file << "\taddq $1, %r12\n";
                    break;
                case '<':
                    assembly_file << "\tsubq $1, %r12\n";
                    break;
                case '+':
                    assembly_file << "\taddb $1, (%r12)\n";
                    break;
                case '-':
                    assembly_file << "\tsubb $1, (%r12)\n";
                    break;
                case '.':
                    assembly_file << "\tmovb (%r12), %al\n";
                    assembly_file << "\tmovzbl %al, %edi\n";
                    assembly_file << "\tcall putchar\n";
                    break;
                case ',':
                    assembly_file << "\tcall getchar\n";
                    assembly_file << "\tmovb %al, (%r12)\n";
                    break;
                case '[':
                    assembly_file << "#LOOP \n";
                    assembly_file << loop_labels[PC_index] + "_start" << ":\n";
                    assembly_file << "\tcmpb $0, (%r12)\n";
                    assembly_file << "\tje " << loop_labels[PC_index] + "_end\n\n";
                    break;
                case ']':
                    assembly_file << "\n\tjmp " << loop_labels[PC_index] + "_start" << "\n";
                    assembly_file << loop_labels[PC_index] + "_end" << ":\n";
                    assembly_file << "#LOOP END\n";
                    break;
                case '$':
                    assembly_file << "# Optimized loop\n";
                    // using dollar_to_offsetcount write assembly to do maths
                    for (auto& [offset, count] : dollar_to_offsetcount[PC_index]) {
                        // skip the value with offset == 0
                        if (offset == 0) {
                            continue;
                        }
                        // p[base+offset] = p[base+offset] + count*p[base]
                        // %r12 = base
                        // (%r12) = p[base]
                        // generate assembly for p[base+offset] = p[base+offset] + count*p[base]
                        assembly_file << "\tmovb " << offset << "(%r12), %al\n";
                        assembly_file << "\tmovzbl %al, %edi\n";
                        assembly_file << "\tmovb (%r12), %al\n";
                        assembly_file << "\tmovzbl %al, %esi\n";
                        assembly_file << "\timull $" << count << ", %esi\n";
                        assembly_file << "\taddl %esi, %edi\n";
                        assembly_file << "\tmovb %dil, " << offset << "(%r12)\n";
                        // Where is the addition at the end? It is done in the next iteration
                    }
                    // for offset=0 make // make p[offset] = 0
                    assembly_file << "\tmovb $0, (%r12)\n";
                    assembly_file << "# Optimized loop end\n";
                    break;
                case '0':
                    // this is a special case when the loop is optimized into some commands using the 
                    break;
                default:
                    std::cerr << "Failed to compile code from file=" << filename
                              << ", at position = " << PC_index
                              << ": and at instruction = '" << instruction << "'" << std::endl;
                    exit(1);
            }
        }
    }

    void optimize(std::ofstream &assembly_file, const std::string& filename) {
        // implement simple_loop_optimization here.
        if (optimize_simple_loops) {
            std::vector<int> stack;
            for (size_t idx = 0; idx < preprocessed.size(); ++idx) {
                char command = preprocessed[idx];
                if (command == '[') {
                    stack.push_back(idx);
                } else if (command == ']') {
                    if (stack.empty()) {
                        throw std::runtime_error("Unmatched ']' found");
                    }
                    int start = stack.back();
                    stack.pop_back();
                    if (is_simple_loop(start, idx)) {
                        auto offset_count_map = getoffset_count_map(start, idx);
                        rewrite_$_for_loop(start, idx, offset_count_map);
                    }
                }
            }
            if (!stack.empty()) {
                throw std::runtime_error("Unmatched '[' found");
            }
        }
    }

    void compile(const std::string& code, const std::string& filename, std::ofstream &assembly_file) {
        for (char eachword : code) {
            if (std::string("><+-.,[]").find(eachword) != std::string::npos) {
                preprocessed.push_back(eachword);
            }
        }
        optimize(assembly_file, filename);
        assign_loop_label();
        initial_setup_assembly_structure(assembly_file);
        gen_assembly(assembly_file, filename);
        final_setup_assembly_structute(assembly_file);

        std::cout << "\nSuccessfully compiled code from file=" << filename << std::endl;
    }
};

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <input_file> [--optimize-simple-loops] [--optimize-memory-scans]" << std::endl;
        return 1;
    }

    std::string input_file = argv[1];
    bool optimize_simple_loops = false;
    bool optimize_memory_scans = false;

    for (int i = 2; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--optimize-simple-loops") {
            optimize_simple_loops = true;
        } else if (arg == "--optimize-memory-scans") {
            optimize_memory_scans = true;
        } else{
            std::cerr << "Unknown command line argument: " << arg << std::endl;
            std::cerr << "Usage: " << argv[0] << " <input_file> [--optimize-simple-loops] [--optimize-memory-scans]" << std::endl;
            return 1;
        }
    }

    std::cout << "Running compiler on input file = " << input_file << std::endl;

    Compiler compiler(optimize_simple_loops, optimize_memory_scans);
    std::ifstream file(input_file);
    if (!file) {
        std::cerr << "Failed to open file: " << input_file << std::endl;
        return 1;
    }
    std::ofstream assemblyfile("assembly_output.s");
    if (!assemblyfile) {
        std::cerr << "Failed to open assembly output file." << std::endl;
        return 1;
    }

    std::string code((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    auto start = std::chrono::high_resolution_clock::now();
    compiler.compile(code, input_file, assemblyfile);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;

    std::cout << "Compilation time: " << elapsed.count() << " seconds" << std::endl;

    assemblyfile.close();

    return 0;
}
