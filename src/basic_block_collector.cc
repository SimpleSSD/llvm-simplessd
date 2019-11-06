// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2019 CAMELab
 *
 * Author: Donghyun Gouk <kukdh1@camelab.org>
 */

#include "src/basic_block_collector.hh"

#include <cxxabi.h>

#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"

#define DEBUG_TYPE "SimpleSSD::LLVM::BasicBlockCollector"

using namespace llvm;

static cl::opt<std::string> outputfile(
    "inststat-output",
    cl::desc("Output file of SimpleSSD basic block infomation"),
    cl::value_desc("filename"), cl::ValueRequired);

namespace SimpleSSD::LLVM {

BasicBlockCollector::BasicBlockCollector() : FunctionPass(ID) {
#if DEBUG_MODE
  outs() << "SimpleSSD instruction statistic generator.\n";
  outs() << " Output file: " << outputfile << "\n";
#endif
}

bool BasicBlockCollector::runOnFunction(Function &func) {
  int ret = 0;

  if (isMarked(func)) {
#ifdef DEBUG_MODE
    // We found CPU::Function object
    auto mangle = func.getName();
    auto funcname = abi::__cxa_demangle(mangle.data(), nullptr, nullptr, &ret);

    outs() << "Collecting basic block information of:\n";
    outs() << "  " << mangle;

    if (ret == 0) {
      outs() << " (" << funcname << ")";
    }

    outs() << "\n";

    free(funcname);
#endif
  }

  return false;
}

void BasicBlockCollector::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
}

// Don't remove below
char SimpleSSD::LLVM::BasicBlockCollector::ID = 0;
static RegisterPass<SimpleSSD::LLVM::BasicBlockCollector> X(
    "inststat", "SimpleSSD instruction statistic generator");
static RegisterStandardPasses Y(PassManagerBuilder::EP_EarlyAsPossible,
                                [](const PassManagerBuilder &,
                                   legacy::PassManagerBase &p) {
                                  p.add(new BasicBlockCollector());
                                });

}  // namespace SimpleSSD::LLVM
