//===-- PROL16MCTargetDesc.h - PROL16 Target Descriptions ---------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file provides PROL16 specific target descriptions.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_PROL16_MCTARGETDESC_PROL16MCTARGETDESC_H
#define LLVM_LIB_TARGET_PROL16_MCTARGETDESC_PROL16MCTARGETDESC_H

namespace llvm {
class Target;

Target& getThePROL16Target();

} // End llvm namespace

#endif
