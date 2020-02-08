//===-- PROL16MachineInstrLowering.h - Lower MachineInstr to MCInst ------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
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
