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