// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2019 CAMELab
 *
 * Author: Donghyun Gouk <kukdh1@camelab.org>
 */

#include "src/basic_block_collector.hh"

#include <string>

#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/CommandLine.h"

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

  if (isMarked(func)) {
#ifdef DEBUG_MODE
    // We found CPU::Function object
    outs() << "Collecting basic block of ";

    printFunctionName(outs(), func);

    outs() << ".\n";
#endif

    // Write function name
    outfile << "func: " << func.getName().data() << std::endl;
    outfile << " at: ";

    printLineInfo(outfile, func);

    outfile << std::endl;

    for (auto &block : func) {
      // Write basic block name
      outfile << " block: " << block.getName().data() << std::endl;

      // Print first instruction (with debug info)
      {
        auto iter = block.begin();

        outfile << "  from: ";

        while (iter != block.end()) {
          if (printLineInfo(outfile, *iter)) {
            break;
          }

          ++iter;
        }

        outfile << std::endl;
      }

      // Print last instruction (with debug info)
      {
        auto iter = block.rbegin();

        outfile << "  to: ";

        while (iter != block.rend()) {
          if (printLineInfo(outfile, *iter)) {
            break;
          }

          ++iter;
        }

        outfile << std::endl;
      }
    }
  }

  return false;
}

// Don't remove below
char SimpleSSD::LLVM::BasicBlockCollector::ID = 0;
static RegisterPass<SimpleSSD::LLVM::BasicBlockCollector> X(
    "blockcollector", "SimpleSSD basic block collector", false, false);

}  // namespace SimpleSSD::LLVM
