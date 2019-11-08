// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2019 CAMELab
 *
 * Author: Donghyun Gouk <kukdh1@camelab.org>
 */

#include <fstream>
#include <iostream>
#include <regex>
#include <string>
#include <vector>

#include "insts/insts.hh"
#include "src/def.hh"

struct BasicBlock {
  bool skip;

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
      : skip(false),
        begin(0),
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

namespace Assembly {

struct BasicBlock {
  uint32_t id;
  std::string name;

  uint32_t begin;
  uint32_t end;

  uint64_t branch;
  uint64_t load;
  uint64_t store;
  uint64_t arithmetic;
  uint64_t floatingPoint;
  uint64_t otherInsts;

  uint64_t cycles;

  BasicBlock()
      : id(0),
        begin(std::numeric_limits<uint32_t>::max()),
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

}  // namespace Assembly

bool loadBasicBlockInfo(std::vector<Function> &list, std::string filename) {
  std::ifstream file(filename);

  if (!file.is_open()) {
#ifdef DEBUG_MODE
    std::cerr << "Failed to open file " << filename << std::endl;
#endif
    return false;
  }

  // State machine
  // func -> at -> block -> size -> from -> to
  //   |             `----------------------|
  //   `------------------------------------'

  std::string line;
  enum STATE {
    IDLE,        // -> FUNC/terminate
    FUNC,        // -> FUNC_AT
    FUNC_AT,     // -> BLOCK
    BLOCK,       // -> BLOCK_SIZE
    BLOCK_SIZE,  // -> BLOCK_FROM
    BLOCK_FROM,  // -> BLOCK_TO
    BLOCK_TO,    // -> IDLE/FUNC_AT
  } state = IDLE;
  Function *current = nullptr;
  BasicBlock *bb = nullptr;

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

  while (!file.eof()) {
    // Read one line
    std::getline(file, line);

    if (state == BLOCK_TO) {
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
        return false;
      }
    }

    switch (state) {
      case IDLE:
        // Expect 'func: <Function name>'
        if (line.compare(0, 6, "func: ") != 0) {
          return false;
        }

        // Create function entry
        list.emplace_back(Function());
        current = &list.back();

        // Store function name
        current->name = std::move(line.substr(6));

        state = FUNC;

        break;
      case FUNC:
        // Expect ' at: [filename:line]'
        if (line.compare(0, 5, " at: ") != 0) {
          return false;
        }

        // Store filename
        current->at = parseFile(line, current->file, 5);

        state = FUNC_AT;

        break;
      case FUNC_AT:
        // Expect ' block: <basic block name>'
        if (line.compare(0, 8, " block: ") != 0) {
          return false;
        }

        // Create basicblock entry
        current->blocks.emplace_back(BasicBlock());
        bb = &current->blocks.back();

        // Store basicblock name
        bb->name = std::move(line.substr(8));

        state = BLOCK;

        break;
      case BLOCK:
        // Expect '  size: <IR count>'
        if (line.compare(0, 8, "  size: ") != 0) {
          return false;
        }

        state = BLOCK_SIZE;

        break;
      case BLOCK_SIZE:
        // Expect '  from: [filename:line]'
        if (line.compare(0, 8, "  from: ") != 0) {
          return false;
        }

        // Store source info
        bb->begin = parseFile(line, bb->from, 8);

        state = BLOCK_FROM;

        break;
      case BLOCK_FROM:
        // Expect '  to: [filename:line]'
        if (line.compare(0, 6, "  to: ") != 0) {
          return false;
        }

        // Store source info
        bb->end = parseFile(line, bb->to, 6);

        state = BLOCK_TO;

        break;
      default:
        return false;
    }
  }

  return true;
}

bool parseAssembly(std::vector<Assembly::Function> &list, std::string filename,
                   Instruction::Base *isa) {
  std::ifstream file(filename);

  if (!file.is_open()) {
#ifdef DEBUG_MODE
    std::cerr << "Failed to open file " << filename << std::endl;
#endif
    return false;
  }

  std::string line;
  bool inFunction = false;
  Assembly::Function *current = nullptr;
  Assembly::BasicBlock *bb = nullptr;

  std::regex regex_loc(
      "\\s+\\.loc\\s+\\d+\\s+\\d+\\s+\\d+\\s+[#@] (.+):(\\d+):\\d+");
  std::regex regex_bb("[#@] %bb\\.(\\d+):\\s+[#@] %(.+)");
  std::regex regex_inst("\\s+([^\\.][\\w\\d\\.]*)\\s+.+");
  std::regex regex_func("[#@] -- Begin function (.+)");
  std::regex regex_end("@ -- End function");
  std::regex regex_cpu("\\s+\\.cpu\\s+(.+)");

  std::smatch match;

  while (!file.eof()) {
    std::getline(file, line);

    if (inFunction) {
      if (std::regex_match(line, match, regex_loc)) {
        auto &name = match[1];
        uint32_t row = strtoul(match[2].str().c_str(), nullptr, 10);

#ifdef DEBUG_MODE
        std::cout << "Location: " << name << ":" << row << std::endl;
#endif

        if (bb == nullptr) {
          current->name = std::move(name);
          current->at = row;
        }
        else {
          // Ignore file name if different with function file
          if (current->name.compare(name.str()) == 0) {
            // Store min value to begin
            if (bb->begin > row) {
              bb->begin = row;
            }

            // Store max value to end
            if (bb->end < row) {
              bb->end = row;
            }
          }
        }
      }
      else if (std::regex_match(line, match, regex_bb)) {
        uint32_t id = strtoul(match[1].str().c_str(), nullptr, 10);
        auto &name = match[2];

#ifdef DEBUG_MODE
        std::cout << "BasicBlock: " << name << " (" << id << ")" << std::endl;
#endif

        // Append to list
        current->blocks.emplace_back(Assembly::BasicBlock());
        bb = &current->blocks.back();

        // Store name and id
        bb->name = std::move(name);
        bb->id = id;
      }
      else if (std::regex_match(line, match, regex_inst)) {
        if (isa == nullptr) {
          return false;
        }

        auto op = match[1].str();

#ifdef DEBUG_MODE
        std::cout << "Instruction: " << op << std::endl;
#endif

        // Get instruction type and cycle
        uint64_t cycle = 0;
        uint64_t *where = nullptr;
        auto type = isa->getStatistic(op, cycle);

        switch (type) {
          case Instruction::Type::Branch:
            where = &bb->branch;
            break;
          case Instruction::Type::Load:
            where = &bb->load;
            break;
          case Instruction::Type::Store:
            where = &bb->store;
            break;
          case Instruction::Type::Arithmetic:
            where = &bb->arithmetic;
            break;
          case Instruction::Type::FloatingPoint:
            where = &bb->floatingPoint;
            break;
          case Instruction::Type::Other:
            where = &bb->otherInsts;
            break;
          default:
            break;
        }

        if (where) {
          (*where)++;
          bb->cycles += cycle;
        }
      }
      else if (std::regex_search(line, match, regex_end)) {
        inFunction = false;
        current = nullptr;
        bb = nullptr;
      }
    }
    else {
      if (std::regex_search(line, match, regex_func)) {
        auto &name = match[1];

#ifdef DEBUG_MODE
        std::cout << "Function: " << name << std::endl;
#endif

        // Append to list
        list.emplace_back(Assembly::Function());
        current = &list.back();

        // Store name
        current->name = std::move(name);

        inFunction = true;
      }
      else if (isa == nullptr && std::regex_match(line, match, regex_cpu)) {
        auto cpu = match[1].str();

        isa = Instruction::initialize(cpu);
      }
    }
  }

  return !inFunction;
}

bool generateStatistic(std::vector<Function> &bbinfo,
                       std::vector<Assembly::Function> &asmbb) {

  return true;
}

bool saveStatistic(std::vector<Function> &list, std::string filename) {
  std::ofstream file(filename);

  if (!file.is_open()) {
#ifdef DEBUG_MODE
    std::cerr << "Failed to open file " << filename << std::endl;
#endif
    return false;
  }

  for (auto &func : list) {
    file << "func: " << func.name << std::endl;

    for (auto &block : func.blocks) {
      if (block.skip) {
        continue;
      }

      file << " block: " << block.name << std::endl;
      file << " from: " << block.from << ":" << block.begin << std::endl;
      file << " to: " << block.to << ":" << block.end << std::endl;
      file << " stat: ";
      file << block.branch << ", ";
      file << block.load << ", ";
      file << block.store << ", ";
      file << block.arithmetic << ", ";
      file << block.floatingPoint << ", ";
      file << block.otherInsts << ", ";
      file << block.cycles << std::endl;
    }
  }

  return true;
}

int main(int argc, char *argv[]) {
  std::string bbinfo;
  std::string asmfile;
  std::string inststat;

  switch (argc) {
    case 2:
#ifdef DEBUG_MODE
      std::cout << "From module name: " << argv[1] << std::endl;
#endif

      bbinfo = argv[1];
      bbinfo += BBC_FILE_POSTFIX;

      asmfile = argv[1];
      asmfile += ASM_FILE_POSTFIX;

      inststat = argv[1];
      inststat += IA_FILE_POSTFIX;
      break;
    default:
#ifdef DEBUG_MODE
      std::cerr << "Invalid number of arguments" << std::endl;
      std::cerr << " Usage: " << argv[0] << " <module file name>" << std::endl;
#endif
      return 1;
  }

  std::vector<Function> funclist;
  std::vector<Assembly::Function> asmfunclist;

  if (!loadBasicBlockInfo(funclist, bbinfo)) {
    return 1;
  }

  if (!parseAssembly(asmfunclist, asmfile, nullptr)) {
    return 1;
  }

  if (!generateStatistic(funclist, asmfunclist)) {
    return 1;
  }

  if (!saveStatistic(funclist, inststat)) {
    return 1;
  }

  return 0;
}
