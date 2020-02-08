//===-- PROL16TargetInfo.cpp - PROL16 Target Implementation -----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "llvm/Support/TargetRegistry.h"

#include "MCTargetDesc/PROL16MCTargetDesc.h"

using namespace llvm;

Target& llvm::getThePROL16Target() {
	static Target ThePROL16Target;
	return ThePROL16Target;
}

extern "C" void LLVMInitializePROL16TargetInfo() {
	RegisterTarget<Triple::prol16> X(getThePROL16Target(), "prol16", "PROL16 [experimental]", "PROL16");
}
