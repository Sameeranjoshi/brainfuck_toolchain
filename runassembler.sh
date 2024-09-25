

# Commands
clang++ -std=c++17 -O3 -o wrapper compiler_x86_64.cpp
./wrapper
as -o program.o assembly_output.s 
gcc -o out program.o -lc
./out
