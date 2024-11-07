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
// LLVM headers
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Instructions.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Support/FileSystem.h>


class Compiler {
private:
    std::vector<char> preprocessed;
    std::unordered_map<int, std::string> loop_labels;

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

public:
    Compiler(){}

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
                default:
                    std::cerr << "Failed to compile code from file=" << filename
                              << ", at position = " << PC_index
                              << ": and at instruction = '" << instruction << "'" << std::endl;
                    exit(1);
            }
        }
    }

    void compile(const std::string& code, const std::string& filename, std::ofstream &assembly_file) {
        for (char eachword : code) {
            if (std::string("><+-.,[]").find(eachword) != std::string::npos) {
                preprocessed.push_back(eachword);
            }
        }
        assign_loop_label();
        initial_setup_assembly_structure(assembly_file);
        gen_assembly(assembly_file, filename);
        final_setup_assembly_structute(assembly_file);

        // std::cout << "\nSuccessfully compiled code from file=" << filename << std::endl;
    }
};

llvm::Function* createMainFunction(llvm::Module& module, llvm::LLVMContext& context) {
    // Define the `main` function signature
    llvm::FunctionType* mainType = llvm::FunctionType::get(
        llvm::Type::getInt32Ty(context), {}, false);
    llvm::Function* mainFunc = llvm::Function::Create(
        mainType, llvm::Function::ExternalLinkage, "main", module);

    // Create a new basic block for main
    llvm::BasicBlock* entry = llvm::BasicBlock::Create(context, "entry", mainFunc);
    llvm::IRBuilder<> builder(entry);

    // Return a simple integer from main
    builder.CreateRet(llvm::ConstantInt::get(context, llvm::APInt(32, 0)));

    return mainFunc;
}

void llvm_apis(){
    llvm::LLVMContext context;
    llvm::Module module("my_module", context);

    // Create a main function
    createMainFunction(module, context);

    // The rest of your setup code
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    auto targetTriple = llvm::sys::getDefaultTargetTriple();
    module.setTargetTriple(targetTriple);

    // Write the module to output.ll
    std::error_code EC;
    llvm::raw_fd_ostream dest("output.ll", EC, llvm::sys::fs::OF_None);
    module.print(dest, nullptr);

}



int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <input_file> " << std::endl;
        return 1;
    }

    std::string input_file = argv[1];
    Compiler compiler;
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

    compiler.compile(code, input_file, assemblyfile);
    
    assemblyfile.close();

    llvm_apis();
    return 0;
}
