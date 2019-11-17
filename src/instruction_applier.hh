// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2019 CAMELab
 *
 * Author: Donghyun Gouk <kukdh1@camelab.org>
 */

#pragma once

#ifndef __SRC_INSTRUCTION_APPLIER_HH__
#define __SRC_INSTRUCTION_APPLIER_HH__

#include <fstream>
#include <string>
#include <vector>

#include "llvm/Pass.h"
#include "src/util.hh"

namespace SimpleSSD::LLVM {

struct BlockStat {
  std::string name;

  std::string from;
  std::string to;
  uint32_t begin;
  uint32_t end;

  // Instruction count
  uint64_t branch;
  uint64_t load;
  uint64_t store;
  uint64_t arithmetic;
  uint64_t floatingPoint;
  uint64_t otherInsts;

  // Cycle consumed
  uint64_t cycles;
};

struct FuncStat {
  std::string name;
  std::string file;
  uint32_t at;

  std::vector<BlockStat> blocks;
};

/**
 * \brief Instruction statistics applier
 *
 * This LLVM Pass reads text file contains instruction statistics and insert
 * LLVM IR to increase instruction counter and cycles variables in CPU::Function
 * class.
 */
class InstructionApplier : public llvm::FunctionPass, Utility {
 private:
  bool inited;

  std::ifstream infile;
  std::ofstream resultfile;
  std::vector<FuncStat> funclist;

  struct {
    llvm::Value *branch;
    llvm::Value *load;
    llvm::Value *store;
    llvm::Value *arithmetic;
    llvm::Value *floating;
    llvm::Value *other;
    llvm::Value *cycles;
  } pointers;

  void makePointers(llvm::Instruction *, llvm::Value *);
  void makeAdd(llvm::Instruction *, llvm::Value *, uint64_t);

  void parseStatFile();

 public:
  static char ID;

  InstructionApplier();

  bool doInitialization(llvm::Module &) override;
  bool runOnFunction(llvm::Function &) override;
  bool doFinalization(llvm::Module &) override;
};

}  // namespace SimpleSSD::LLVM

#endif
