

# Commands
clang++ -std=c++17 -O3 -o bfi inter_cpp.cpp

#benchmarks in benches folder
for file in benches/*.b; do
    ./bfi $file
done