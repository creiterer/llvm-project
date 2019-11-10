//===-- PROL16TargetInfo.cpp - PROL16 Target Implementation -----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
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
