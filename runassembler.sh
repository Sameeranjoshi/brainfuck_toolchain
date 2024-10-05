#!/bin/bash

# Commands
clang++ -std=c++17 -O3 -o wrapper compiler_x86_64.cpp

# Create a directory to store the timing results
mkdir -p timing_results

# Benchmarks in benches folder
for file in benches/*.b; do
    # Run with --baseline
    echo "Processing $file " >> timing_results/all_results_baseline.time
    ./wrapper $file
    as -o program.o assembly_output.s 
    gcc -o out program.o -lc
    
    # Measure the time and save the result
    { time ./out; } 2>> timing_results/all_results_baseline.time
    echo "" >> timing_results/all_results_baseline.time


    # Run with --optimize-simple-loops
    echo "Processing $file " >> timing_results/all_results_with_loop.time
    ./wrapper $file --optimize-simple-loops 
    as -o program.o assembly_output.s 
    gcc -o out program.o -lc
    
    # Measure the time and save the result
    { time ./out; } 2>> timing_results/all_results_with_loop.time
    echo "" >> timing_results/all_results_with_loop.time

    # Run with --optimize-memory-scans
    echo "Processing $file " >> timing_results/all_results_with_scan.time
    ./wrapper $file --optimize-memory-scans
    as -o program.o assembly_output.s
    gcc -o out program.o -lc

    # Measure the time and save the result
    { time ./out; } 2>> timing_results/all_results_with_scan.time
    echo "" >> timing_results/all_results_with_scan.time


    # Run with --optimize-simple-loops and --optimize-memory-scans
    echo "Processing $file " >> timing_results/all_results_with_loop_and_scan.time
    ./wrapper $file --optimize-simple-loops --optimize-memory-scans
    as -o program.o assembly_output.s
    gcc -o out program.o -lc
    # Measure the time and save the result
    { time ./out; } 2>> timing_results/all_results_with_loop_and_scan.time
    echo "" >> timing_results/all_results_with_loop_and_scan.time


done
