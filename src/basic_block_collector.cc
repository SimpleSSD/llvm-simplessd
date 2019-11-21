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
    cl::value_desc("filename"), cl::init(""));

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
  filename += BBC_FILE_POSTFIX;

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

    std::string funcfile;
    uint32_t line;

    // Get function info
    line = getLineInfo(func, funcfile);

    // Write function name
    outfile << "func: " << func.getName().data() << std::endl;
    outfile << " at: " << funcfile << ":" << line << std::endl;

    for (auto &block : func) {
      std::vector<uint32_t> linelist;
      std::string file;

      // Filter blocks by name
      /// All blocks begins with dot (.)
      if (block.getName().str().compare(0, 1, ".") == 0) {
        continue;
      }
      /// All blocks begins with eh (exception handler)
      if (block.getName().str().compare(0, 2, "eh") == 0) {
        continue;
      }
      /// All blocks begins with cleanup
      if (block.getName().str().compare(0, 7, "cleanup") == 0) {
        continue;
      }
      /// All blocks begins with unreach
      if (block.getName().str().compare(0, 7, "unreach") == 0) {
        continue;
      }

      // Skip empty block
      if (block.size() == 0) {
        continue;
      }

      // Reserve for performance
      linelist.reserve(block.size());

      // Write all line information if line info is valid + same module
      for (auto &inst : block) {
        line = getLineInfo(inst, file);

        if (line > 0 && file.compare(funcfile) == 0) {
          linelist.emplace_back(line);
        }
      }

      // Skip empty block
      if (linelist.size() == 0) {
        continue;
      }

      // Sort
      std::sort(linelist.begin(), linelist.end());

      // Unique
      auto end = std::unique(linelist.begin(), linelist.end());

      // Printout
      outfile << " block: " << block.getName().data() << std::endl;

      for (auto iter = linelist.begin(); iter != end; ++iter) {
        outfile << "  " << *iter << ":" << std::endl;
      }
    }

    // We removed markFunction
    return true;
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
    "blockcollector", "SimpleSSD basic block collector", true, false);

}  // namespace SimpleSSD::LLVM
