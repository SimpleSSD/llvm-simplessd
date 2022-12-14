// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2019 CAMELab
 *
 * Author: Donghyun Gouk <kukdh1@camelab.org>
 */

#include "src/util.hh"

#include <cxxabi.h>

#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/Instructions.h"

using namespace llvm;

#define FUNCTION_TYPE_NAME "class.SimpleSSD::CPU::Function"
#define MARK_FUNCION_NAME "_ZN9SimpleSSD3CPU12markFunctionERNS0_8FunctionE"

namespace SimpleSSD::LLVM {

bool Utility::isMarked(Function &func, Value **ppValue, Instruction **ppNext) {
  // Check function
  auto &entry = func.getEntryBlock();

  for (auto &inst : entry) {
    if (auto call = dyn_cast<CallInst>(&inst)) {
      auto callee = call->getCalledFunction();

      if (callee && callee->getName().compare(MARK_FUNCION_NAME) == 0) {
        // Get argument
        if (ppValue) {
          *ppValue = call->getArgOperand(0);
        }

        // Remove this instruction
        auto next = inst.eraseFromParent();

        if (ppNext) {
          *ppNext = &*next;
        }

        return true;
      }
    }
  }

  return false;
}

void Utility::printFunctionName(raw_ostream &os, Function &func) {
  int ret = 0;
  auto mangle = func.getName();
  auto funcname = abi::__cxa_demangle(mangle.data(), nullptr, nullptr, &ret);

  os << mangle;

  if (ret == 0) {
    os << " (" << funcname << ")";
  }

  free(funcname);
}

uint32_t Utility::getLineInfo(Instruction &inst, std::string &file) {
  auto &debug = inst.getDebugLoc();

  // Check it contains valid DILocation
  if (debug.get() && debug.getLine() != 0) {
    auto scope = cast<llvm::DIScope>(debug.getScope());
    file = scope->getFilename().data();
    return debug.getLine();
  }

  return 0;
}

uint32_t Utility::getLineInfo(Function &func, std::string &file) {
  auto subprog = func.getSubprogram();

  if (subprog) {
    file = subprog->getFilename().data();

    return subprog->getLine();
  }

  return 0;
}

bool Utility::printLineInfo(std::ofstream &os, Instruction &inst) {
  std::string file;
  uint32_t line;

  line = getLineInfo(inst, file);

  if (line > 0) {
    os << file << ":" << line;

    return true;
  }

  return false;
}

bool Utility::printLineInfo(std::ofstream &os, Function &func) {
  std::string file;
  uint32_t line;

  line = getLineInfo(func, file);

  if (line > 0) {
    os << file << ":" << line;

    return true;
  }

  return false;
}

uint32_t Utility::getFirstLine(BasicBlock &block, std::string &file) {
  uint32_t line = 0;

  auto iter = block.begin();

  while (iter != block.end()) {
    line = getLineInfo(*iter, file);

    if (line > 0) {
      break;
    }

    ++iter;
  }

  return line;
}

uint32_t Utility::getLastLine(BasicBlock &block, std::string &file) {
  uint32_t line = 0;

  auto iter = block.rbegin();

  while (iter != block.rend()) {
    line = getLineInfo(*iter, file);

    if (line > 0) {
      break;
    }

    ++iter;
  }

  return line;
}

}  // namespace SimpleSSD::LLVM
