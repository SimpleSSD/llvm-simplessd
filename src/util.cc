// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2019 CAMELab
 *
 * Author: Donghyun Gouk <kukdh1@camelab.org>
 */

#include "src/util.hh"

#include <cxxabi.h>

#include "llvm/IR/Instructions.h"

using namespace llvm;

#define FUNCTION_TYPE_NAME "class.SimpleSSD::CPU::Function"
#define MARK_FUNCION_NAME "_ZN9SimpleSSD3CPU12markFunctionERNS0_8FunctionE"

namespace SimpleSSD::LLVM {

bool Utility::isMarked(Function &func) {
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

void Utility::printFunctionName(Function &func) {
  int ret = 0;
  auto mangle = func.getName();
  auto funcname = abi::__cxa_demangle(mangle.data(), nullptr, nullptr, &ret);

  outs() << "Collecting basic block information of:\n";
  outs() << "  " << mangle;

  if (ret == 0) {
    outs() << " (" << funcname << ")";
  }

  outs() << "\n";

  free(funcname);
}

}  // namespace SimpleSSD::LLVM
