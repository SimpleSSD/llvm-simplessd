// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2019 CAMELab
 *
 * Author: Donghyun Gouk <kukdh1@camelab.org>
 */

#include "src/inst_stat_gen.hh"

#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"

#define DEBUG_TYPE "SimpleSSD::LLVM::InstructionStatisticGenerator"

using namespace llvm;

static cl::opt<std::string> instOutputfile(
    "inststat-output",
    cl::desc("Output file of SimpleSSD instruction statistics"),
    cl::value_desc("filename"), cl::ValueRequired);

namespace SimpleSSD::LLVM {

InstructionStatisticGenerator::InstructionStatisticGenerator()
    : FunctionPass(ID) {
  outs() << "SimpleSSD instruction statistic generator.\n";
  outs() << " Output file: " << instOutputfile << "\n";
}

bool InstructionStatisticGenerator::runOnFunction(Function &F) {
  errs() << "Hello: ";
  errs().write_escaped(F.getName()) << '\n';

  return false;
}

void InstructionStatisticGenerator::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
}

// Don't remove below
char SimpleSSD::LLVM::InstructionStatisticGenerator::ID = 0;
static RegisterPass<SimpleSSD::LLVM::InstructionStatisticGenerator> X(
    "inststat", "SimpleSSD instruction statistic generator");
static RegisterStandardPasses Y(PassManagerBuilder::EP_EarlyAsPossible,
                                [](const PassManagerBuilder &,
                                   legacy::PassManagerBase &p) {
                                  p.add(new InstructionStatisticGenerator());
                                });

}  // namespace SimpleSSD::LLVM
