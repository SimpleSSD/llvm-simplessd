// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2019 CAMELab
 *
 * Author: Donghyun Gouk <kukdh1@camelab.org>
 */

#include "src/insts/insts.hh"

#include <iostream>
#include <vector>

#include "src/insts/arm/cortex_a57.hh"
#include "src/insts/arm/cortex_r52.hh"

namespace Instruction {

// Global variables
ARM::CortexR52 arm_cortex_r52;
ARM::CortexA57 arm_cortex_a57;

std::vector<Base *> inst_list = {
    &arm_cortex_r52,
    &arm_cortex_a57,
};

Base *initialize(std::string &cpuname) {
  // Find exact match
  for (auto &iter : inst_list) {
    if (cpuname.compare(iter->getName()) == 0) {
      return iter;
    }
  }

#ifdef DEBUG_MODE
  std::cerr << "Not exact-match CPU found for " << cpuname << std::endl;
#endif

  // First one
  return inst_list.front();
}

}  // namespace Instruction
