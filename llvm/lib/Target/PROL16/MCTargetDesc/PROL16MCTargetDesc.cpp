//===-- PROL16MCTargetDesc.cpp - PROL16 Target Descriptions -----------------===//
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

#include "PROL16MCTargetDesc.h"

#include "llvm/Support/TargetRegistry.h"
#include "llvm/MC/MCRegisterInfo.h"			// needed for PROL16GenRegisterInfo.inc
#include "llvm/MC/MCInstrDesc.h"			// needed for PROL16GenInstrInfo.inc
#include "llvm/MC/MCInstrInfo.h"			// needed for PROL16GenInstrInfo.inc
#include "llvm/MC/SubtargetFeature.h"		// needed for PROL16GenSubtargetInfo.inc
#include "llvm/MC/MCSchedule.h"				// needed for PROL16GenSubtargetInfo.inc
#include "llvm/MC/MCSubtargetInfo.h"		// needed for PROL16GenSubtargetInfo.inc

#include "PROL16MCAsmInfo.h"
#include "PROL16TargetAsmStreamer.h"
#include "InstPrinter/PROL16InstPrinter.h"

using namespace llvm;

#define GET_REGINFO_MC_DESC
// Defines symbolic names for PROL16 registers.  This defines a mapping from
// register name to register number.
#define GET_REGINFO_ENUM
#include "PROL16GenRegisterInfo.inc"

#define GET_INSTRINFO_MC_DESC
#include "PROL16GenInstrInfo.inc"

#define GET_SUBTARGETINFO_MC_DESC
#include "PROL16GenSubtargetInfo.inc"

static MCInstrInfo* createPROL16MCInstrInfo() {
	MCInstrInfo *prol16MCInstrInfo = new MCInstrInfo();
	InitPROL16MCInstrInfo(prol16MCInstrInfo);

	return prol16MCInstrInfo;
}

static MCRegisterInfo* createPROL16MCRegisterInfo(Triple const &targetTriple) {
	MCRegisterInfo *prol16MCRegisterInfo = new MCRegisterInfo();
	// FIXME PROL16: think about return address register (parameter RA), keep in sync with PROL16RegisterInfo.cpp
	InitPROL16MCRegisterInfo(prol16MCRegisterInfo, PROL16::RRA);

	return prol16MCRegisterInfo;
}

static MCSubtargetInfo* createPROL16MCSubtargetInfo(Triple const &targetTriple, StringRef cpu, StringRef featureString) {
	return createPROL16MCSubtargetInfoImpl(targetTriple, cpu, featureString);
}

static MCInstPrinter* createPROL16MCInstPrinter(Triple const &targetTriple, unsigned const syntaxVariant,
												MCAsmInfo const &mcAsmInfo, MCInstrInfo const &mcInstrInfo,
												MCRegisterInfo const &mcRegisterInfo) {
	return new PROL16InstPrinter(mcAsmInfo, mcInstrInfo, mcRegisterInfo);
}

static MCTargetStreamer* createTargetAsmStreamer(MCStreamer &mcStreamer, formatted_raw_ostream &os,
												 MCInstPrinter *mcInstPrinter, bool const isVerboseAsm) {
	return new PROL16TargetAsmStreamer(mcStreamer/*, os*/);
}

extern "C" void LLVMInitializePROL16TargetMC() {
	// Register the MC asm info.
	RegisterMCAsmInfo<PROL16MCAsmInfo> X(getThePROL16Target());

	// Register the MC instruction info.
	TargetRegistry::RegisterMCInstrInfo(getThePROL16Target(), createPROL16MCInstrInfo);

	// Register the MC register info.
	TargetRegistry::RegisterMCRegInfo(getThePROL16Target(), createPROL16MCRegisterInfo);

	// Register the MC subtarget info.
	TargetRegistry::RegisterMCSubtargetInfo(getThePROL16Target(), createPROL16MCSubtargetInfo);

	// Register the MCInstPrinter.
	TargetRegistry::RegisterMCInstPrinter(getThePROL16Target(), createPROL16MCInstPrinter);

	// Register the MCTargetStreamer
	TargetRegistry::RegisterAsmTargetStreamer(getThePROL16Target(), createTargetAsmStreamer);
}
