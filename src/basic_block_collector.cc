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
#include "llvm/IR/Module.h"
#include "llvm/Support/CommandLine.h"

#define DEBUG_TYPE "SimpleSSD::LLVM::BasicBlockCollector"

using namespace llvm;

static cl::opt<std::string> outputFile(
    "blockcollector-prefix",
    cl::desc("Output file prefix of SimpleSSD basic block infomation"),
    cl::value_desc("filename"), cl::init("bbinfo_"));

namespace SimpleSSD::LLVM {

BasicBlockCollector::BasicBlockCollector() : FunctionPass(ID), inited(false) {
#if DEBUG_MODE
  outs() << "SimpleSSD instruction statistic generator.\n";
#endif
}

bool BasicBlockCollector::doInitialization(Module &module) {
  std::string filename(outputFile);

  // Get module name
  auto name = module.getName().data();

  // Make filename
  filename += name;
  filename += ".txt";

#if DEBUG_MODE
  outs() << " Output filename: " << filename << "\n";
#endif

  // Validate file
  outfile.open(filename);

  if (outfile.is_open()) {
    inited = true;
  }
  else {
    errs() << " Failed to open file: " << filename << "\n";
  }

  return false;
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

bool BasicBlockCollector::doFinalization(Module &) {
  if (inited) {
    outfile.close();
  }

  inited = false;

  return false;
}

// Don't remove below
char SimpleSSD::LLVM::BasicBlockCollector::ID = 0;
static RegisterPass<SimpleSSD::LLVM::BasicBlockCollector> X(
    "blockcollector", "SimpleSSD basic block collector", false, false);

}  // namespace SimpleSSD::LLVM
