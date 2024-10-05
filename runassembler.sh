#!/bin/bash

# Commands
clang++ -std=c++17 -O3 -o wrapper compiler_x86_64.cpp

# Create a directory to store the timing results
mkdir -p timing_results

# Benchmarks in benches folder
run_benchmark() {
    local file=$1
    local options=$2
    local result_file=$3

    echo "Processing $file " >> $result_file
    ./wrapper $file $options
    as -o program.o assembly_output.s
    gcc -o out program.o -lc
    { time ./out; } 2>> $result_file
    echo "" >> $result_file
}

for file in benches/*.b; do
    run_benchmark $file "" "timing_results/all_results_baseline.time"
    run_benchmark $file "--optimize-simple-loops" "timing_results/all_results_with_loop.time"
    run_benchmark $file "--optimize-memory-scans" "timing_results/all_results_with_scan.time"
    run_benchmark $file "--optimize-simple-loops --optimize-memory-scans" "timing_results/all_results_with_loop_and_scan.time"
done
