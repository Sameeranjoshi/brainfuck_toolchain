
#AOT
# Commands
clang++ -std=c++17 -O3 -o llvmcompiler aot_llvm.cpp

# Benchmarks in benches folder
run_benchmark() {
    local file=$1
    local result_file=$3

    echo "Processing $file " >> $result_file
    ./llvmcompiler $file
    as -o program.o assembly_output.s
    gcc -o out program.o -lc
    { time ./out ; } 2>> $result_file
    echo "" >> $result_file
}

for file in benches/*.b; do
    run_benchmark $file "" "timing_results_jit/all_results_llvm_baseline.time"
done
