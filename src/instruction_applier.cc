// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2019 CAMELab
 *
 * Author: Donghyun Gouk <kukdh1@camelab.org>
 */

#include "src/instruction_applier.hh"

#include <string>

#include "llvm/ADT/Statistic.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/CommandLine.h"

#define DEBUG_TYPE "SimpleSSD::LLVM::InstructionApplier"

using namespace llvm;

static cl::opt<std::string> inputFile(
    "inststat-prefix",
    cl::desc("Input file prefix of SimpleSSD instruction statistics"),
    cl::value_desc("filename"), cl::init(""));

namespace SimpleSSD::LLVM {

InstructionApplier::InstructionApplier() : FunctionPass(ID), inited(false) {
#if DEBUG_MODE
  outs() << "SimpleSSD instruction statistic applier.\n";
#endif
}

void InstructionApplier::makePointers(Instruction *next, Value *fstat) {
  // %ptr = getelementptr inbounds %"class.SimpleSSD::CPU::Function",
  // %"class.SimpleSSD::CPU::Function"* %fstat, i32 0, i32 %idx

  // Create builder
  IRBuilder<> builder(next);

  // %idx
  auto idx0 = builder.getInt32(0);
  auto idx1 = builder.getInt32(1);
  auto idx2 = builder.getInt32(2);
  auto idx3 = builder.getInt32(3);
  auto idx4 = builder.getInt32(4);
  auto idx5 = builder.getInt32(5);
  auto idx6 = builder.getInt32(6);

  // IdxList
  Value *idxList0[2] = {idx0, idx0};
  Value *idxList1[2] = {idx0, idx1};
  Value *idxList2[2] = {idx0, idx2};
  Value *idxList3[2] = {idx0, idx3};
  Value *idxList4[2] = {idx0, idx4};
  Value *idxList5[2] = {idx0, idx5};
  Value *idxList6[2] = {idx0, idx6};

  // Add instructions
  pointers.branch = builder.CreateInBoundsGEP(
      fstat, ArrayRef<Value *>(idxList0, 2), "fstat_branch");
  pointers.load = builder.CreateInBoundsGEP(
      fstat, ArrayRef<Value *>(idxList1, 2), "fstat_load");
  pointers.store = builder.CreateInBoundsGEP(
      fstat, ArrayRef<Value *>(idxList2, 2), "fstat_store");
  pointers.arithmetic = builder.CreateInBoundsGEP(
      fstat, ArrayRef<Value *>(idxList3, 2), "fstat_arithmetic");
  pointers.floating = builder.CreateInBoundsGEP(
      fstat, ArrayRef<Value *>(idxList4, 2), "fstat_floating");
  pointers.other = builder.CreateInBoundsGEP(
      fstat, ArrayRef<Value *>(idxList5, 2), "fstat_other");
  pointers.cycles = builder.CreateInBoundsGEP(
      fstat, ArrayRef<Value *>(idxList6, 2), "fstat_cycles");
}

void InstructionApplier::makeAdd(llvm::Instruction *next, Value *target,
                                 uint64_t value) {
  // %reg = load i64, i64* %target, align 8
  // %add = add i64 %reg, %value
  // store i64 %add, i64* %target, align 8

  // Create builder
  IRBuilder<> builder(next);

  // Load
  auto load = builder.CreateLoad(target);
  load->setAlignment(8);

  // Add
  auto add = builder.CreateAdd(builder.getInt64(value), load);

  // Store
  auto store = builder.CreateStore(add, target);
  store->setAlignment(8);
}

void InstructionApplier::parseStatFile() {
  // State machine
  // func -> at -> block -> from -> to -> stat
  //   |             `----------------------|
  //   `------------------------------------'

  std::string line;
  enum STATE {
    IDLE,        // -> FUNC/terminate
    FUNC,        // -> FUNC_AT
    FUNC_AT,     // -> BLOCK
    BLOCK,       // -> BLOCK_FROM
    BLOCK_FROM,  // -> BLOCK_TO
    BLOCK_TO,    // -> BLOCK_STAT
    BLOCK_STAT,  // -> IDLE/FUNC
  } state = IDLE;
  FuncStat *current = nullptr;
  BlockStat *bb = nullptr;

  auto parseFile = [](std::string &line, std::string &file,
                      size_t from) -> uint32_t {
    auto idx = line.find_last_of(':');

    if (idx == std::string::npos) {
      return 0;
    }
    else {
      file = std::move(line.substr(from, idx - from));

      return strtoul(line.substr(idx + 1).c_str(), nullptr, 10);
    }
  };

  while (!infile.eof()) {
    // Read one line
    std::getline(infile, line);

    if (state == BLOCK_STAT) {
      if (line.compare(0, 6, "func: ") == 0) {
        state = IDLE;
      }
      else if (line.compare(0, 8, " block: ") == 0) {
        state = FUNC_AT;
      }
      else if (line.length() == 0) {
        break;
      }
      else {
        return;
      }
    }

    switch (state) {
      case IDLE: {
        // Expect 'func: <Function name>'
        if (line.compare(0, 6, "func: ") != 0) {
          return;
        }

        // Create function entry
        funclist.emplace_back(FuncStat());
        current = &funclist.back();

        // Store function name
        current->name = std::move(line.substr(6));

#ifdef DEBUG_MODE
        outs() << "Function: " << current->name << "\n";
#endif

        state = FUNC;
      } break;
      case FUNC:
        // Expect ' at: [filename:line]'
        if (line.compare(0, 5, " at: ") != 0) {
          return;
        }

        // Store source info
        current->at = parseFile(line, current->file, 5);

        state = FUNC_AT;

        break;
      case FUNC_AT:
        // Expect ' block: <basic block name>'
        if (line.compare(0, 8, " block: ") != 0) {
          return;
        }

        // Create basicblock entry
        current->blocks.emplace_back(BlockStat());
        bb = &current->blocks.back();

        // Store basicblock name
        bb->name = std::move(line.substr(8));

#ifdef DEBUG_MODE
        outs() << " BasicBlock: " << bb->name << "\n";
#endif

        state = BLOCK;

        break;
      case BLOCK:
        // Expect '  from: [filename:line]'
        if (line.compare(0, 8, "  from: ") != 0) {
          return;
        }

        // Store source info
        bb->begin = parseFile(line, bb->from, 8);

        state = BLOCK_FROM;

        break;
      case BLOCK_FROM:
        // Expect '  to: [filename:line]'
        if (line.compare(0, 6, "  to: ") != 0) {
          return;
        }

        // Store source info
        bb->end = parseFile(line, bb->to, 6);

        state = BLOCK_TO;

        break;
      case BLOCK_TO: {
        // Expect '  stat: <stat>'
        if (line.compare(0, 8, "  stat: ") != 0) {
          return;
        }

        // Parse stat
        char *str = (char *)line.c_str() + 8;
        char *last = nullptr;

        bb->branch = strtoul(str, &last, 10);
        str = last + 1;
        bb->load = strtoul(str, &last, 10);
        str = last + 1;
        bb->store = strtoul(str, &last, 10);
        str = last + 1;
        bb->arithmetic = strtoul(str, &last, 10);
        str = last + 1;
        bb->floatingPoint = strtoul(str, &last, 10);
        str = last + 1;
        bb->otherInsts = strtoul(str, &last, 10);
        str = last + 1;
        bb->cycles = strtoul(str, nullptr, 10);

#ifdef DEBUG_MODE
        outs() << "  Stat: " << bb->branch << ", " << bb->load << ", "
               << bb->store << ", " << bb->arithmetic << ", "
               << bb->floatingPoint << ", " << bb->otherInsts << ", "
               << bb->cycles << ", "
               << "\n";
#endif

        state = BLOCK_STAT;
      } break;
      default:
        return;
    }
  }

  inited = true;
}

bool InstructionApplier::doInitialization(Module &module) {
  std::string filename(inputFile);

  // Get module name
  auto name = module.getName().data();

  // Make filename
  filename += name;
  filename += IA_FILE_POSTFIX;

#if DEBUG_MODE
  outs() << " Input filename: " << filename << "\n";
#endif

  // Validate file
  infile.open(filename);

  if (infile.is_open()) {
    parseStatFile();
  }
  else {
    errs() << " Failed to open file: " << filename << "\n";
  }

  return false;
}

bool InstructionApplier::runOnFunction(Function &func) {
  if (!inited) {
    return false;
  }

  Value *fstat = nullptr;
  Instruction *next = nullptr;

  if (isMarked(func, &fstat, &next)) {
#ifdef DEBUG_MODE
    outs() << "Handling function: ";

    printFunctionName(outs(), func);

    outs() << ", storing statistics to value: ";
    outs() << fstat->getName() << ".\n";
#endif

    std::string from;
    std::string to;
    uint32_t begin;
    uint32_t end;

    // Find function
    auto iter = funclist.begin();

    // Match name
    for (; iter != funclist.end(); ++iter) {
      if (iter->name.compare(func.getName().data()) == 0) {
        break;
      }
    }

    if (iter == funclist.end()) {
      // (u)int64_t is different in 32bit ((unsigned) long long) and 64bit
      // ((unsigned) long), introducing different C++ mangled name.
      // Just match with file name and line number.
      std::string ffile;
      uint32_t fline;

      fline = getLineInfo(func, ffile);

      if (fline > 0) {
        for (iter = funclist.begin(); iter != funclist.end(); ++iter) {
          if (iter->file.compare(ffile) == 0 && iter->at == fline) {
            break;
          }
        }
      }
    }

    if (iter != funclist.end()) {
      // Setup pointers of fstat
      makePointers(next, fstat);

      // Apply instruction stats
      for (auto &block : func) {
        auto &last = block.back();

        // Get line info
        begin = getFirstLine(block, from);
        end = getLastLine(block, to);

        // Find block
        auto stat = iter->blocks.begin();

        for (; stat != iter->blocks.end(); ++stat) {
          if (stat->begin <= begin && end <= stat->end) {
            break;
          }
        }

        if (stat == iter->blocks.end()) {
          errs() << "Function: ";
          printFunctionName(errs(), func);
          errs() << "\n BasicBlock: " << block.getName() << " (" << begin << ":"
                 << end << ") is not found in instruction statistic file.\n";

          continue;
        }

        // Always use filename and line info -- may have different CFG
        if (stat->name.compare(block.getName().data()) == 0) {
          if (stat->branch > 0) {
            makeAdd(&last, pointers.branch, stat->branch);
          }
          if (stat->load > 0) {
            makeAdd(&last, pointers.load, stat->load);
          }
          if (stat->store > 0) {
            makeAdd(&last, pointers.store, stat->store);
          }
          if (stat->arithmetic > 0) {
            makeAdd(&last, pointers.arithmetic, stat->arithmetic);
          }
          if (stat->floatingPoint > 0) {
            makeAdd(&last, pointers.floating, stat->floatingPoint);
          }
          if (stat->otherInsts > 0) {
            makeAdd(&last, pointers.other, stat->otherInsts);
          }
          if (stat->cycles > 0) {
            makeAdd(&last, pointers.cycles, stat->cycles);
          }
        }
      }

      // Verify function
      if (verifyFunction(func, &errs())) {
        func.dump();
      }
    }
    else {
      errs() << "Function: ";
      printFunctionName(errs(), func);
      errs() << " is not found in instruction statistic file.\n";
    }

    return true;
  }

  return false;
}

bool InstructionApplier::doFinalization(Module &) {
  if (inited) {
    infile.close();
  }

  inited = false;

  return false;
}

// Don't remove below
char SimpleSSD::LLVM::InstructionApplier::ID = 0;
static RegisterPass<SimpleSSD::LLVM::InstructionApplier> X(
    "inststat", "SimpleSSD instruction statistics applier", true, false);

}  // namespace SimpleSSD::LLVM
