// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2019 CAMELab
 *
 * Author: Donghyun Gouk <kukdh1@camelab.org>
 */

#include "src/inst_stat_gen.hh"

#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/raw_ostream.h"

#define DEBUG_TYPE "SimpleSSD::LLVM::InstructionStatisticGenerator"

static llvm::cl::opt<std::string> instOutputfile(
    "inststat-output",
    llvm::cl::desc("Output file of SimpleSSD instruction statistics"),
    llvm::cl::value_desc("filename"), llvm::cl::ValueRequired);

namespace SimpleSSD::LLVM {

InstructionStatisticGenerator::InstructionStatisticGenerator()
    : llvm::FunctionPass(ID) {
  llvm::outs() << "SimpleSSD instruction statistic generator.\n";
  llvm::outs() << " Output file: " << instOutputfile << "\n";
}

bool InstructionStatisticGenerator::runOnFunction(llvm::Function &F) {
  llvm::errs() << "Hello: ";
  llvm::errs().write_escaped(F.getName()) << '\n';

  return false;
}

// We don't modify the program, so we preserve all analyses.
void InstructionStatisticGenerator::getAnalysisUsage(
    llvm::AnalysisUsage &AU) const {
  AU.setPreservesAll();
}

// Don't remove below
char SimpleSSD::LLVM::InstructionStatisticGenerator::ID = 0;
static llvm::RegisterPass<SimpleSSD::LLVM::InstructionStatisticGenerator> X(
    "inststat", "SimpleSSD instruction statistic generator");

}  // namespace SimpleSSD::LLVM
