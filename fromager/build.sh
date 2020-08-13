#!/bin/bash
set -e

# Note: if your LLVM binaries have a version suffix (such as `llvm-link-9`),
# set the `LLVM_SUFFIX` environment variable (e.g. `LLVM_SUFFIX=-9`).

# Build only the libraries we actually need.
mkdir -p build
make libcldib.a libgrit.a build/driver.o \
    CXX="clang++${LLVM_SUFFIX} -flto -O1 -mprefer-vector-width=1"

# Link the full libs + driver into a single bitcode file
llvm-link${LLVM_SUFFIX} -o driver-full.bc build/*.o

# Optimize, removing unused public symbols
opt${LLVM_SUFFIX} \
    --internalize --internalize-public-api-list=main \
    --strip-debug --force-vector-width=1 \
    -O3 --scalarizer -O1 -o driver-opt.bc driver-full.bc

# Disassemble to LLVM's textual IR format
llvm-dis${LLVM_SUFFIX} driver-opt.bc -o driver-opt.ll
# driver-opt.bc and driver-opt.ll are the final LLVM outputs.

# Produce an executable
clang++${LLVM_SUFFIX} -o driver driver-opt.bc -lm -lpthread
