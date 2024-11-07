cmake_minimum_required(VERSION 3.20.0)
project(AOT_LLVM_Project)

# Find the LLVM package
find_package(LLVM REQUIRED CONFIG)

# Print LLVM information
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

# Include LLVM header files
include_directories(${LLVM_INCLUDE_DIRS})
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})

# Specify the source file
add_executable(aot_llvm aot_llvm.cpp)

# Link the required LLVM components
llvm_map_components_to_libnames(llvm_libs Core Support x86asmparser x86codegen passes irreader)
target_link_libraries(aot_llvm ${llvm_libs})

