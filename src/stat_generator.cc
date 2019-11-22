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

struct Line {
  // Instruction count
  uint64_t branch;
  uint64_t load;
  uint64_t store;
  uint64_t arithmetic;
  uint64_t floatingPoint;
  uint64_t otherInsts;

  // Cycle consumed
  uint64_t cycles;

  Line()
      : branch(0),
        load(0),
        store(0),
        arithmetic(0),
        floatingPoint(0),
        otherInsts(0),
        cycles(0) {}
};

struct BasicBlock {
  std::string name;

  std::unordered_map<uint32_t, Line> lines;
};

struct Function {
  std::string name;
  std::string file;
  uint32_t at;

  std::vector<BasicBlock> blocks;
};

namespace Assembly {

struct Line {
  uint64_t branch;
  uint64_t load;
  uint64_t store;
  uint64_t arithmetic;
  uint64_t floatingPoint;
  uint64_t otherInsts;

  uint64_t cycles;

  Line()
      : branch(0),
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

  std::unordered_map<uint32_t, Line> lines;

  Function() : at(0) {}
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

#ifdef DEBUG_MODE
  std::cout << "Loading bbinfo file " << filename << std::endl;
#endif

  // State machine
  // func -> at -> block -> numbers:
  //   |             `--------|
  //   `----------------------'

  std::string line;
  enum STATE {
    IDLE,     // -> FUNC/terminate
    FUNC,     // -> FUNC_AT
    FUNC_AT,  // -> BLOCK
    BLOCK,    // -> IDLE/FUNC_AT/BLOCK
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

  std::smatch match;
  std::regex regex_line("  (\\d+):");

  uint32_t linenumber;

  while (!file.eof()) {
    // Read one line
    std::getline(file, line);

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
        bb->lines.emplace(linenumber, Line());

        // No state change
        continue;
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
        // Empty file
        if (line.length() == 0) {
          break;
        }

        // Expect 'func: <Function name>'
        if (line.compare(0, 6, "func: ") != 0) {
          return false;
        }

        // Create function entry
        list.emplace_back(Function());
        current = &list.back();

        // Store function name
        current->name = std::move(line.substr(6));

#ifdef DEBUG_MODE
        std::cout << " Function: " << current->name << std::endl;
#endif

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

#ifdef DEBUG_MODE
  std::cout << "Loading assembly file " << filename << std::endl;
#endif

  std::string line;
  bool inFunction = false;
  Assembly::Function *current = nullptr;

  std::regex regex_loc(
      "\\s+\\.loc\\s+\\d+\\s+\\d+\\s+\\d+.+[#@] (.+):(\\d+):\\d+",
      std::regex::ECMAScript | std::regex::icase);
  std::regex regex_inst("\\s+([^\\s\\.#@][\\w\\d\\.]*)\\s+.+");
  std::regex regex_func("[#@] -- Begin function (.+)");
  std::regex regex_end("[#@] -- End function");
  std::regex regex_cpu("\\s+\\.cpu\\s+(.+)",
                       std::regex::ECMAScript | std::regex::icase);

  std::smatch match;

  bool lineValid = false;
  std::unordered_map<uint32_t, Assembly::Line>::iterator currentLine;

  while (!file.eof()) {
    std::getline(file, line);

    if (inFunction) {
      if (std::regex_match(line, match, regex_loc)) {
        auto &name = match[1];
        uint32_t row = strtoul(match[2].str().c_str(), nullptr, 10);

        if (current->at == 0) {
          current->file = std::move(name);
          current->at = row;
        }
        else {
          // Ignore file name if different with function file and line 0
          if (current->file.compare(name.str()) == 0 && row != 0) {
            auto ret = current->lines.emplace(row, Assembly::Line());

            currentLine = ret.first;
            lineValid = true;
          }
          else {
            lineValid = false;
          }
        }
      }
      else if (std::regex_match(line, match, regex_inst)) {
        if (isa == nullptr) {
          return false;
        }

        if (!lineValid) {
          continue;
        }

        auto op = match[1].str();

        // Get instruction type and cycle
        uint64_t cycle = 0;
        uint64_t *where = nullptr;
        auto type = isa->getStatistic(op, cycle);

        switch (type) {
          case Instruction::Type::Branch:
            where = &currentLine->second.branch;
            break;
          case Instruction::Type::Load:
            where = &currentLine->second.load;
            break;
          case Instruction::Type::Store:
            where = &currentLine->second.store;
            break;
          case Instruction::Type::Arithmetic:
            where = &currentLine->second.arithmetic;
            break;
          case Instruction::Type::FloatingPoint:
            where = &currentLine->second.floatingPoint;
            break;
          case Instruction::Type::Other:
            where = &currentLine->second.otherInsts;
            break;
          default:
            break;
        }

        if (where) {
          (*where)++;
          currentLine->second.cycles += cycle;
        }
      }
      else if (std::regex_search(line, match, regex_end)) {
        inFunction = false;

        current = nullptr;
        lineValid = false;
      }
    }
    else {
      if (std::regex_search(line, match, regex_func)) {
        auto &name = match[1];

        if (isa == nullptr) {
          std::string cpu("amd64-generic");

          isa = Instruction::initialize(cpu);
        }

        // Append to list
        list.emplace_back(Assembly::Function());
        current = &list.back();

        // Store name
        current->name = std::move(name);

#ifdef DEBUG_MODE
        std::cout << " Function: " << current->name << std::endl;
#endif

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
                       std::vector<Assembly::Function> &asmbbinfo) {
#ifdef DEBUG_MODE
  std::cout << "Generating statistics" << std::endl;
#endif

  // Matching asmbb to bbinfo
  for (auto &irfunc : bbinfo) {
    for (auto &asmfunc : asmbbinfo) {
      // Find function
      if (irfunc.name.compare(asmfunc.name) == 0 && irfunc.at == asmfunc.at) {
#ifdef DEBUG_MODE
        std::cout << "Function: " << irfunc.name << std::endl;
#endif
        // Matching basicblocks
        for (auto &irbb : irfunc.blocks) {
          // Fill each lines with line statistics
          for (auto &irline : irbb.lines) {
            auto asmline = asmfunc.lines.find(irline.first);

            if (asmline != asmfunc.lines.end() && asmline->second.cycles > 0) {
              // Addup stats
              irline.second.branch += asmline->second.branch;
              irline.second.load += asmline->second.load;
              irline.second.store += asmline->second.store;
              irline.second.arithmetic += asmline->second.arithmetic;
              irline.second.floatingPoint += asmline->second.floatingPoint;
              irline.second.otherInsts += asmline->second.otherInsts;
              irline.second.cycles += asmline->second.cycles;
            }
          }
        }

        break;
      }
    }
  }

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

#ifdef DEBUG_MODE
  std::cout << "Saving statistics to file" << filename << std::endl;
#endif

  for (auto &func : list) {
    file << "func: " << func.name << std::endl;
    file << " at: " << func.file << ":" << func.at << std::endl;

    for (auto &block : func.blocks) {
      if (block.lines.size() == 0) {
        continue;
      }

      file << " block: " << block.name << std::endl;

      for (auto &line : block.lines) {
        if (line.second.cycles == 0) {
          continue;
        }

        file << "  " << line.first << ": ";
        file << line.second.branch << ", ";
        file << line.second.load << ", ";
        file << line.second.store << ", ";
        file << line.second.arithmetic << ", ";
        file << line.second.floatingPoint << ", ";
        file << line.second.otherInsts << ", ";
        file << line.second.cycles << std::endl;
      }
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
    return 2;
  }

  if (!parseAssembly(asmfunclist, asmfile, nullptr)) {
    return 3;
  }

  if (!generateStatistic(funclist, asmfunclist)) {
    return 4;
  }

  if (!saveStatistic(funclist, inststat)) {
    return 5;
  }

  return 0;
}
