// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2019 CAMELab
 *
 * Author: Donghyun Gouk <kukdh1@camelab.org>
 */

#pragma once

#ifndef __SRC_BASIC_BLOCK_COLLECTOR_HH__
#define __SRC_BASIC_BLOCK_COLLECTOR_HH__

#include "llvm/Pass.h"
#include "src/util.hh"

namespace SimpleSSD::LLVM {

class BasicBlockCollector : public llvm::FunctionPass, Utility {
 public:
  static char ID;

  BasicBlockCollector();

  bool runOnFunction(llvm::Function &) override;
  void getAnalysisUsage(llvm::AnalysisUsage &) const override;
};

}  // namespace SimpleSSD::LLVM

#endif