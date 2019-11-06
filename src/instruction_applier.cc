// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2019 CAMELab
 *
 * Author: Donghyun Gouk <kukdh1@camelab.org>
 */

#include "src/instruction_applier.hh"

#include <string>

#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/CommandLine.h"

#define DEBUG_TYPE "SimpleSSD::LLVM::InstructionApplier"

using namespace llvm;

static cl::opt<std::string> inputFile(
    "inststat-prefix",
    cl::desc("Input file prefix of SimpleSSD instruction statistics"),
    cl::value_desc("filename"), cl::init(""));

namespace SimpleSSD::LLVM {

InstructionApplier::InstructionApplier() : FunctionPass(ID), inited(false) {
#if DEBUG_MODE
  outs() << "SimpleSSD instruction statistic applier.\n";
#endif
}

void InstructionApplier::makePointers(Instruction *next, Value *fstat) {
  // %ptr = getelementptr inbounds %"class.SimpleSSD::CPU::Function",
  // %"class.SimpleSSD::CPU::Function"* %fstat, i32 0, i32 %idx

  // Create builder
  IRBuilder<> builder(next);

  // Context
  auto &ctx = builder.getContext();

  // %idx
  auto idx0 = ConstantInt::get(ctx, APInt(32, 0));  // Constant uint32_t 0
  auto idx1 = ConstantInt::get(ctx, APInt(32, 1));  // Constant uint32_t 1
  auto idx2 = ConstantInt::get(ctx, APInt(32, 2));  // Constant uint32_t 2
  auto idx3 = ConstantInt::get(ctx, APInt(32, 3));  // Constant uint32_t 3
  auto idx4 = ConstantInt::get(ctx, APInt(32, 4));  // Constant uint32_t 4
  auto idx5 = ConstantInt::get(ctx, APInt(32, 5));  // Constant uint32_t 5
  auto idx6 = ConstantInt::get(ctx, APInt(32, 6));  // Constant uint32_t 6

  // IdxList
  Value *idxList0[2] = {idx0, idx0};
  Value *idxList1[2] = {idx0, idx1};
  Value *idxList2[2] = {idx0, idx2};
  Value *idxList3[2] = {idx0, idx3};
  Value *idxList4[2] = {idx0, idx4};
  Value *idxList5[2] = {idx0, idx5};
  Value *idxList6[2] = {idx0, idx6};

  // Add instructions
  pointers.branch =
      builder.CreateGEP(fstat, ArrayRef<Value *>(idxList0, 2), "fstat_branch");
  pointers.load =
      builder.CreateGEP(fstat, ArrayRef<Value *>(idxList1, 2), "fstat_load");
  pointers.store =
      builder.CreateGEP(fstat, ArrayRef<Value *>(idxList2, 2), "fstat_store");
  pointers.arith =
      builder.CreateGEP(fstat, ArrayRef<Value *>(idxList3, 2), "fstat_arith");
  pointers.fp =
      builder.CreateGEP(fstat, ArrayRef<Value *>(idxList4, 2), "fstat_fp");
  pointers.other =
      builder.CreateGEP(fstat, ArrayRef<Value *>(idxList5, 2), "fstat_other");
  pointers.cycles =
      builder.CreateGEP(fstat, ArrayRef<Value *>(idxList6, 2), "fstat_cycles");
}

void InstructionApplier::makeAdd(llvm::Instruction *next, Value *target,
                                 uint64_t value) {
  // %reg = load i64, i64* %target, align 8
  // %add = add i64 %reg, %value
  // store i64 %add, i64* %target, align 8

  // Create builder
  IRBuilder<> builder(next);

  // Context
  auto &ctx = builder.getContext();

  // Value to add
  auto constant = ConstantInt::get(ctx, APInt(64, value));

  // Load
  auto load = builder.CreateLoad(target);

  // Add
  auto add = builder.CreateAdd(constant, load);

  // Store
  builder.CreateStore(add, target);
}

bool InstructionApplier::doInitialization(Module &module) {
  std::string filename(inputFile);

  // Get module name
  auto name = module.getName().data();

  // Make filename
  filename += name;
  filename += ".inststat.txt";

#if DEBUG_MODE
  outs() << " Input filename: " << filename << "\n";
#endif

  // Validate file
  infile.open(filename);

  if (infile.is_open()) {
    inited = true;
  }
  else {
    errs() << " Failed to open file: " << filename << "\n";
  }

  return false;
}

bool InstructionApplier::runOnFunction(Function &func) {
  if (!inited) {
    return false;
  }

  Value *fstat = nullptr;

  if (isMarked(func, &fstat)) {
#ifdef DEBUG_MODE
    outs() << "Handling function: ";

    printFunctionName(outs(), func);

    outs() << ", storing statistics to value: ";
    outs() << fstat->getName() << ".\n";
#endif

    // Setup pointers of fstat
    auto &entry = func.getEntryBlock();
    auto last = entry.getFirstInsertionPt();

    makePointers(&*last, fstat);

    // Apply instruction stats
    for (auto &block : func) {
      auto &last = block.back();

      // TEMPCODE
      // For each blocks, add 1 for all seven fields
      makeAdd(&last, pointers.branch, 1);
      makeAdd(&last, pointers.load, 1);
      makeAdd(&last, pointers.store, 1);
      makeAdd(&last, pointers.arith, 1);
      makeAdd(&last, pointers.fp, 1);
      makeAdd(&last, pointers.other, 1);
      makeAdd(&last, pointers.cycles, 7);
    }

    return true;
  }

  return false;
}

bool InstructionApplier::doFinalization(Module &) {
  if (inited) {
    infile.close();
  }

  inited = false;

  return false;
}

// Don't remove below
char SimpleSSD::LLVM::InstructionApplier::ID = 0;
static RegisterPass<SimpleSSD::LLVM::InstructionApplier> X(
    "inststat", "SimpleSSD instruction statistics applier", true, false);

}  // namespace SimpleSSD::LLVM
