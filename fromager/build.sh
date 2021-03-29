#!/bin/bash
set -e

# Note: if your LLVM binaries have a version suffix (such as `llvm-link-9`),
# set the `LLVM_SUFFIX` environment variable (e.g. `LLVM_SUFFIX=-9`).

GRIT_DIR="$(dirname "$0")/.."
PICOLIBC_HOME="$GRIT_DIR/../picolibc/build/image/picolibc/x86_64-unknown-fromager"
CLANG_DIR="$(clang${LLVM_SUFFIX} -print-resource-dir)"

CFLAGS="-flto -O1 -mprefer-vector-width=1
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

$CC -c fromager/cc_trace_exec.c -o build/fromager/cc_trace_exec.bc

# Link the full libs + driver into a single bitcode file
llvm-link${LLVM_SUFFIX} build/*.o -o build/fromager/driver-main.bc


# MicroRAM LLVM output

# Link in the libraries prior to optimizing.  We don't add the driver secrets
# yet, so the optimizer won't propagate information about them.
llvm-link${LLVM_SUFFIX} \
    $PICOLIBC_HOME/lib/libc.bc \
    $PICOLIBC_HOME/lib/libm.bc \
    --override=build/fromager/driver-main.bc \
    -o build/fromager/driver-microram-nosecret.bc

keep_symbols=main
keep_symbols=$keep_symbols,__llvm__memcpy__p0i8__p0i8__i64
keep_symbols=$keep_symbols,__llvm__memmove__p0i8__p0i8__i64
keep_symbols=$keep_symbols,__llvm__memset__p0i8__i64
keep_symbols=$keep_symbols,__llvm__bswap__i32
keep_symbols=$keep_symbols,__llvm__ctpop__i32

# Optimize, removing unused public symbols
opt${LLVM_SUFFIX} \
    -load ../llvm-passes/passes.so \
    --internalize --internalize-public-api-list="$keep_symbols" \
    --cc-instrument \
    --force-vector-width=1 \
    -O3 --scalarizer -O1 \
    --strip-debug \
    build/fromager/driver-microram-nosecret.bc \
    -o build/fromager/driver-microram-nosecret-opt.bc

( cd build/fromager; ar x /dev/stdin ) <$PICOLIBC_HOME/lib/libmachine_syscalls.a
( cd build/fromager; ar x /dev/stdin ) <$PICOLIBC_HOME/lib/libmachine_syscalls_native.a
( cd build/fromager; ar x /dev/stdin ) <$PICOLIBC_HOME/lib/libmachine_builtins.a

# Link the driver code and the secrets.  No more optimizations should be run on
# the code after this.
llvm-link${LLVM_SUFFIX} \
    build/fromager/driver-microram-nosecret-opt.bc \
    build/fromager/driver_secret.bc \
    build/fromager/syscalls.c.o \
    build/fromager/llvm_intrin.c.o \
    -o build/fromager/driver-microram-full.bc

opt${LLVM_SUFFIX} \
    build/fromager/driver-microram-full.bc \
    --strip-debug --globaldce \
    -o build/fromager/driver-microram-full-nodebug.bc

# Disassemble to LLVM's textual IR format
llvm-dis${LLVM_SUFFIX} build/fromager/driver-microram-full-nodebug.bc -o driver-link.ll
# driver-link.bc and driver-link.ll are the final LLVM outputs.
sed -i -e 's/nofree//g' driver-link.ll


# Native binary output

llvm-link${LLVM_SUFFIX} \
    $PICOLIBC_HOME/lib/libc.bc \
    $PICOLIBC_HOME/lib/libm.bc \
    --override=build/fromager/driver-microram-full.bc \
    --override=build/fromager/syscalls_native.c.o \
    --override=build/fromager/cc_trace_exec.bc \
    -o build/fromager/driver-native-full.bc

keep_symbols_native=main
# Explicitly preserve some libm symbols.  `globaldce` can't see the connection
# between `llvm.floor.f64` and `floor` and will wrongly remove the latter.
keep_symbols_native=$keep_symbols_native,ceil,floor,trunc,llrint
keep_symbols_native=$keep_symbols_native,exp,exp2,log,pow
keep_symbols_native=$keep_symbols_native,memcpy,memmove,memset

opt${LLVM_SUFFIX} \
    build/fromager/driver-native-full.bc \
    --internalize --internalize-public-api-list="$keep_symbols_native" \
    --globaldce \
    -o build/fromager/driver-native-full-opt.bc

clang++${LLVM_SUFFIX} \
    -o driver \
    build/fromager/driver-native-full-opt.bc

