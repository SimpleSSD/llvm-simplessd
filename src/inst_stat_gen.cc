// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2019 CAMELab
 *
 * Author: Donghyun Gouk <kukdh1@camelab.org>
 */

#include "src/inst_stat_gen.hh"

#include <cxxabi.h>

#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"

#define DEBUG_TYPE "SimpleSSD::LLVM::InstructionStatisticGenerator"

#define FUNCTION_TYPE_NAME "class.SimpleSSD::CPU::Function"
#define MARK_FUNCION_NAME "_ZN9SimpleSSD3CPU12markFunctionERNS0_8FunctionE"

using namespace llvm;

static cl::opt<std::string> instOutputfile(
    "inststat-output",
    cl::desc("Output file of SimpleSSD instruction statistics"),
    cl::value_desc("filename"), cl::ValueRequired);

namespace SimpleSSD::LLVM {

InstructionStatisticGenerator::InstructionStatisticGenerator()
    : FunctionPass(ID) {
#if DEBUG_MODE
  outs() << "SimpleSSD instruction statistic generator.\n";
  outs() << " Output file: " << instOutputfile << "\n";
#endif
}

bool InstructionStatisticGenerator::isMarked(llvm::Function &func) {
  // Check function
  auto &entry = func.getEntryBlock();

  for (auto &inst : entry) {
    if (auto call = dyn_cast<CallInst>(&inst)) {
      auto callee = call->getCalledFunction();

      if (callee && callee->getName().compare(MARK_FUNCION_NAME) == 0) {
        return true;
      }
    }
  }

  return false;
}

bool InstructionStatisticGenerator::runOnFunction(Function &func) {
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
