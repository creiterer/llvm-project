//===-- PROL16Subtarget.cpp - PROL16 Subtarget Information ------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the PROL16 specific subclass of TargetSubtargetInfo.
//
//===----------------------------------------------------------------------===//

#include "PROL16Subtarget.h"

#include "llvm/ADT/StringRef.h"

#include "PROL16.h"

using namespace llvm;

#define DEBUG_TYPE "prol16-subtarget"

#define GET_SUBTARGETINFO_TARGET_DESC
#define GET_SUBTARGETINFO_CTOR
#include "PROL16GenSubtargetInfo.inc"

PROL16Subtarget::PROL16Subtarget(Triple const &targetTriple, std::string const &cpu, std::string const &featureString,
		TargetMachine const &targetMachine)
: PROL16GenSubtargetInfo(targetTriple, cpu, featureString),
  frameLowering(),
  targetLowering(targetMachine, *this),
  instructionInfo() {}

TargetFrameLowering const* PROL16Subtarget::getFrameLowering() const {
	return &frameLowering;
}

PROL16TargetLowering const* PROL16Subtarget::getTargetLowering() const {
	return &targetLowering;
}

PROL16InstrInfo const* PROL16Subtarget::getInstrInfo() const {
	return &instructionInfo;
}

TargetRegisterInfo const* PROL16Subtarget::getRegisterInfo() const {
	return &instructionInfo.getRegisterInfo();
}
