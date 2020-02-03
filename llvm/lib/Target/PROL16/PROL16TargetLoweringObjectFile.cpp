//===-- PROL16TargetLoweringObjectFile.cpp - PROL16 Object Info -----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "PROL16TargetLoweringObjectFile.h"

using namespace llvm;

MCSection* PROL16TargetLoweringObjectFile::SelectSectionForGlobal(GlobalObject const *GO,
																  SectionKind Kind,
																  TargetMachine const &TM) const {
	return DataSection;
}
