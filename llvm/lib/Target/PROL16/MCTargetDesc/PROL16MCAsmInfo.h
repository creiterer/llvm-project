//===- PROL16MCAsmInfo.h - PROL16 asm properties -----------------*- C++ -*--===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the declaration of the PROL16MCAsmInfo class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_PROL16_MCTARGETDESC_PROL16MCASMINFO_H
#define LLVM_LIB_TARGET_PROL16_MCTARGETDESC_PROL16MCASMINFO_H

#include "llvm/MC/MCAsmInfo.h"
#include "llvm/ADT/Triple.h"

namespace llvm {

class PROL16MCAsmInfo final : public MCAsmInfo {
public:
  explicit PROL16MCAsmInfo(Triple const &targetTriple);
};

} // end namespace llvm

#endif
