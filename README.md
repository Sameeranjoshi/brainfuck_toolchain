# Brainfuck Interpreter C++ and Python

This repository contains two implementations of a Brainfuck language processor
one in C++ and other in Python.
The Python interpreter is way slow! Switched to C++ then.

## Table of Contents

- [Requirements](#requirements)
- [Installation](#installation)
- [Usage](#usage)

## Requirements

- **C++ Compiler**: A C++17 compliant compiler.

## Installation

### 1. Clone the Repository

```bash
git clone https://github.com/Sameeranjoshi/brainfuck_toolchain.git
cd brainfuck_toolchain/
```

### 3. Build the Interpreter

#### Manual Compilation

- **Brainfuck Interpreter**

  ```bash
  clang++ -std=c++17 -O3 -o bfi inter_cpp.cpp
  ```

## Usage

### Brainfuck Interpreter

Run the interpreter: This runs all the benchmarks

```bash
./bfi
```
---

Run the interpreter with profiling enabled:
```bash
./bfi -p
```
This generates a `profiling_output.txt` file in the current directory.

### Brainfunk Compiler for X86-64

Run the compiler:

1. Generates the driver binary
2. runs assembler (`as`)
3. Links with C library using `gcc`
4. Executes the binary

```bash
bash runassembler.sh
```
Above command runs the compiler and generates assembly with and without using
optimization flags and generates the timing results in timing_results/ folder.

To control the optimization flags use
```bash
clang++ -std=c++17 -O3 -o compiler compiler_x86_64.cpp
./compiler <input_file> [--optimize-simple-loops] [--optimize-memory-scans]
```

