#!/bin/bash
PREFIX="temp_build"
SOURCE_FILE=$1
SOURCE_DIR=$(dirname $SOURCE_FILE)

TEXT_MODE=1
TEXT_FLAG="-S"

if [ $TEXT_MODE -eq "1" ];
then
  LLVM_ASSEMBLY=$PREFIX"/"$1".ll"
  LLVM_ASSEMBLY_OPT=$PREFIX"/"$1".opt.ll"
else
  LLVM_ASSEMBLY=$PREFIX"/"$1".bc"
  LLVM_ASSEMBLY_OPT=$PREFIX"/"$1".opt.bc"
  TEXT_FLAG=""
fi

ASSEMBLY=$LLVM_ASSEMBLY".S"
OBJECT=$LLVM_ASSEMBLY".o"

mkdir -p $PREFIX"/"$SOURCE_DIR

# Basic Block collection
clang++ -std=c++17 -DEXCLUDE_CPU -g -emit-llvm -I. -I../lib/drampower/src -c $TEXT_FLAG -o $LLVM_ASSEMBLY $SOURCE_FILE
opt --load ./lib/llvm-simplessd/build/libllvm-simplessd.so --blockcollector -O2 $TEXT_FLAG -o $LLVM_ASSEMBLY_OPT $LLVM_ASSEMBLY
llc -O2 -filetype=asm -o $ASSEMBLY $LLVM_ASSEMBLY_OPT

# Instruction count
./lib/llvm-simplessd/build/inststat-generator $LLVM_ASSEMBLY

# Apply instruction statistics
clang++ -std=c++17 -g -emit-llvm -I. -I../lib/drampower/src -c $TEXT_FLAG -o $LLVM_ASSEMBLY $SOURCE_FILE
opt --load ./lib/llvm-simplessd/build/libllvm-simplessd.so --inststat -O2 $TEXT_FLAG -o $LLVM_ASSEMBLY_OPT $LLVM_ASSEMBLY
llc -O2 -filetype=obj -o $OBJECT $LLVM_ASSEMBLY_OPT
