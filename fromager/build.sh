#!/bin/bash
set -e

# Note: if your LLVM binaries have a version suffix (such as `llvm-link-9`),
# set the `LLVM_SUFFIX` environment variable (e.g. `LLVM_SUFFIX=-9`).

GRIT_DIR="$(dirname "$0")/.."
PICOLIBC_HOME="$GRIT_DIR/../picolibc/build/image/picolibc/x86_64-unknown-fromager"
CLANG_DIR="$(clang${LLVM_SUFFIX} -print-resource-dir)"

CFLAGS="-flto -O1 -mprefer-vector-width=1
    -DFROMAGER
    -nostdinc++
    -isystem $GRIT_DIR/../llvm-project/libcxxabi/include
    -isystem $GRIT_DIR/../llvm-project/libcxx/include
    -nostdinc
    -isystem $CLANG_DIR/include
    -isystem $PICOLIBC_HOME/include
    -ggdb
    "

if [[ -n "$FROMAGER_DEBUG" ]]; then
    CFLAGS="$CFLAGS -DFROMAGER_DEBUG"
fi

CC="clang${LLVM_SUFFIX} $(echo $CFLAGS)"
CXX="clang++${LLVM_SUFFIX} -fno-rtti $(echo $CFLAGS) -v"

# Build only the libraries we actually need.
mkdir -p build build/fromager
make libcldib.a libgrit.a build/driver.o CXX="$CXX"

$CXX -I cldib -I libgrit \
    -c fromager/driver_secret.cpp -o build/fromager/driver_secret.bc

# Link the full libs + driver into a single bitcode file
llvm-link${LLVM_SUFFIX} build/*.o -o build/fromager/driver-main.bc


cc_objects="
        libcldib.a
        libgrit.a
        --override=build/driver.o
    " \
    cc_secret_objects=build/fromager/driver_secret.bc \
    $PICOLIBC_HOME/lib/fromager-link.sh "$@"
