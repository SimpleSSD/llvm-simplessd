#!/bin/bash

SOURCE_FILE=$1
LLVM_ASSEMBLY=$1".ll"
LLVM_ASSEMBLY_OPT=$1".opt.ll"
ASSEMBLY=$1".S"

clang++ -std=c++17 -DDISABLE_LOG -emit-llvm -I. -I../lib/drampower/src -S -c $SOURCE_FILE -o $LLVM_ASSEMBLY
opt -O2 -S -march arm -mcpu cortex-r7 -o $LLVM_ASSEMBLY_OPT $LLVM_ASSEMBLY
llc -march=arm -mcpu=cortex-r7 -filetype=asm -o $ASSEMBLY $LLVM_ASSEMBLY_OPT
