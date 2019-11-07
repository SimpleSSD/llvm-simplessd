// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2019 CAMELab
 *
 * Author: Donghyun Gouk <kukdh1@camelab.org>
 */

#pragma once

#ifndef __SRC_INSTS_ARM_CORTEX_A57_HH__
#define __SRC_INSTS_ARM_CORTEX_A57_HH__

#include "src/insts/insts.hh"

namespace Instruction::ARM {

/**
 * \brief ARM Corted-A57 instruction statistics
 *
 * Check following document:
 *  ARM Cortex-A57 Software Optimization Guide (ARM UAN 0015B)
 */
class CortexA57 : public Base {
 public:
  Type getStatistic(std::string &, uint64_t &) override;
  const char *getName() override { return "cortex-a57"; }
};

}  // namespace Instruction::ARM

#endif
