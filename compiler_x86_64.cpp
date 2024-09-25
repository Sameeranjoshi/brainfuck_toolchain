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

namespace fs = std::filesystem;

class Constants {
public:
    static const int SIZE_OF_TAPE = 1000;
    std::vector<std::string> input_files;

    Constants() {
        std::string directory = "benches/";
        for (const auto& entry : fs::directory_iterator(directory)) {
            if (entry.path().extension() == ".b") {
                input_files.push_back(entry.path().string());
            }
        }
    }
};

class Interpreter {
private:
    std::vector<uint8_t> tape;
    int tape_pointer;
    std::vector<char> preprocessed;
    std::unordered_map<int, std::string> loop_labels;
    bool _is_profiling_enabled;
    std::unordered_map<char, int> _profiling_data;
    std::map<std::pair<int, int>, int> loop_profiled_data;    // (start, end) -> count    - e.g. [] - 10
    int total_loops;

public:
    Interpreter(int tape_size, bool profiling) : tape(tape_size, 0), tape_pointer(tape_size / 2), _is_profiling_enabled(profiling), total_loops(0) {}

    void increment() {
        tape[tape_pointer]++;
    }

    void decrement() {
        tape[tape_pointer]--;
    }

    void move_left() {
        if (tape_pointer == 0) {
            tape.insert(tape.begin(), 1000, 0);
            tape_pointer += 1000;
        }
        tape_pointer--;
    }

    void move_right() {
        if (tape_pointer == tape.size() - 1) {
            tape.insert(tape.end(), 1000, 0);
        }
        tape_pointer++;
    }

    void write() {
        std::cout << static_cast<char>(tape[tape_pointer]);
    }

    void replace(uint8_t input_value) {
        tape[tape_pointer] = input_value;
    }

    bool is_currentcell_zero() {
        return tape[tape_pointer] == 0;
    }

    void preprocess_code() {
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
                // +"_start";
                loop_labels[idx] = ".L" + std::to_string(start);
                // +"_end";
            }
        }
        if (!stack.empty()) {
            throw std::runtime_error("Unmatched '[' found");
        }
    }

    bool is_simple_loop(int start, int end) {
        int net_pointer_change = 0;
        int net_cell_change = 0;
        for (int i = start + 1; i < end; ++i) {
            switch (preprocessed[i]) {
                case '>':
                    net_pointer_change++;
                    break;
                case '<':
                    net_pointer_change--;
                    break;
                case '+':
                    if (net_pointer_change == 0) net_cell_change++;
                    break;
                case '-':
                    if (net_pointer_change == 0) net_cell_change--;
                    break;
                case '.':
                case ',':
                    return false; // Contains I/O
                default:
                    break;
            }
        }
        return net_pointer_change == 0 && (net_cell_change == 1 || net_cell_change == -1);
    }
    void final_setup_assembly_structute(std::ofstream &assembly_file){

        // Restore registers
        assembly_file << "\n\n\n# Epilogue\n";
        assembly_file << "\tpopq %r12                 # Restore r12\n";
        assembly_file << "\tpopq %rbx                 # Restore rbx\n";
        assembly_file << "\tmovq %rbp, %rsp           # Restore stack pointer\n";
        assembly_file << "\tpopq %rbp                 # Restore base pointer\n";
        assembly_file << "\tret                       # Return to the kernel\n";
        // // Exit the program (using syscall)
        // assembly_file << "\tmovq $60, %rax            # syscall: exit\n";
        // assembly_file << "\txor %rdi, %rdi            # status: 0\n";
        // assembly_file << "\tsyscall                   # Call the kernel\n";
        
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


        // common string for all the assembly code
        // assembly_file << "# Set the current pointer to rax\n";
        // // assembly_file << "\tmovq %r12, %rax\n\n\n";
    }

    void interpret(const std::string& code, const std::string& filename, std::ofstream &assembly_file) {
        for (char eachword : code) {
            if (std::string("><+-.,[]").find(eachword) != std::string::npos) {
                preprocessed.push_back(eachword);
            }
        }
        preprocess_code();
        
        initial_setup_assembly_structure(assembly_file);

        size_t PC_index = 0;
        while (PC_index < preprocessed.size()) {
            char instruction = preprocessed[PC_index];
            // profiling
            if (_is_profiling_enabled)
                _profiling_data[instruction]++;

            switch (instruction) {
                case '>':
                    //move_right();
                    assembly_file << "\taddq $1, %r12\n";
                    break;
                case '<':
                    //move_left();
                    assembly_file << "\tsubq $1, %r12\n";
                    break;
                case '+':
                    //increment();
                    assembly_file << "\taddb $1, (%r12)\n";
                    break;
                case '-':
                    //decrement();
                    assembly_file << "\tsubb $1, (%r12)\n";
                    break;
                case '.':
                    //write();
                    assembly_file << "\tmovq %r12, %rax\n";
                    assembly_file << "\tmovb (%rax), %al\n";
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
                default:
                    std::cerr << "Failed to interpret code from file=" << filename
                              << ", at position = " << PC_index
                              << ": and at instruction = '" << instruction << "'" << std::endl;
                    exit(1);
            }

            PC_index++;
        }

        final_setup_assembly_structute(assembly_file);
        std::cout << "\nSuccessfully interpreted code from file=" << filename << std::endl;
    }

    void print_instr_count(std::ofstream &profiling_output) {
        profiling_output << "\nInstruction count:" << std::endl;
        for (const auto& [key, value] : _profiling_data) {
            profiling_output << key << ": " << value << std::endl;
        }
    }
    void print_simple_loops(std::ofstream &profiling_output) {
        std::vector<std::pair<std::pair<int, int>, int>> sorted_loops(loop_profiled_data.begin(), loop_profiled_data.end());
        std::sort(sorted_loops.begin(), sorted_loops.end(), [](const auto& a, const auto& b) {
            return a.second > b.second;
        });

        profiling_output << "\nSimple innermost loops profile data:" << std::endl;
        for (const auto& [loop, count] : sorted_loops) {
            profiling_output << "Loop [" << loop.first << ", " << loop.second << "] executed " << count << " times" << std::endl;
        }
        profiling_output << std::endl;

        int simple_loop_count = sorted_loops.size();
        profiling_output << "Total simple loops: " << simple_loop_count << std::endl;
        profiling_output << "Total loops: " << total_loops << std::endl;
        profiling_output << "Non simple loops: " << total_loops - simple_loop_count << std::endl;
    }
};

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <input_file> [-p]" << std::endl;
        return 1;
    }

    std::string input_file = argv[1];
    bool profiling = false;
    if (argc > 2 && std::string(argv[2]) == "-p") {
        profiling = true;
    }

    std::ofstream profiling_output("profiling_output.txt");
    if (!profiling_output) {
        std::cerr << "Failed to open profiling output file." << std::endl;
        return 1;
    }

    std::cout << "Running interpreter on input file = " << input_file << std::endl;

    Interpreter interpreter(Constants::SIZE_OF_TAPE, profiling);
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
    interpreter.interpret(code, input_file, assemblyfile);

    // profiling output
    if (profiling) {
        profiling_output << "\nProfiling data for file=" << input_file << std::endl;
        std::streambuf* cout_buf = std::cout.rdbuf(); // Save old buf
        std::cout.rdbuf(profiling_output.rdbuf()); // Redirect std::cout to profiling_output

        interpreter.print_instr_count(profiling_output);
        interpreter.print_simple_loops(profiling_output);

        std::cout.rdbuf(cout_buf); // Reset to standard output again
    }

    assemblyfile.close();

    return 0;
}
