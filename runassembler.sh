#!/bin/bash

# Commands
clang++ -std=c++17 -O3 -o wrapper compiler_x86_64.cpp

# Create a directory to store the timing results
mkdir -p timing_results

# Benchmarks in benches folder
for file in benches/*.b; do
    # Run without --optimize-simple-loops
    ./wrapper $file
    as -o program.o assembly_output.s 
    gcc -o out program.o -lc
    
    # Measure the time and save the result
    echo "Results for $(basename $file) without --optimize-simple-loops:" >> timing_results/all_results_no_loop.time
    { time ./out; } 2>> timing_results/all_results_no_loop.time
    echo "" >> timing_results/all_results_no_loop.time

    # Run with --optimize-simple-loops
    ./wrapper $file --optimize-simple-loops 
    as -o program.o assembly_output.s 
    gcc -o out program.o -lc
    
    # Measure the time and save the result
    echo "Results for $(basename $file) with --optimize-simple-loops:" >> timing_results/all_results_with_loop.time
    { time ./out; } 2>> timing_results/all_results_with_loop.time
    echo "" >> timing_results/all_results_with_loop.time
done
