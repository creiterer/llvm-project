//===-- PROL16MachineInstrLowering.cpp - Convert PROL16 MachineInstr to an MCInst --===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===-------------------------------------------------------------------------------===//
//
// This file contains code to lower PROL16 MachineInstrs to their corresponding
// MCInst records.
//
//===-------------------------------------------------------------------------------===//

#include "PROL16MachineInstrLowering.h"

#include "llvm/CodeGen/MachineOperand.h"
#include "llvm/MC/MCExpr.h"

using namespace llvm;

#define DEBUG_TYPE "prol16"

PROL16MachineInstrLowering::PROL16MachineInstrLowering(AsmPrinter const &asmPrinter)
: asmPrinter(asmPrinter) {}

MCInst PROL16MachineInstrLowering::lowerToMCInst(MachineInstr const * const machineInstruction) const {
	LLVM_DEBUG(dbgs() << "PROL16MachineInstrLowering::lowerToMCInst() -- machine instruction: ");
	LLVM_DEBUG(machineInstruction->dump());

	MCInst loweredInstruction;

	loweredInstruction.setOpcode(machineInstruction->getOpcode());

	for (MachineOperand const &machineOperand : machineInstruction->operands()) {
		loweredInstruction.addOperand(lowerToMCOperand(machineOperand));
	}

	LLVM_DEBUG(dbgs() << "PROL16MachineInstrLowering::lowerToMCInst() -- lowered mc instruction: ");
	LLVM_DEBUG(loweredInstruction.dump());

	return loweredInstruction;
}

MCOperand PROL16MachineInstrLowering::lowerToMCOperand(MachineOperand const &machineOperand) const {
	switch (machineOperand.getType()) {
	case MachineOperand::MO_Register:
		LLVM_DEBUG(dbgs() << "PROL16MachineInstrLowering::lowerToMCOperand() -- machine operand register: ");
		LLVM_DEBUG(machineOperand.dump());

		return MCOperand::createReg(machineOperand.getReg());
	case MachineOperand::MO_Immediate:
		LLVM_DEBUG(dbgs() << "PROL16MachineInstrLowering::lowerToMCOperand() -- machine operand immediate: ");
		LLVM_DEBUG(machineOperand.dump());

		return MCOperand::createImm(machineOperand.getImm());
	case MachineOperand::MO_GlobalAddress:
		LLVM_DEBUG(dbgs() << "PROL16MachineInstrLowering::lowerToMCOperand() -- machine operand global address: ");
		LLVM_DEBUG(machineOperand.dump());

		return lowerSymbolOperand(machineOperand, asmPrinter.getSymbol(machineOperand.getGlobal()));
	case MachineOperand::MO_MachineBasicBlock:
		LLVM_DEBUG(dbgs() << "PROL16MachineInstrLowering::lowerToMCOperand() -- machine operand basic block: ");
		LLVM_DEBUG(machineOperand.dump());

		return MCOperand::createExpr(MCSymbolRefExpr::create(machineOperand.getMBB()->getSymbol(), asmPrinter.OutContext));
	case MachineOperand::MO_ExternalSymbol:
		LLVM_DEBUG(dbgs() << "PROL16MachineInstrLowering::lowerToMCOperand() -- machine operand external symbol: ");
		LLVM_DEBUG(machineOperand.dump());

		return lowerSymbolOperand(machineOperand, asmPrinter.GetExternalSymbolSymbol(machineOperand.getSymbolName()));
	default:
		machineOperand.print(errs());
		llvm_unreachable("Encountered an unknown machine operand type while lowering to machine code operand");
	}
}

MCOperand PROL16MachineInstrLowering::lowerSymbolOperand(MachineOperand const &machineOperand, MCSymbol *symbol) const {
	switch (machineOperand.getTargetFlags()) {
	case 0:	// TODO(PROL16): create an enum for this (see RISCV target)
		break;
	default:
		llvm_unreachable("Encountered unknown target flag on GV operand while lowering symbol operand");
	}

	MCExpr const *expression = MCSymbolRefExpr::create(symbol, asmPrinter.OutContext);

	if (!machineOperand.isJTI() && machineOperand.getOffset()) {
		llvm_unreachable("Offset handling not implemented yet");
	}

	return MCOperand::createExpr(expression);
}
