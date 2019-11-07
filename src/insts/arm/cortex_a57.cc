// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2019 CAMELab
 *
 * Author: Donghyun Gouk <kukdh1@camelab.org>
 */

#include "src/insts/arm/cortex_a57.hh"

namespace Instruction::ARM {

RuleList rule_a57 = {
    {'N', {"NOP", Type::Other, 1}},
    {'C', {"CAS[B|H|P|]", Type::Other, 1}},
    {'S', {"SWP[B|H|]", Type::Other, 1}},
    {'B',
     {"B\\.(EQ|NE|CS|HS|CC|LO|MI|PL|VS|VC|HI|LS|GE|LT|GT|LE|AL|NV)",
      Type::Branch, 1}},
    {'C', {"CBN?Z", Type::Branch, 1}},
    {'T', {"TBN?Z", Type::Branch, 1}},
    {'B', {"B", Type::Branch, 1}},
    {'B', {"BL", Type::Branch, 1}},
    {'B', {"BLR", Type::Branch, 2}},
    {'B', {"BR", Type::Branch, 1}},
    {'R', {"RET", Type::Branch, 1}},
    {'L', {"LDR(B|SB|H|SH|SW|)", Type::Load, 4}},
    {'L', {"LDUR(B|SB|H|SH|SW|)", Type::Load, 4}},
    {'L', {"LDP(SW|)", Type::Load, 4}},
    {'L', {"LDNP", Type::Load, 4}},
    {'L', {"LDTR(B|SB|H|SH|SW|)", Type::Load, 4}},
    {'L', {"LDXR(B|H|)", Type::Load, 4}},
    {'L', {"LDXP", Type::Load, 4}},
    {'L', {"LDAPR(B|H|)", Type::Load, 4}},
    {'L', {"LDAR(B|H|)", Type::Load, 4}},
    {'L', {"LDAXR(B|H|)", Type::Load, 4}},
    {'L', {"LDAXP", Type::Load, 4}},
    {'L', {"LDLAR(B|H|)", Type::Load, 4}},
    {'S', {"STR(B|H|)", Type::Store, 1}},
    {'S', {"STUR(B|H|)", Type::Store, 1}},
    {'S', {"STP", Type::Store, 2}},
    {'S', {"STNP", Type::Store, 2}},
    {'S', {"STTR(B|H|)", Type::Store, 1}},
    {'S', {"STXR(B|H|)", Type::Store, 1}},
    {'S', {"STXP", Type::Store, 1}},
    {'S', {"STLR(B|H|)", Type::Store, 1}},
    {'S', {"STLXR(B|H|)", Type::Store, 1}},
    {'S', {"STLXP", Type::Store, 1}},
    {'S', {"STLLR(B|H|)", Type::Store, 1}},
    {'A', {"ADD(S|)", Type::Arithmetic, 1}},
    {'S', {"SUB(S|)", Type::Arithmetic, 1}},
    {'C', {"CMP", Type::Arithmetic, 1}},
    {'C', {"CMN", Type::Arithmetic, 1}},
    {'A', {"AND(S|)", Type::Arithmetic, 1}},
    {'E', {"EOR", Type::Arithmetic, 1}},
    {'O', {"ORR", Type::Arithmetic, 1}},
    {'T', {"TST", Type::Arithmetic, 1}},
    {'M', {"MOV(Z|N|K|)", Type::Arithmetic, 1}},
    {'A', {"ADR(P|)", Type::Arithmetic, 1}},
    {'B', {"BFM", Type::Arithmetic, 2}},
    {'S', {"SBFM", Type::Arithmetic, 2}},
    {'U', {"UBFM", Type::Arithmetic, 2}},
    {'B', {"BFC", Type::Arithmetic, 2}},
    {'B', {"BFI", Type::Arithmetic, 2}},
    {'B', {"BFXIL", Type::Arithmetic, 1}},
    {'S', {"SBFIZ", Type::Arithmetic, 2}},
    {'S', {"SBFX", Type::Arithmetic, 1}},
    {'U', {"UBFIZ", Type::Arithmetic, 2}},
    {'U', {"UBFX", Type::Arithmetic, 1}},
    {'E', {"EXTR", Type::Arithmetic, 1}},
    {'A', {"ASR(V|)", Type::Arithmetic, 1}},
    {'L', {"LSL(V|)", Type::Arithmetic, 1}},
    {'L', {"LSR(V|)", Type::Arithmetic, 1}},
    {'R', {"ROR(V|)", Type::Arithmetic, 1}},
    {'S', {"SXT(B|H|W|)", Type::Arithmetic, 2}},
    {'U', {"UXT(B|H|)", Type::Arithmetic, 2}},
    {'N', {"NEG(S|)", Type::Arithmetic, 1}},
    {'A', {"ADC(S|)", Type::Arithmetic, 1}},
    {'S', {"SBC(S|)", Type::Arithmetic, 1}},
    {'N', {"NGC(S|)", Type::Arithmetic, 1}},
    {'B', {"BIC(S|)", Type::Arithmetic, 1}},
    {'E', {"EON", Type::Arithmetic, 1}},
    {'M', {"MNV", Type::Arithmetic, 1}},
    {'O', {"ORN", Type::Arithmetic, 1}},
    {'M', {"MADD", Type::Arithmetic, 3}},
    {'M', {"MSUB", Type::Arithmetic, 3}},
    {'M', {"MNEG", Type::Arithmetic, 3}},
    {'M', {"MUL", Type::Arithmetic, 3}},
    {'S', {"SMADDL", Type::Arithmetic, 3}},
    {'S', {"SMSUBL", Type::Arithmetic, 3}},
    {'S', {"SMNEGL", Type::Arithmetic, 3}},
    {'S', {"SMULL", Type::Arithmetic, 3}},
    {'S', {"SMULH", Type::Arithmetic, 6}},
    {'U', {"UMADDL", Type::Arithmetic, 3}},
    {'U', {"UMSUBL", Type::Arithmetic, 3}},
    {'U', {"UMNEGL", Type::Arithmetic, 3}},
    {'U', {"UMULL", Type::Arithmetic, 3}},
    {'U', {"UMULH", Type::Arithmetic, 6}},
    {'S', {"SDIV", Type::Arithmetic, 20}},
    {'U', {"UDIV", Type::Arithmetic, 20}},
    {'C', {"CLS", Type::Arithmetic, 1}},
    {'C', {"CLZ", Type::Arithmetic, 1}},
    {'R', {"RBIT", Type::Arithmetic, 1}},
    {'R', {"REV(16|32|64|)", Type::Arithmetic, 1}},
    {'C', {"CSEL", Type::Arithmetic, 1}},
    {'C', {"CSINC", Type::Arithmetic, 1}},
    {'C', {"CSINV", Type::Arithmetic, 1}},
    {'C', {"CSNEG", Type::Arithmetic, 1}},
    {'C', {"CSET(M|)", Type::Arithmetic, 1}},
    {'C', {"CINC", Type::Arithmetic, 1}},
    {'C', {"CINV", Type::Arithmetic, 1}},
    {'C', {"CNEG", Type::Arithmetic, 1}},
    {'C', {"CCMN", Type::Arithmetic, 1}},
    {'C', {"CCMP", Type::Arithmetic, 1}},
    {'F', {"FMOV", Type::FloatingPoint, 5}},
    {'F', {"FCVT(XN|)", Type::FloatingPoint, 5}},
    {'F', {"FCVT(AS|AU|MS|MU|NS|NU|PS|PU|ZS|ZU)", Type::FloatingPoint, 10}},
    {'F', {"FJCVTZS", Type::FloatingPoint, 1}},
    {'S', {"SCVTF", Type::FloatingPoint, 10}},
    {'U', {"UCVTF", Type::FloatingPoint, 10}},
    {'F', {"FRINT(A|I|M|N|P|X|Z|)", Type::FloatingPoint, 5}},
    {'F', {"FMADD", Type::FloatingPoint, 9}},
    {'F', {"FMSUB", Type::FloatingPoint, 9}},
    {'F', {"FNMADD", Type::FloatingPoint, 9}},
    {'F', {"FNMSUB", Type::FloatingPoint, 9}},
    {'F', {"FABS", Type::FloatingPoint, 3}},
    {'F', {"FNEG", Type::FloatingPoint, 3}},
    {'F', {"FSQRT", Type::FloatingPoint, 20}},
    {'F', {"FADD", Type::FloatingPoint, 5}},
    {'F', {"FDIV", Type::FloatingPoint, 20}},
    {'F', {"FMUL", Type::FloatingPoint, 6}},
    {'F', {"FNMUL", Type::FloatingPoint, 6}},
    {'F', {"FSUB", Type::FloatingPoint, 5}},
    {'F', {"FMAX", Type::FloatingPoint, 5}},
    {'F', {"FMAXNM", Type::FloatingPoint, 5}},
    {'F', {"FMIN", Type::FloatingPoint, 5}},
    {'F', {"FMINNM", Type::FloatingPoint, 5}},
    {'F', {"FCMP(E|P|PE|)", Type::FloatingPoint, 3}},
    {'F', {"FCSEL", Type::FloatingPoint, 3}},
};

Type CortexA57::getStatistic(std::string &op, uint64_t &cycles) {
  if (rule_a57.count(op.front()) > 0) {
    auto range = rule_a57.equal_range(op.front());
    std::smatch match;

    for (auto iter = range.first; iter != range.second; ++iter) {
      if (std::regex_match(op, match, iter->second.regex)) {
        cycles = iter->second.cycle;

        return iter->second.type;
      }
    }
  }

  return Type::Ignore;
}

}  // namespace Instruction::ARM
