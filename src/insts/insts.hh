// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2019 CAMELab
 *
 * Author: Donghyun Gouk <kukdh1@camelab.org>
 */

#pragma once

#ifndef __SRC_INSTS_INSTS_HH__
#define __SRC_INSTS_INSTS_HH__

#include <cinttypes>
#include <regex>
#include <string>
#include <unordered_map>

namespace Instruction {

enum class Type : uint8_t {
  Branch,
  Load,
  Store,
  Arithmetic,
  FloatingPoint,
  Other,
  Ignore,
};

struct Rule {
  const Type type;
  const uint16_t cycle;
  std::regex regex;

  Rule(const std::string &r, Type t, uint16_t c) : Rule(r.c_str(), t, c) {}
  Rule(const char *s, Type t, uint16_t c) : type(t), cycle(c) {
    regex = std::regex(s, std::regex::ECMAScript | std::regex::icase);
  }
};

using RuleList = std::unordered_multimap<char, Rule>;

class Base {
 public:
  virtual Type getStatistic(std::string &, uint64_t &);
};

};  // namespace Instruction

#endif
