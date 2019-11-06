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
