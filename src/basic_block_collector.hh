// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2019 CAMELab
 *
 * Author: Donghyun Gouk <kukdh1@camelab.org>
 */

#pragma once

#ifndef __SRC_BASIC_BLOCK_COLLECTOR_HH__
#define __SRC_BASIC_BLOCK_COLLECTOR_HH__

#include <fstream>

#include "llvm/Pass.h"
#include "src/util.hh"

namespace SimpleSSD::LLVM {

class BasicBlockCollector : public llvm::FunctionPass, Utility {
 private:
  bool inited;

  std::ofstream outfile;

 public:
  static char ID;

  BasicBlockCollector();

  bool doInitialization(llvm::Module &) override;
  bool runOnFunction(llvm::Function &) override;
  bool doFinalization(llvm::Module &) override;
};

}  // namespace SimpleSSD::LLVM

#endif
