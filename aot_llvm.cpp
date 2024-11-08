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
// #include <llvm/IR/Constants.h>
// #include <llvm/IR/Instructions.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/raw_ostream.h>
// #include <llvm/Target/TargetOptions.h>
// #include <llvm/Target/TargetMachine.h>
// #include <llvm/MC/TargetRegistry.h>


class Compiler {
private:
    std::vector<char> preprocessed;
    std::unordered_map<int, std::string> loop_labels;
    std::unordered_map<int, llvm::BasicBlock*> loop_bb_label;  // index, basic block

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

    llvm::Function* createPrintTesting(llvm::Module& module, llvm::LLVMContext& context, std::string &my_custom_string) {
        // Define the `printTesting` function signature
        llvm::FunctionType* printTestingType = llvm::FunctionType::get(llvm::Type::getVoidTy(context), {}, false);
        llvm::Function* printTestingFunc = llvm::Function::Create(printTestingType, llvm::Function::ExternalLinkage, "printTesting", module);

        // Create a new basic block for printTesting
        llvm::BasicBlock* entry = llvm::BasicBlock::Create(context, "entry", printTestingFunc);
        llvm::IRBuilder<> builder(entry);

        // Create a global string constant for "Testing LLVM\n"
        llvm::Value* testingStr = builder.CreateGlobalStringPtr(my_custom_string);
        // Call `printf` with the string as the argument
        llvm::FunctionType* printfType = llvm::FunctionType::get(llvm::Type::getInt32Ty(context), {llvm::Type::getInt8PtrTy(context)}, true);
        llvm::FunctionCallee printfFunc = module.getOrInsertFunction("printf", printfType);
        builder.CreateCall(printfFunc, {testingStr});

        // Return from the function
        builder.CreateRetVoid();

        return printTestingFunc;
    }

    void assign_loop_label(llvm::Module& module, llvm::LLVMContext& context, llvm::Function* MainFunc) {
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

                llvm::BasicBlock *loopStart = llvm::BasicBlock::Create(context, "loop_start" + std::to_string(start), MainFunc);
                llvm::BasicBlock *loopEnd = llvm::BasicBlock::Create(context, "loop_end" + std::to_string(start), MainFunc);

                loop_bb_label[start] = loopStart;
                loop_bb_label[idx] = loopEnd;
            }
        }
        if (!stack.empty()) {
            throw std::runtime_error("Unmatched '[' found");
        }
    }

    void gen_llvm(llvm::Module& module, llvm::LLVMContext& context, llvm::IRBuilder<>& builder, llvm::Function* MainFunc) {

    // tape allocation
    llvm::ArrayType *tape_type = llvm::ArrayType::get(builder.getInt8Ty(), 30000);
    llvm::Value *tape = builder.CreateAlloca(tape_type, nullptr, "tape");
    // init with 0 all values as CreateAlloca doesn't initialize
    builder.CreateMemSet(tape, builder.getInt8(0), 30000, llvm::MaybeAlign(1));

    // pointer to tape[0]
    llvm::Value *ptr = builder.CreateInBoundsGEP(
        tape_type, tape, {builder.getInt32(0), builder.getInt32(0)}, "ptr");
    llvm::Value *tape_ptr = builder.CreateAlloca(
        builder.getInt8Ty()->getPointerTo(), nullptr, "tape_ptr");
    // point to 0th location of tape. 
    builder.CreateStore(ptr, tape_ptr);

    // Declare external functions
    llvm::FunctionCallee putcharFunc = module.getOrInsertFunction("putchar", builder.getInt32Ty(), builder.getInt32Ty());
    llvm::FunctionCallee getcharFunc = module.getOrInsertFunction("getchar", builder.getInt32Ty());

    // from now on tape_ptr is the important.
    // tape_ptr is the pointer to the current memory location which is 0.    


        for (int PC_index = 0; PC_index < preprocessed.size(); ++PC_index) {
            char instruction = preprocessed[PC_index];
            //
            switch (instruction) {
                case '>':
                {
                    llvm::Value *currentTapePointer = builder.CreateLoad(builder.getInt8Ty()->getPointerTo(), tape_ptr, "load_tape_ptr");
                    llvm::Value *newTapePointer = builder.CreateInBoundsGEP(builder.getInt8Ty(), currentTapePointer, builder.getInt32(1), "inc_tape_ptr");
                    builder.CreateStore(newTapePointer, tape_ptr);
                }
                break;
                case '<':
                {
                    llvm::Value *currentTapePointer = builder.CreateLoad(builder.getInt8Ty()->getPointerTo(), tape_ptr, "load_tape_ptr");
                    llvm::Value *newTapePointer = builder.CreateInBoundsGEP(builder.getInt8Ty(), currentTapePointer, builder.getInt32(-1), "dec_tape_ptr");
                    builder.CreateStore(newTapePointer, tape_ptr);
                }
                break;
                case '+':
                {
                    llvm::Value *currentTapePointer = builder.CreateLoad(builder.getInt8Ty()->getPointerTo(), tape_ptr, "load_tape_ptr");
                    llvm::Value *currentValue = builder.CreateLoad(builder.getInt8Ty(), currentTapePointer, "load_tape_value");
                    llvm::Value *newValue = builder.CreateAdd(currentValue, builder.getInt8(1), "inc_tape_value");
                    builder.CreateStore(newValue, currentTapePointer);
                }
                break;
                case '-':
                {
                    llvm::Value *currentTapePointer = builder.CreateLoad(builder.getInt8Ty()->getPointerTo(), tape_ptr, "load_tape_ptr");
                    llvm::Value *currentValue = builder.CreateLoad(builder.getInt8Ty(), currentTapePointer, "load_tape_value");
                    llvm::Value *newValue = builder.CreateSub(currentValue, builder.getInt8(1), "dec_tape_value");
                    builder.CreateStore(newValue, currentTapePointer);
                }
                break;
                
                case '.': {
                    // Output the current cell value using putchar
                    llvm::Value *currentTapePointer = builder.CreateLoad(builder.getInt8Ty()->getPointerTo(), tape_ptr, "load_tape_ptr");
                    llvm::Value *currentValue = builder.CreateLoad(builder.getInt8Ty(), currentTapePointer, "load_tape_value");

                    // Cast i8 to i32 since putchar expects an int argument
                    llvm::Value *currentValueAsInt32 = builder.CreateSExt(currentValue, builder.getInt32Ty(), "sext_value");
                    builder.CreateCall(putcharFunc, currentValueAsInt32);
                } break;

                case ',': {
                    // Read a character using getchar and store it in the current cell
                    llvm::Value *currentTapePointer = builder.CreateLoad(builder.getInt8Ty()->getPointerTo(), tape_ptr, "load_tape_ptr");

                    // Call getchar and store the result in the current tape cell
                    llvm::Value *inputChar = builder.CreateCall(getcharFunc, {}, "input_char");

                    // Truncate i32 to i8 to store in the tape
                    llvm::Value *inputCharAsInt8 = builder.CreateTrunc(inputChar, builder.getInt8Ty(), "trunc_input");
                    builder.CreateStore(inputCharAsInt8, currentTapePointer);
                } break;
                // case '[':
                // {
                //     llvm::BasicBlock* loopStart = loop_bb_label[PC_index];
                //     llvm::BasicBlock* loopEnd = loop_bb_label[PC_index + 1];  // assuming it matches ']'
                    
                //     llvm::Value* equalToZero = builder.CreateICmpEQ(builder.CreateLoad(builder.getInt8Ty(), builder.CreateLoad(builder.getInt8Ty()->getPointerTo(), tape_ptr, "load_tape_ptr"), "load_tape_value"),builder.getInt8(0), "zeroCheck");
                //     builder.CreateCondBr(equalToZero, loopEnd, loopStart);  // Loop end if zero
                //     builder.SetInsertPoint(loopStart);
                // }
                // break;
                // case ']':
                // {
                //     llvm::BasicBlock* loopStart = loop_bb_label[PC_index];
                //     llvm::BasicBlock* loopEnd = loop_bb_label[PC_index + 1];  // assuming it matches ']'
                //     // load value from tape pointer

                //     // pass value to compare 
                //     llvm::Value* notEqualToZero = builder.CreateICmpNE(builder.CreateLoad(builder.getInt8Ty(), builder.CreateLoad(builder.getInt8Ty()->getPointerTo(), tape_ptr, "load_tape_ptr"), "load_tape_value"),builder.getInt8(0), "CheckNotZero");
                //     // create BB
                //     builder.CreateCondBr(notEqualToZero, loopStart, loopEnd);  // Loop end if zero
                //     builder.SetInsertPoint(loopEnd);
                // }

        //         case '[':
        //                 {
        //                     llvm::BasicBlock* loopStart = loop_bb_label[PC_index];
        //                     llvm::BasicBlock* loopEnd = loop_bb_label[PC_index + 1];  // assuming it matches ']'
                            
        //                     llvm::Value* equalToZero = builder.CreateICmpEQ(builder.CreateLoad(/*pointer to memory*/),llvm::ConstantInt::get(context, llvm::APInt(8, 0)), "zeroCheck");
        //                     builder.CreateCondBr(equalToZero, loopEnd, loopStart);  // Loop end if zero
        //                     builder.SetInsertPoint(loopStart);
        //                 }
        //         break;
        //         case ']':
        //         {
        //                     llvm::BasicBlock* loopStart = loop_bb_label[PC_index];
        //                     llvm::BasicBlock* loopEnd = loop_bb_label[PC_index + 1];  // assuming it matches ']'
        //                     // load value from tape pointer

        //                     // pass value to compare 
        //                     llvm::Value* notEqualToZero = builder.CreateICmpNE(builder.CreateLoad(/*pointer to memory*/),llvm::ConstantInt::get(context, llvm::APInt(8, 0)), "CheckNotZero");
        //                     // create BB
        //                     builder.CreateCondBr(notEqualToZero, loopStart, loopEnd);  // Loop end if zero
        //                     builder.SetInsertPoint(loopEnd);
        //         }
        //             break;
        //     //     default:
        //     //         std::cerr << "Failed to compile code from file=" << filename
        //     //                   << ", at position = " << PC_index
        //     //                   << ": and at instruction = '" << instruction << "'" << std::endl;
        //     //         exit(1);
            }
        }
    }
    
    llvm::Function* createMainFunction(llvm::Module& module, llvm::LLVMContext& context) {
        // Define the `main` function signature
        llvm::FunctionType* mainType = llvm::FunctionType::get(llvm::Type::getInt32Ty(context), {}, false);
        llvm::Function* mainFunc = llvm::Function::Create(mainType, llvm::Function::ExternalLinkage, "main", module);

        // Create a new basic block for main
        llvm::BasicBlock* entry = llvm::BasicBlock::Create(context, "entry", mainFunc);
        llvm::IRBuilder<> builder(entry);

        // // Call printTesting function
        // llvm::Function* printTestingFunc = module.getFunction("printTesting");
        // builder.CreateCall(printTestingFunc);

        // iterate on all instructions
        // assign_loop_label(module, context, mainFunc);
        gen_llvm(module, context, builder, mainFunc);

        // Return a simple integer from main
        builder.CreateRet(llvm::ConstantInt::get(context, llvm::APInt(32, 0)));

        return mainFunc;
    }

    void preprocess(const std::string& code) {
        for (char eachword : code) {
            if (std::string("><+-.,[]").find(eachword) != std::string::npos) {
                preprocessed.push_back(eachword);
            }
        }
    }

    void lower_to_llvm(const std::string& code, const std::string& filename, llvm::Module& module, llvm::LLVMContext& context) {
        preprocess(code);
        // createPrintTesting(module, context);
        createMainFunction(module, context);

        // gen_assembly(assembly_file, filename);
        // if (retval == true) {
            // std::cout << "\nSuccessfully compiled code from file=" << filename << std::endl;
        //  }
    }
};

void post_instr_lowering(llvm::Module& module, llvm::LLVMContext& context) {

    // rest of code 
    
    // setup target
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    auto targetTriple = llvm::sys::getDefaultTargetTriple();
    module.setTargetTriple(targetTriple);

    // // verify module
    auto res = llvm::verifyModule(module, &llvm::errs());
    assert(!res);

    // TODO: optimize

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

    // input file
    std::string input_file = argv[1];
    std::ifstream file(input_file);
    if (!file) {
        std::cerr << "Failed to open file: " << input_file << std::endl;
        return 1;
    }
    // parsed code
    std::string code((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    
    // llvm module file
    llvm::LLVMContext context;
    llvm::Module module("BF_module", context);

    Compiler compiler;
    compiler.lower_to_llvm(code, input_file, module, context);

    post_instr_lowering(module, context);
    return 0;
}
