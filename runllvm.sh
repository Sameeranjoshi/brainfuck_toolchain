
#AOT
# Commands
mkdir -p build
cd build
cmake -DLLVM_DIR=/usr/lib64/cmake/llvm/ -G Ninja ..
ninja

# now you should find a aot_llvm file in the build folder
# Replaces:    clang++ -std=c++17 -O3 -o aot_llvm aot_llvm.cpp 


# Benchmarks in benches folder
run_benchmark() {
    local file=$1
    local result_file=$3

    echo "Processing $file " >> $result_file
    ./aot_llvm $file
    llc -march=x86-64 output.ll -o output.s
    gcc output.s -o out
    { time ./out ; } 2>> $result_file
    echo "" >> $result_file
}

mkdir -p ../timing_results_llvm
for file in ../benches/*.b; do
    run_benchmark $file "" "../timing_results_llvm/all_results_llvm_baseline.time"
done
