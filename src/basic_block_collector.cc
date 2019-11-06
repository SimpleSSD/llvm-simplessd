// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2019 CAMELab
 *
 * Author: Donghyun Gouk <kukdh1@camelab.org>
 */

#include "src/basic_block_collector.hh"

#include <cxxabi.h>

#include <string>

#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/raw_ostream.h"

#define DEBUG_TYPE "SimpleSSD::LLVM::BasicBlockCollector"

#define BB_OPTION_OUTFILE "block-output"

using namespace llvm;

static cl::opt<std::string> outputFile(
    BB_OPTION_OUTFILE,
    cl::desc("Output file of SimpleSSD basic block infomation"),
    cl::value_desc("filename"), cl::ValueRequired);

namespace SimpleSSD::LLVM {

BasicBlockCollector::BasicBlockCollector() : FunctionPass(ID), inited(false) {
#if DEBUG_MODE
  outs() << "SimpleSSD instruction statistic generator.\n";
#endif

  // Validate file
  if (outputFile.length() == 0) {
    errs() << " You must specify option '--" << BB_OPTION_OUTFILE
           << "=<filename>'\n";
  }
  else {
    outfile.open(outputFile);

    if (outfile.is_open()) {
      inited = true;
    }
    else {
      errs() << " Failed to open file: " << outputFile << "\n";
    }
  }
}

BasicBlockCollector::~BasicBlockCollector() {
  if (inited) {
    outfile.close();
  }

  inited = false;
}

bool BasicBlockCollector::runOnFunction(Function &func) {
  if (!inited) {
    return false;
  }

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

// Don't remove below
char SimpleSSD::LLVM::BasicBlockCollector::ID = 0;
static RegisterPass<SimpleSSD::LLVM::BasicBlockCollector> X(
    "blockcollector", "SimpleSSD basic block collector");

}  // namespace SimpleSSD::LLVM
