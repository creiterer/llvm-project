//===- PROL16MCAsmInfo.h - PROL16 asm properties -----------------*- C++ -*--===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
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
