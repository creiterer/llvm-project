//===-- PROL16MachineInstrLowering.h - Lower MachineInstr to MCInst ------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===-------------------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_PROL16_PROL16MACHINEINSTRLOWERING_H
#define LLVM_LIB_TARGET_PROL16_PROL16MACHINEINSTRLOWERING_H

#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/MC/MCInst.h"

namespace llvm {

class PROL16MachineInstrLowering final {
public:
	PROL16MachineInstrLowering(AsmPrinter const &asmPrinter);

	MCInst lowerToMCInst(MachineInstr const * const machineInstruction) const;

	MCOperand lowerToMCOperand(MachineOperand const &machineOperand) const;

	MCOperand lowerSymbolOperand(MachineOperand const &machineOperand, MCSymbol *symbol) const;

private:
	AsmPrinter const &asmPrinter;

};

}

#endif
