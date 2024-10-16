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

constexpr int MEMORY_SIZE = 30000;

// Represents a JITed program in memory. Create it with a vector of code
// encoded as a binary sequence.
//
// The constructor maps memory with proper permissions and copies the
// code into it. The pointer returned by program_memory() then points to
// the code in executable memory. When JitProgram dies, it automatically
// cleans up the memory it mapped.
class JitProgram {
public:
  JitProgram(const std::vector<uint8_t>& code);
  ~JitProgram();

  // Get the pointer to program memory. This pointer is valid only as long as
  // the JitProgram object is alive.
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

// Helps emit a binary stream of code into a buffer. Entities larger than 8 bits
// are emitted in little endian.
class CodeEmitter {
public:
  CodeEmitter() = default;
  CodeEmitter(bool debug_print_machine_code) : debug_print_machine_code(debug_print_machine_code) {}

  void EmitByte(uint8_t v);

  // Emits a sequence of consecutive bytes.
  void EmitBytes(std::initializer_list<uint8_t> seq);

  void EmitUint32(uint32_t v);
  void EmitUint64(uint64_t v);

  // Replaces the byte at 'offset' with 'v'. Assumes offset < size().
  void ReplaceByteAtOffset(size_t offset, uint8_t v);

  // Replaces the 32-bit word at 'offset' with 'v'. Assumes offset + 3 < size().
  void ReplaceUint32AtOffset(size_t offset, uint32_t v);

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

// Utilities for writing a JIT.
//
// Note: the implementation is POSIX-specific, requiring mmap/munmap with
// appropriate flags.
//
// Eli Bendersky [http://eli.thegreenplace.net]
// This code is in the public domain.
#include <cassert>
#include <cstring>
#include <limits>
#include <sys/mman.h>

// Allocates RW memory of given size and returns a pointer to it. On failure,
// prints out the error and returns nullptr. mmap is used to allocate, so
// deallocation has to be done with munmap, and the memory is allocated
// on a page boundary so it's suitable for calling mprotect.
void* alloc_writable_memory(size_t size) {
  void* ptr =
      mmap(0, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (ptr == (void*)-1) {
    perror("mmap");
    return nullptr;
  }
  return ptr;
}

// Sets a RX permission on the given memory, which must be page-aligned. Returns
// 0 on success. On failure, prints out the error and returns -1.
int make_memory_executable(void* m, size_t size) {
  if (mprotect(m, size, PROT_READ | PROT_EXEC) == -1) {
    perror("mprotect");
    return -1;
  }
  return 0;
}

JitProgram::JitProgram(const std::vector<uint8_t>& code) {
  program_size_ = code.size();
  program_memory_ = alloc_writable_memory(program_size_);
  if (program_memory_ == nullptr) {
    std::cerr << "unable to allocate writable memory";
  }
  memcpy(program_memory_, code.data(), program_size_);
  if (make_memory_executable(program_memory_, program_size_) < 0) {
    std::cerr << "unable to mark memory as executable";
  }
}

JitProgram::~JitProgram() {
  if (program_memory_ != nullptr) {
    if (munmap(program_memory_, program_size_) < 0) {
      perror("munmap");
      std::cerr << "unable to unmap memory";
    }
  }
}

void CodeEmitter::EmitByte(uint8_t v) {
  code_.push_back(v);
  if (debug_print_machine_code)
    std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(v) << " ";
}

void CodeEmitter::EmitBytes(std::initializer_list<uint8_t> seq) {
  for (auto v : seq) {
    EmitByte(v);
  }
  if (debug_print_machine_code)
    std::cout << std::endl;
}

void CodeEmitter::ReplaceByteAtOffset(size_t offset, uint8_t v) {
  assert(offset < code_.size() && "replacement fits in code");
  code_[offset] = v;
}

void CodeEmitter::ReplaceUint32AtOffset(size_t offset, uint32_t v) {
  ReplaceByteAtOffset(offset, v & 0xFF);
  ReplaceByteAtOffset(offset + 1, (v >> 8) & 0xFF);
  ReplaceByteAtOffset(offset + 2, (v >> 16) & 0xFF);
  ReplaceByteAtOffset(offset + 3, (v >> 24) & 0xFF);
}

void CodeEmitter::EmitUint32(uint32_t v) {
  EmitByte(v & 0xFF);
  EmitByte((v >> 8) & 0xFF);
  EmitByte((v >> 16) & 0xFF);
  EmitByte((v >> 24) & 0xFF);
}

void CodeEmitter::EmitUint64(uint64_t v) {
  EmitUint32(v & 0xFFFFFFFF);
  EmitUint32((v >> 32) & 0xFFFFFFFF);
}

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
    std::unordered_map<int, std::string> loop_labels;
  // Registers used in the program:
  //
  // r13: the data pointer -- contains the address of memory.data()
  //
  // rax, rdi, rsi, rdx: used for making system calls, per the ABI.
    CodeEmitter emitter{debug_print_machine_code};
    std::vector<uint8_t> memory;
    bool debug_print_machine_code = false;
    std::stack<size_t> open_bracket_stack;


public:
    Compiler(bool debug_print = false) : memory(MEMORY_SIZE, 0), debug_print_machine_code(debug_print), emitter(debug_print) {
        // Does nothing
    }
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

    void final_setup_assembly_structute() {
        emitter.EmitByte(0xC3);
    }

    void initial_setup_assembly_structure() {

        // movabs <address of memory.data>, %r13
        emitter.EmitBytes({0x49, 0xBD});
        emitter.EmitUint64((uint64_t)memory.data());
    }

    void gen_assembly(const std::string& filename) {

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
                                                        // To read one byte from stdin, call the read syscall with fd=0 (for
                                      // stdin),
                                      // buf=address of byte, count=1.
                                      emitter.EmitBytes({0x48, 0xC7, 0xC0, 0x00, 0x00, 0x00, 0x00});
                                      emitter.EmitBytes({0x48, 0xC7, 0xC7, 0x00, 0x00, 0x00, 0x00});
                                      emitter.EmitBytes({0x4C, 0x89, 0xEE});
                                      emitter.EmitBytes({0x48, 0xC7, 0xC2, 0x01, 0x00, 0x00, 0x00});
                                      emitter.EmitBytes({0x0F, 0x05});
                                                        break;
                                                    case '[':
                                                        // cmpb $0, 0(%r13)
                                      emitter.EmitBytes({0x41, 0x80, 0x7d, 0x00, 0x00});

                                      // Save the location in the stack, and emit JZ (with 32-bit relative
                                      // offset) with 4 placeholder zeroes that will be fixed up later.
                                      open_bracket_stack.push(emitter.size());
                                      emitter.EmitBytes({0x0F, 0x84});
                                      emitter.EmitUint32(0);
                    break;
                case ']':
                {
                      if (open_bracket_stack.empty()) {
                        std::cerr << "unmatched closing ']' at pc=" << PC_index;
                      }
                      size_t open_bracket_offset = open_bracket_stack.top();
                      open_bracket_stack.pop();

                      // cmpb $0, 0(%r13)
                      emitter.EmitBytes({0x41, 0x80, 0x7d, 0x00, 0x00});

                      // open_bracket_offset points to the JZ that jumps to this closing
                      // bracket. We'll need to fix up the offset for that JZ, as well as emit a
                      // JNZ with a correct offset back. Note that both [ and ] jump to the
                      // instruction *after* the matching bracket if their condition is
                      // fulfilled.

                      // Compute the offset for this jump. The jump start is computed from after
                      // the jump instruction, and the target is the instruction after the one
                      // saved on the stack.
                      size_t jump_back_from = emitter.size() + 6;
                      size_t jump_back_to = open_bracket_offset + 6;
                      uint32_t pcrel_offset_back =
                          compute_relative_32bit_offset(jump_back_from, jump_back_to);

                      // jnz <open_bracket_location>
                      emitter.EmitBytes({0x0F, 0x85});
                      emitter.EmitUint32(pcrel_offset_back);

                      // Also fix up the forward jump at the matching [. Note that here we don't
                      // need to add the size of this jmp to the "jump to" offset, since the jmp
                      // was already emitted and the emitter size was bumped forward.
                      size_t jump_forward_from = open_bracket_offset + 6;
                      size_t jump_forward_to = emitter.size();
                      uint32_t pcrel_offset_forward =
                          compute_relative_32bit_offset(jump_forward_from, jump_forward_to);
                      emitter.ReplaceUint32AtOffset(open_bracket_offset + 2,
                                                    pcrel_offset_forward);
                    break;
                }
                default:
                    std::cerr << "Failed to compile code from file=" << filename
                              << ", at position = " << PC_index
                              << ": and at instruction = '" << instruction << "'" << std::endl;
                    exit(1);
            }
        }
    }
    
    void compile(const std::string& code, const std::string& filename) {
        for (char eachword : code) {
            if (std::string("><+-.,[]").find(eachword) != std::string::npos) {
                preprocessed.push_back(eachword);
            }
        }
        assign_loop_label();
        initial_setup_assembly_structure();
        gen_assembly(filename);
        final_setup_assembly_structute();

        
        // Load the emitted code to executable memory and run it.
        std::vector<uint8_t> emitted_code = emitter.code();

        // std::cout << std::hex << std::setfill('0');
        // for (auto each : emitted_code) {
        //     std::cout << std::setw(2) << static_cast<int>(each) << " ";
        // }
        // std::cout << std::dec << std::setfill(' ');

        JitProgram jit_program(emitted_code);
        using JittedFunc = void (*)(void);
        JittedFunc func = (JittedFunc)jit_program.program_memory();
        func();

    }
};

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <input_file> [--verbose]" << std::endl;
        return 1;
    }
    std::string input_file = argv[1];
    bool verbose = (argc > 2 && std::string(argv[2]) == "--verbose");

    std::cout << "Running JIT on input file = " << input_file << std::endl;

    Compiler compiler(verbose);
    std::ifstream file(input_file);
    if (!file) {
        std::cerr << "Failed to open file: " << input_file << std::endl;
        return 1;
    }

    std::string code((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    compiler.compile(code, input_file);

    return 0;
}
