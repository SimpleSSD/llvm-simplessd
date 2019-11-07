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
#include "llvm/IR/Verifier.h"
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

  // %idx
  auto idx0 = builder.getInt32(0);
  auto idx1 = builder.getInt32(1);
  auto idx2 = builder.getInt32(2);
  auto idx3 = builder.getInt32(3);
  auto idx4 = builder.getInt32(4);
  auto idx5 = builder.getInt32(5);
  auto idx6 = builder.getInt32(6);

  // IdxList
  Value *idxList0[2] = {idx0, idx0};
  Value *idxList1[2] = {idx0, idx1};
  Value *idxList2[2] = {idx0, idx2};
  Value *idxList3[2] = {idx0, idx3};
  Value *idxList4[2] = {idx0, idx4};
  Value *idxList5[2] = {idx0, idx5};
  Value *idxList6[2] = {idx0, idx6};

  // Add instructions
  pointers.branch = builder.CreateInBoundsGEP(
      fstat, ArrayRef<Value *>(idxList0, 2), "fstat_branch");
  pointers.load = builder.CreateInBoundsGEP(
      fstat, ArrayRef<Value *>(idxList1, 2), "fstat_load");
  pointers.store = builder.CreateInBoundsGEP(
      fstat, ArrayRef<Value *>(idxList2, 2), "fstat_store");
  pointers.arithmetic = builder.CreateInBoundsGEP(
      fstat, ArrayRef<Value *>(idxList3, 2), "fstat_arithmetic");
  pointers.floating = builder.CreateInBoundsGEP(
      fstat, ArrayRef<Value *>(idxList4, 2), "fstat_floating");
  pointers.other = builder.CreateInBoundsGEP(
      fstat, ArrayRef<Value *>(idxList5, 2), "fstat_other");
  pointers.cycles = builder.CreateInBoundsGEP(
      fstat, ArrayRef<Value *>(idxList6, 2), "fstat_cycles");
}

void InstructionApplier::makeAdd(llvm::Instruction *next, Value *target,
                                 uint64_t value) {
  // %reg = load i64, i64* %target, align 8
  // %add = add i64 %reg, %value
  // store i64 %add, i64* %target, align 8

  // Create builder
  IRBuilder<> builder(next);

  // Load
  auto load = builder.CreateLoad(target);
  load->setAlignment(8);

  // Add
  auto add = builder.CreateAdd(builder.getInt64(value), load);

  // Store
  auto store = builder.CreateStore(add, target);
  store->setAlignment(8);
}

bool InstructionApplier::doInitialization(Module &module) {
  std::string filename(inputFile);

  // Get module name
  auto name = module.getName().data();

  // Make filename
  filename += name;
  filename += IA_FILE_POSTFIX;

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
  Instruction *next = nullptr;

  if (isMarked(func, &fstat, &next)) {
#ifdef DEBUG_MODE
    outs() << "Handling function: ";

    printFunctionName(outs(), func);

    outs() << ", storing statistics to value: ";
    outs() << fstat->getName() << ".\n";
#endif

    // Setup pointers of fstat
    makePointers(next, fstat);

    // Apply instruction stats
    for (auto &block : func) {
      auto &last = block.back();

      // TEMPCODE
      // For each blocks, add 1 for all seven fields
      makeAdd(&last, pointers.branch, 1);
      makeAdd(&last, pointers.load, 1);
      makeAdd(&last, pointers.store, 1);
      makeAdd(&last, pointers.arithmetic, 1);
      makeAdd(&last, pointers.floating, 1);
      makeAdd(&last, pointers.other, 1);
      makeAdd(&last, pointers.cycles, 7);
    }

    // Verify function
    if (verifyFunction(func, &errs())) {
      func.dump();
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
