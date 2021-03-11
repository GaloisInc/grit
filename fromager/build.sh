#!/bin/bash
set -e

# Note: if your LLVM binaries have a version suffix (such as `llvm-link-9`),
# set the `LLVM_SUFFIX` environment variable (e.g. `LLVM_SUFFIX=-9`).

if [[ -n "$FROMAGER_DEBUG" ]]; then
    CFLAGS="$CFLAGS -DFROMAGER_DEBUG"
fi

CC="clang${LLVM_SUFFIX} -flto -O1 -mprefer-vector-width=1 $CFLAGS"
CXX="clang++${LLVM_SUFFIX} -flto -O1 -mprefer-vector-width=1 -fno-rtti $CFLAGS"

# Build only the libraries we actually need.
mkdir -p build build/fromager
make libcldib.a libgrit.a build/driver.o CXX="$CXX"

echo "### BUILD COMPLETED"

$CXX -I cldib -I libgrit \
    -c fromager/driver_secret.cpp -o build/fromager/driver_secret.o

echo "### C++ call completed"

# Link the full libs + driver into a single bitcode file
llvm-link${LLVM_SUFFIX} build/*.o -o build/fromager/driver-main.bc

echo "### Link call completed"

# Optimize, removing unused public symbols
opt${LLVM_SUFFIX} \
    --internalize --internalize-public-api-list=main \
    --strip-debug --force-vector-width=1 \
    -O3 --scalarizer -O1 \
    build/fromager/driver-main.bc \
    -o build/fromager/driver-opt.bc

echo "### optimization call completed"


llvm-link${LLVM_SUFFIX} \
    build/fromager/driver-opt.bc \
    build/fromager/driver_secret.o \
    -o build/fromager/driver-full.bc

# Compile with -no-builtin so that Clang/LLVM doesn't try to optimize our
# implementation of `memcpy` into a simple `memcpy` call.
$CC -O3 -c fromager/libfromager.c -I fromager -o build/fromager/libfromager.o -fno-builtin
$CXX -O3 -c fromager/libfromager++.cpp -I fromager -o build/fromager/libfromager++.o

llvm-link${LLVM_SUFFIX} \
    build/fromager/{driver-full.bc,libfromager.o,libfromager++.o} \
    -o driver-link.bc

# Disassemble to LLVM's textual IR format
llvm-dis${LLVM_SUFFIX} driver-link.bc -o driver-link.ll
# driver-link.bc and driver-link.ll are the final LLVM outputs.
sed -i -e 's/nofree//g' driver-link.ll

# Produce an executable
clang++${LLVM_SUFFIX} -o driver driver-opt.bc -lm -lpthread
