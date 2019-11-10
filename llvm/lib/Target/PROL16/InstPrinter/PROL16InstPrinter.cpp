//===-- PROL16InstPrinter.cpp - Convert PROL16 MCInst to assembly syntax -----==//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This class prints a PROL16 MCInst to a .s file.
//
//===----------------------------------------------------------------------===//

#include "PROL16InstPrinter.h"

#include "llvm/MC/MCExpr.h"

using namespace llvm;

// Include the auto-generated portion of the assembly writer.
#include "PROL16GenAsmWriter.inc"

#define GET_REGINFO_ENUM
#include "PROL16GenRegisterInfo.inc"

PROL16InstPrinter::PROL16InstPrinter(MCAsmInfo const &mcAsmInfo, MCInstrInfo const &mcInstrInfo,
									 MCRegisterInfo const &mcRegisterInfo)
: MCInstPrinter(mcAsmInfo, mcInstrInfo, mcRegisterInfo) {}

void PROL16InstPrinter::printInst(MCInst const *MI, raw_ostream &OS, StringRef Annot, MCSubtargetInfo const &STI) {
	printInstruction(MI, OS);
	printAnnotation(OS, Annot);
}

void PROL16InstPrinter::printOperand(MCInst const *mcInst, unsigned operandNumber, raw_ostream &rawOstream) {
	MCOperand const &operand = mcInst->getOperand(operandNumber);

	if (operand.isReg()) {
		rawOstream << getRegisterName(operand.getReg());
	} else if (operand.isImm()) {
		rawOstream << static_cast<uint16_t>(operand.getImm());
	} else if (operand.isExpr()) {
		operand.getExpr()->print(rawOstream, &MAI);
	} else {
		llvm_unreachable("Encountered unknown operand kind while printing operand");
	}
}
