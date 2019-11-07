// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2019 CAMELab
 *
 * Author: Donghyun Gouk <kukdh1@camelab.org>
 */

#pragma once

#ifndef __SRC_INSTS_ARM_CORTEX_R52_HH__
#define __SRC_INSTS_ARM_CORTEX_R52_HH__

#include "src/insts/insts.hh"

namespace Instruction::ARM {

/**
 * \brief ARM Corted-R52 instruction statistics
 *
 * Check following document:
 *  ARM Cortex-R52 Technical Reference Manual (100026-0102-00)
 */
class CortexR52 : public Base {
 public:
  Type getStatistic(std::string &, uint64_t &) override;
};

}  // namespace Instruction::ARM

#endif
