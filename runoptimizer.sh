#!/bin/bash

# Commands
clang++ -std=c++17 -O3 -g -o wrapper compiler_x86_64.cpp

# Benchmarks in benches folder
for file in benches/hanoi.b; do
    ./wrapper $file --optimize-memory-scans
    as -o program.o assembly_output.s 
    gcc -o out program.o -lc
    ./out
done

