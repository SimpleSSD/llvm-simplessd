// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2019 CAMELab
 *
 * Author: Donghyun Gouk <kukdh1@camelab.org>
 */

#include "src/instruction_applier.hh"

#include <regex>
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
#if LLVM_VERSION_MAJOR >= 10
  load->setAlignment(Align(8));
#else
  load->setAlignment(8);
#endif

  // Add
  auto add = builder.CreateAdd(builder.getInt64(value), load);

  // Store
  auto store = builder.CreateStore(add, target);
#if LLVM_VERSION_MAJOR >= 10
  store->setAlignment(Align(8));
#else
  store->setAlignment(8);
#endif
}

void InstructionApplier::parseStatFile() {
  // State machine
  // func -> at -> block -> number:
  //   |             `--------|
  //   `----------------------'

  std::string line;
  enum STATE {
    IDLE,     // -> FUNC/terminate
    FUNC,     // -> FUNC_AT
    FUNC_AT,  // -> BLOCK
    BLOCK,    // -> IDLE/FUNC_AT/BLOCK
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

  std::smatch match;
  std::regex regex_line(
      "  (\\d+): (\\d+), (\\d+), (\\d+), (\\d+), (\\d+), (\\d+), (\\d+)");

  uint32_t linenumber;

  while (!infile.eof()) {
    // Read one line
    std::getline(infile, line);

    if (state == BLOCK) {
      if (line.compare(0, 6, "func: ") == 0) {
        state = IDLE;
      }
      else if (line.compare(0, 8, " block: ") == 0) {
        state = FUNC_AT;
      }
      else if (std::regex_match(line, match, regex_line)) {
        // Expect '  %u:'
        linenumber = strtoul(match[1].str().c_str(), nullptr, 10);

        auto ret = current->lines.emplace(linenumber, LineStat());

        ret.first->second.branch +=
            strtoul(match[2].str().c_str(), nullptr, 10);
        ret.first->second.load += strtoul(match[3].str().c_str(), nullptr, 10);
        ret.first->second.store += strtoul(match[4].str().c_str(), nullptr, 10);
        ret.first->second.arithmetic +=
            strtoul(match[5].str().c_str(), nullptr, 10);
        ret.first->second.floatingPoint +=
            strtoul(match[6].str().c_str(), nullptr, 10);
        ret.first->second.otherInsts +=
            strtoul(match[7].str().c_str(), nullptr, 10);
        ret.first->second.cycles +=
            strtoul(match[8].str().c_str(), nullptr, 10);

        // Link to BB
        if (bb) {
          bb->lines.emplace_back(linenumber);
        }

        // No state change
        continue;
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

    filename += ".log";

    resultfile.open(filename);
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

    std::string ffile;
    std::string file;
    uint32_t fline = 0;
    uint32_t line;

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
      auto &funcstat = *iter;

      // Setup pointers of fstat
      makePointers(next, fstat);

      // Log result if possible
      if (resultfile.is_open()) {
        resultfile << "Function: " << func.getName().data() << std::endl;

        // Only print when file info is valid
        if (fline > 0) {
          resultfile << " at: " << ffile << ":" << fline << std::endl;
        }
      }

      // Apply instruction stats ...
      for (auto &block : func) {
        // Total stat of current basic block
        LineStat sum;

        // Where we need to insert stats
        auto &last = block.back();

        // ... from each lines
        for (auto &inst : block) {
          // Get line info
          line = getLineInfo(inst, file);

          if (line > 0 && ffile.compare(file) == 0) {
            // Find line from database
            auto stat = funcstat.lines.find(line);

            if (stat != funcstat.lines.end() && stat->second.cycles > 0) {
              // Sum stat value to sum
              sum.branch += stat->second.branch;
              sum.load += stat->second.load;
              sum.store += stat->second.store;
              sum.arithmetic += stat->second.arithmetic;
              sum.floatingPoint += stat->second.floatingPoint;
              sum.otherInsts += stat->second.otherInsts;
              sum.cycles += stat->second.cycles;

              stat->second.cycles = 0;
            }
          }
        }

        if (sum.cycles == 0) {
          // Current block does not have line information, use old method
          uint32_t begin = getFirstLine(block, file);
          uint32_t end = getLastLine(block, file);

          for (auto &bbstat : funcstat.blocks) {
            if (bbstat.lines.size() == 0) {
              continue;
            }

            // Sort lines
            std::sort(bbstat.lines.begin(), bbstat.lines.end());

            // Match
            if (bbstat.lines.front() <= begin && end <= bbstat.lines.back()) {
              for (auto &bbline : bbstat.lines) {
                auto bblinestat = funcstat.lines.find(bbline);

                if (bblinestat != funcstat.lines.end() &&
                    bblinestat->second.cycles > 0) {
                  // Sum stat value to sum
                  sum.branch += bblinestat->second.branch;
                  sum.load += bblinestat->second.load;
                  sum.store += bblinestat->second.store;
                  sum.arithmetic += bblinestat->second.arithmetic;
                  sum.floatingPoint += bblinestat->second.floatingPoint;
                  sum.otherInsts += bblinestat->second.otherInsts;
                  sum.cycles += bblinestat->second.cycles;

                  bblinestat->second.cycles = 0;
                }
              }

              break;
            }
          }

          if (sum.cycles == 0) {
            // Giving up
            continue;
          }
        }

        // Log result if possible
        if (resultfile.is_open()) {
          resultfile << " BasicBlock: " << block.getName().data() << std::endl;
          resultfile << "  Stat: " << sum.branch << ", " << sum.load << ", "
                     << sum.store << ", " << sum.arithmetic << ", "
                     << sum.floatingPoint << ", " << sum.otherInsts << ", "
                     << sum.cycles << std::endl;
        }

        if (sum.branch > 0) {
          makeAdd(&last, pointers.branch, sum.branch);
        }
        if (sum.load > 0) {
          makeAdd(&last, pointers.load, sum.load);
        }
        if (sum.store > 0) {
          makeAdd(&last, pointers.store, sum.store);
        }
        if (sum.arithmetic > 0) {
          makeAdd(&last, pointers.arithmetic, sum.arithmetic);
        }
        if (sum.floatingPoint > 0) {
          makeAdd(&last, pointers.floating, sum.floatingPoint);
        }
        if (sum.otherInsts > 0) {
          makeAdd(&last, pointers.other, sum.otherInsts);
        }
        if (sum.cycles > 0) {
          makeAdd(&last, pointers.cycles, sum.cycles);
        }
      }

      // Verify function
      if (verifyFunction(func, &errs())) {
        func.dump();
      }
    }
    else {
#ifdef DEBUG_MODE
      errs() << "Function: ";
      printFunctionName(errs(), func);
      errs() << " is not found in instruction statistic file.\n";
#endif
    }

    return true;
  }

  return false;
}

bool InstructionApplier::doFinalization(Module &) {
  if (inited) {
    infile.close();

    if (resultfile.is_open()) {
      resultfile.close();
    }
  }

  inited = false;

  return false;
}

// Don't remove below
char SimpleSSD::LLVM::InstructionApplier::ID = 0;
static RegisterPass<SimpleSSD::LLVM::InstructionApplier> X(
    "inststat", "SimpleSSD instruction statistics applier", true, false);

}  // namespace SimpleSSD::LLVM
