#!/bin/bash

#JIT
# Create a directory to store the timing results
mkdir -p timing_results_jit

result_file="timing_results_jit/all_results_jit_baseline.time"
clang++ -std=c++17 -O3 -g -o jit jit_compiler.cpp
for file in benches/*.b; do
    echo "Processing $file " >> $result_file
    { time ./jit $file ; }  2>> $result_file
    echo "" >> $result_file
done


#AOT
# Commands
clang++ -std=c++17 -O3 -o compiler compiler_x86_64.cpp

# Benchmarks in benches folder
run_benchmark() {
    local file=$1
    local result_file=$3

    echo "Processing $file " >> $result_file
    ./compiler $file
    as -o program.o assembly_output.s
    gcc -o out program.o -lc
    { time ./out ; } 2>> $result_file
    echo "" >> $result_file
}

for file in benches/*.b; do
    run_benchmark $file "" "timing_results_jit/all_results_aot_baseline.time"
done
