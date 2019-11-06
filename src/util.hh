// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2019 CAMELab
 *
 * Author: Donghyun Gouk <kukdh1@camelab.org>
 */

#pragma once

#ifndef __SRC_UTIL_HH__
#define __SRC_UTIL_HH__

#include "llvm/Pass.h"

namespace SimpleSSD::LLVM {

class Utility {
 protected:
  static bool isMarked(llvm::Function &);
};

}  // namespace SimpleSSD::LLVM

#endif
