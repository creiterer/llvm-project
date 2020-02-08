//===-- PROL16AsmPrinter.cpp - PROL16 LLVM assembly writer ------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains a printer that converts from our internal representation
// of machine-dependent LLVM code to GAS-format PROL16 assembly language.
//
//===----------------------------------------------------------------------===//

#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCInst.h"

#include "PROL16MachineInstrLowering.h"
#include "MCTargetDesc/PROL16MCTargetDesc.h"

using namespace llvm;

#define DEBUG_TYPE "asm-printer"

// Defines symbolic names for the PROL16 instructions.
#define GET_INSTRINFO_ENUM
#include "PROL16GenInstrInfo.inc"

#define GET_REGINFO_ENUM
#include "PROL16GenRegisterInfo.inc"

namespace {

class PROL16AsmPrinter final : public AsmPrinter {
public:
	PROL16AsmPrinter(TargetMachine &targetMachine, std::unique_ptr<MCStreamer> mcStreamer)
		: AsmPrinter(targetMachine, std::move(mcStreamer)) {}

	/// This method emits the header for the current function.
	void EmitFunctionHeader() override;

	/// Targets should implement this to emit instructions.
	void EmitInstruction(MachineInstr const *machineInstruction) override;

	bool emitPseudoExpansionLowering(MCStreamer &OutStreamer, MachineInstr const *MI);

	/// Wrapper needed for tblgen'ned pseudo lowering.
	bool lowerOperand(MachineOperand const &machineOperand, MCOperand &mcOperand) const;
};

} // end of anonymous namespace

// Simple pseudo instructions have their lowering (with expansion to real
// instructions) auto-generated.
#include "PROL16GenMCPseudoLowering.inc"

void PROL16AsmPrinter::EmitFunctionHeader() {
	// FIXME PROL16: overriding this method empty omits pretty much code of the base class
	// that does some default handling of things -> check if these things are really not needed
	// so that omitting them is safe!!!
	EmitFunctionEntryLabel();
}

void PROL16AsmPrinter::EmitInstruction(MachineInstr const *machineInstruction) {
	// Do any auto-generated pseudo instruction lowerings.
	if (emitPseudoExpansionLowering(*OutStreamer, machineInstruction)) { return; }

	PROL16MachineInstrLowering machineInstrLowering(*this);

	EmitToStreamer(*OutStreamer, machineInstrLowering.lowerToMCInst(machineInstruction));
}

bool PROL16AsmPrinter::lowerOperand(MachineOperand const &machineOperand, MCOperand &mcOperand) const {
	PROL16MachineInstrLowering machineInstrLowering(*this);

	mcOperand = machineInstrLowering.lowerToMCOperand(machineOperand);

	return true;
}

// Force static initialization.
extern "C" void LLVMInitializePROL16AsmPrinter() {
	RegisterAsmPrinter<PROL16AsmPrinter> X(getThePROL16Target());
}
