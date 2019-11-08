// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2019 CAMELab
 *
 * Author: Donghyun Gouk <kukdh1@camelab.org>
 */

#pragma once

#ifndef __SRC_UTIL_HH__
#define __SRC_UTIL_HH__

#include <fstream>

#include "llvm/IR/Instructions.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "src/def.hh"

namespace SimpleSSD::LLVM {

class Utility {
 protected:
  static bool isMarked(llvm::Function &, llvm::Value ** = nullptr,
                       llvm::Instruction ** = nullptr);
  static void printFunctionName(llvm::raw_ostream &, llvm::Function &);

  static uint32_t getLineInfo(llvm::Instruction &, std::string &);
  static uint32_t getLineInfo(llvm::Function &, std::string &);
  static bool printLineInfo(std::ofstream &, llvm::Instruction &);
  static bool printLineInfo(std::ofstream &, llvm::Function &);

  static uint32_t getFirstLine(llvm::BasicBlock &, std::string &);
  static uint32_t getLastLine(llvm::BasicBlock &, std::string &);
};

}  // namespace SimpleSSD::LLVM

#endif
