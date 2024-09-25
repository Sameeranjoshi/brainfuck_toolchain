

# Commands
clang++ -std=c++17 -O3 -o wrapper compiler_x86_64.cpp

#benchmarks in benches folder
for file in benches/*.b; do
    ./wrapper $file
    as -o program.o assembly_output.s 
    gcc -o out program.o -lc
    ./out
done