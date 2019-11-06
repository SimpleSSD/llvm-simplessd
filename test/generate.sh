#!/bin/bash
PREFIX="temp_build"
SOURCE_FILE=$1
SOURCE_DIR=$(dirname $SOURCE_FILE)
LLVM_ASSEMBLY=$PREFIX"/"$1".ll"
LLVM_ASSEMBLY_OPT=$PREFIX"/"$1".opt.ll"
ASSEMBLY=$PREFIX"/"$1".S"

mkdir -p $PREFIX"/"$SOURCE_DIR

clang++ -std=c++17 -DEXCLUDE_CPU -g -S -emit-llvm -I. -I../lib/drampower/src -c $SOURCE_FILE -o $LLVM_ASSEMBLY
opt --load ./lib/llvm-simplessd/build/libllvm_simplessd.so --blockcollector -O2 -S -march arm -mcpu cortex-r7 -o $LLVM_ASSEMBLY_OPT $LLVM_ASSEMBLY
llc -march=arm -mcpu=cortex-r7 -filetype=asm -o $ASSEMBLY $LLVM_ASSEMBLY_OPT
