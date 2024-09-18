#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <string>
#include <filesystem>
#include <stdexcept>
#include <cstdint>
#include <algorithm>

namespace fs = std::filesystem;

class Constants {
public:
    static const int SIZE_OF_TAPE = 1000;
    std::vector<std::string> input_files;

    Constants() {
        std::string directory = "brainfuck-benchmark/benches/";
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
    std::unordered_map<int, int> loop_bounds;

public:
    Interpreter(int tape_size) : tape(tape_size, 0), tape_pointer(tape_size / 2) {}

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
                loop_bounds[start] = idx;
                loop_bounds[idx] = start;
            }
        }
        if (!stack.empty()) {
            throw std::runtime_error("Unmatched '[' found");
        }
    }

    void interpret(const std::string& code, const std::string& filename) {
        for (char eachword : code) {
            if (std::string("><+-.,[]").find(eachword) != std::string::npos) {
                preprocessed.push_back(eachword);
            }
        }
        preprocess_code();

        size_t PC_index = 0;
        while (PC_index < preprocessed.size()) {
            char instruction = preprocessed[PC_index];

            switch (instruction) {
                case '>':
                    move_right();
                    break;
                case '<':
                    move_left();
                    break;
                case '+':
                    increment();
                    break;
                case '-':
                    decrement();
                    break;
                case '.':
                    write();
                    break;
                case ',':
                    if (std::cin.eof()) {
                        tape[tape_pointer] = 0; // Set to 0 if end of input stream
                    } else {
                        char input_char;
                        std::cin >> input_char;
                        replace(static_cast<uint8_t>(input_char));
                    }
                    break;
                case '[':
                    if (is_currentcell_zero()) {
                        PC_index = loop_bounds[PC_index];
                    }
                    break;
                case ']':
                    if (!is_currentcell_zero()) {
                        PC_index = loop_bounds[PC_index];
                    }
                    break;
                default:
                    std::cerr << "Failed to interpret code from file=" << filename
                              << ", at position = " << PC_index
                              << ": and at instruction = '" << instruction << "'" << std::endl;
                    exit(1);
            }

            PC_index++;
        }

        std::cout << "\nSuccessfully interpreted code from file=" << filename << std::endl;
    }
};

int main() {
    Constants consts;

    for (const auto& eachfile : consts.input_files) {
        std::cout << "Running interpreter on input file = " << eachfile << std::endl;

        Interpreter interpreter(Constants::SIZE_OF_TAPE);
        std::ifstream file(eachfile);
        if (!file) {
            std::cerr << "Failed to open file: " << eachfile << std::endl;
            continue;
        }

        std::string code((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        interpreter.interpret(code, eachfile);
    }

    return 0;
}
