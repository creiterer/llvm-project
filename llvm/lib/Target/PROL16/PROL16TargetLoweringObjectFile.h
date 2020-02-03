//===-- PROL16TargetLoweringObjectFile.h - PROL16 Object Info -*- C++ ---------*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_PROL16_PROL16TargetLoweringObjectFile_H
#define LLVM_LIB_TARGET_PROL16_PROL16TargetLoweringObjectFile_H

#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"

namespace llvm {

class PROL16TargetLoweringObjectFile final : public TargetLoweringObjectFileELF {
protected:
	MCSection* SelectSectionForGlobal(GlobalObject const *GO,
									  SectionKind Kind,
									  TargetMachine const &TM) const override;
};

} // end namespace llvm

#endif
