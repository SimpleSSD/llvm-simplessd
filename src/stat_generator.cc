// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2019 CAMELab
 *
 * Author: Donghyun Gouk <kukdh1@camelab.org>
 */

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "src/def.hh"

struct BasicBlock {
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

  BasicBlock()
      : begin(0),
        end(0),
        branch(0),
        load(0),
        store(0),
        arithmetic(0),
        floatingPoint(0),
        otherInsts(0),
        cycles(0) {}
};

struct Function {
  std::string name;
  std::string file;
  uint32_t at;

  std::vector<BasicBlock> blocks;
};

bool loadBasicBlockInfo(std::vector<Function> &list, std::string filename) {
  return true;
}

bool generateStatistic(std::vector<Function> &list, std::string filename) {
  return true;
}

bool saveStatistic(std::vector<Function> &list, std::string filename) {
  return true;
}

int main(int argc, char *argv[]) {
  std::string bbinfo;
  std::string asmfile;
  std::string inststat;

  if (argc == 2) {
#ifdef DEBUG_MODE
    std::cout << "From module name: " << argv[1] << std::endl;
#endif

    bbinfo = argv[1];
    bbinfo += BBC_FILE_POSTFIX;

    asmfile = argv[1];
    asmfile += ASM_FILE_POSTFIX;

    inststat = argv[1];
    inststat += IA_FILE_POSTFIX;
  }
  else {
#ifdef DEBUG_MODE
    std::cerr << "Invalid number of arguments" << std::endl;
    std::cerr << " Usage: " << argv[0] << " <module file name>" << std::endl;
#endif
    return 1;
  }

  std::vector<Function> funclist;

  if (!loadBasicBlockInfo(funclist, bbinfo)) {
    return 1;
  }

  if (!generateStatistic(funclist, asmfile)) {
    return 1;
  }

  if (!saveStatistic(funclist, inststat)) {
    return 1;
  }

  return 0;
}
