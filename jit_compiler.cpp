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
#include <stack>
#include <cstddef>
#include <cstdint>
#include <vector>
#include <cassert>
#include <cstring>
#include <limits>
#include <sys/mman.h>


constexpr int MEMORY_SIZE = 30000;


class JitProgramMemory {
public:

  JitProgramMemory(const std::vector<uint8_t>& code) {
    program_size_ = code.size();
    // allocate memory that is writable and executable
    void *ptr = mmap(0, program_size_, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (ptr == (void*)-1) {
      perror("mmap");
      std::cerr << "unable to allocate writable memory";
    }
    else 
      program_memory_ = ptr;
      
    memcpy(program_memory_, code.data(), program_size_);
    // mark memory as executable only
    if (mprotect(program_memory_, program_size_, PROT_READ | PROT_EXEC) == -1) {
      perror("mprotect");
      std::cerr << "unable to mark memory as executable";
    }
  }

  ~JitProgramMemory() {
    if (program_memory_ != nullptr) {
      if (munmap(program_memory_, program_size_) < 0) {
        perror("munmap");
        std::cerr << "unable to unmap memory";
      }
    }
  }

  void* program_memory() {
    return program_memory_;
  }

  size_t program_size() {
    return program_size_;
  }

private:
  void* program_memory_ = nullptr;
  size_t program_size_ = 0;
};

class CodeEmitter {
public:

  CodeEmitter() = default;
  CodeEmitter(bool debug_print_machine_code) : debug_print_machine_code(debug_print_machine_code) {}

  void EmitByte(uint8_t v) {
    code_.push_back(v);
    if (debug_print_machine_code)
      std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(v) << " ";
  }

  // Emits a sequence of consecutive bytes.
  void EmitBytes(std::initializer_list<uint8_t> seq) {
    for (auto v : seq) {
      EmitByte(v);
    }
    if (debug_print_machine_code)
      std::cout << std::endl;
  }
  
  // Replaces the byte at 'offset' with 'v'. Assumes offset < size().
  void ReplaceByteAtOffset(size_t offset, uint8_t v) {
    assert(offset < code_.size() && "replacement fits in code");
    code_[offset] = v;
  }
  
  // Replaces the 32-bit word at 'offset' with 'v'. Assumes offset + 3 < size().
  void ReplaceUint32AtOffset(size_t offset, uint32_t v) {
    ReplaceByteAtOffset(offset, v & 0xFF);
    ReplaceByteAtOffset(offset + 1, (v >> 8) & 0xFF);
    ReplaceByteAtOffset(offset + 2, (v >> 16) & 0xFF);
    ReplaceByteAtOffset(offset + 3, (v >> 24) & 0xFF);
  }

  void EmitUint32(uint32_t v) {
    EmitByte(v & 0xFF);
    EmitByte((v >> 8) & 0xFF);
    EmitByte((v >> 16) & 0xFF);
    EmitByte((v >> 24) & 0xFF);
  }

  void EmitUint64(uint64_t v) {
    EmitUint32(v & 0xFFFFFFFF);
    EmitUint32((v >> 32) & 0xFFFFFFFF);
  }

  size_t size() const {
    return code_.size();
  }

  const std::vector<uint8_t>& code() const {
    return code_;
  }

private:
  std::vector<uint8_t> code_;
  bool debug_print_machine_code = false;
};



uint32_t compute_relative_32bit_offset(size_t jump_from, size_t jump_to) {
  if (jump_to >= jump_from) {
    size_t diff = jump_to - jump_from;
    assert(diff < (1ull << 31));
    return diff;
  } else {
    // Here the diff is negative, so we need to encode it as 2s complement.
    size_t diff = jump_from - jump_to;
    assert(diff - 1 < (1ull << 31));
    uint32_t diff_unsigned = static_cast<uint32_t>(diff);
    return ~diff_unsigned + 1;
  }
}




class Compiler {
private:
    std::vector<char> preprocessed;
    // r13  == r12 from previous compiler
    CodeEmitter emitter{debug_print_machine_code};
    std::vector<uint8_t> memory;
    bool debug_print_machine_code = false;


public:
    Compiler(bool debug_print = false) : memory(MEMORY_SIZE, 0), debug_print_machine_code(debug_print), emitter(debug_print) {
        // Does nothing
    }

    void final_setup_assembly_structute() {
        emitter.EmitByte(0xC3);
    }

    void initial_setup_assembly_structure() {
        // movabs <address of memory.data>, %r13
        emitter.EmitBytes({0x49, 0xBD});
        emitter.EmitUint64((uint64_t)memory.data());
    }

    void gen_assembly(const std::string& filename) {

      std::vector<int> stack;
        for (int PC_index = 0; PC_index < preprocessed.size(); ++PC_index) {
            char instruction = preprocessed[PC_index];

            switch (instruction) {
                case '>':
                    // inc %r13
                    emitter.EmitBytes({0x49, 0xFF, 0xC5});
                    break;
                case '<':
                    emitter.EmitBytes({0x49, 0xFF, 0xCD});
                    break;
                case '+':
                    emitter.EmitBytes({0x41, 0x80, 0x45, 0x00, 0x01});
                    break;
                case '-':
                    emitter.EmitBytes({0x41, 0x80, 0x6D, 0x00, 0x01});
                    break;
                case '.':
                    emitter.EmitBytes({0x48, 0xC7, 0xC0, 0x01, 0x00, 0x00, 0x00});
                    emitter.EmitBytes({0x48, 0xC7, 0xC7, 0x01, 0x00, 0x00, 0x00});
                    emitter.EmitBytes({0x4C, 0x89, 0xEE});
                    emitter.EmitBytes({0x48, 0xC7, 0xC2, 0x01, 0x00, 0x00, 0x00});
                    emitter.EmitBytes({0x0F, 0x05});
                    break;
                case ',':
                    
                    emitter.EmitBytes({0x48, 0xC7, 0xC0, 0x00, 0x00, 0x00, 0x00});
                    emitter.EmitBytes({0x48, 0xC7, 0xC7, 0x00, 0x00, 0x00, 0x00});
                    emitter.EmitBytes({0x4C, 0x89, 0xEE});
                    emitter.EmitBytes({0x48, 0xC7, 0xC2, 0x01, 0x00, 0x00, 0x00});
                    emitter.EmitBytes({0x0F, 0x05});
                    break;
                case '[':
                    // cmpb $0, 0(%r13)
                    emitter.EmitBytes({0x41, 0x80, 0x7d, 0x00, 0x00});
                    // jz <offset>
                    stack.push_back(emitter.size());
                    emitter.EmitBytes({0x0F, 0x84});
                    emitter.EmitUint32(0);
                    break;               
                case ']':
                {
                    if (stack.empty()) {
                      throw std::runtime_error("Unmatched ']' found");
                    }
                    int start = stack.back();
                    stack.pop_back();

                    // cmpb $0, 0(%r13)
                    emitter.EmitBytes({0x41, 0x80, 0x7d, 0x00, 0x00});

                    size_t jump_back_from = emitter.size() + 6;
                    size_t jump_back_to = start + 6;
                    uint32_t pcrel_offset_back =
                        compute_relative_32bit_offset(jump_back_from, jump_back_to);

                    // jnz <open_bracket_location>
                    emitter.EmitBytes({0x0F, 0x85});
                    emitter.EmitUint32(pcrel_offset_back);

                    size_t jump_forward_from = start + 6;
                    size_t jump_forward_to = emitter.size();
                    uint32_t pcrel_offset_forward =
                        compute_relative_32bit_offset(jump_forward_from, jump_forward_to);
                    emitter.ReplaceUint32AtOffset(start + 2,
                                                  pcrel_offset_forward);
                    break;
                }
                default:
                    std::cerr << "Failed to JIT compile code from file=" << filename
                              << ", at position = " << PC_index
                              << ": and at instruction = '" << instruction << "'" << std::endl;
                    exit(1);
            }
        }
        if (!stack.empty()) {
          throw std::runtime_error("Unmatched '[' found");
        }
    }
    
    void preprocessing_code(const std::string& code){
        for (char eachword : code) {
            if (std::string("><+-.,[]").find(eachword) != std::string::npos) {
                preprocessed.push_back(eachword);
            }
        }
    }

    void compile(const std::string& code, const std::string& filename) {
        preprocessing_code(code);
        initial_setup_assembly_structure();
        gen_assembly(filename);
        final_setup_assembly_structute();

        // JITTING setup
        std::vector<uint8_t> emitted_code = emitter.code();
        JitProgramMemory jit_program(emitted_code);
        using JittedFunc = void (*)(void);
        JittedFunc func = (JittedFunc)jit_program.program_memory();
        func();

    }
};

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <input_file> [--machine_code]" << std::endl;
        return 1;
    }
    std::string input_file = argv[1];
    bool machine_code = (argc > 2 && std::string(argv[2]) == "--machine_code");

    std::cout << "Running JIT on input file = " << input_file << std::endl;

    Compiler compiler(machine_code);
    std::ifstream file(input_file);
    if (!file) {
        std::cerr << "Failed to open file: " << input_file << std::endl;
        return 1;
    }

    std::string code((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    compiler.compile(code, input_file);

    return 0;
}
