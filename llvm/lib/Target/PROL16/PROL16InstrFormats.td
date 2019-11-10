//===-- PROL16InstrFormats.td - PROL16 Instruction Formats ---*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file describes the PROL16 instruction format.
//
//===----------------------------------------------------------------------===//

class PROL16Instruction<bits<6> opcode, 
						dag outs, dag ins, string asmString, 
						list<dag> pattern> : Instruction {
	field bits<16> instruction;
	
	let instruction{15-10} = opcode;
	
	// Definitions from base class Instruction in Target.td
	let Namespace = "PROL16";
	dag OutOperandList = outs;
  	dag InOperandList = ins;
  	let AsmString = asmString;
	let Pattern = pattern;
}

class PROL16ALUInstruction2Registers<bits<6> opcode, SDNode opNode, string mnemonic>
	: PROL16Instruction<opcode,
						(outs GR16:$dst), (ins GR16:$src, GR16:$src2),
						!strconcat(mnemonic, "\t{$dst, $src2}"),
						[(set GR16:$dst, (opNode GR16:$src, GR16:$src2))]> {
	let Constraints = "$src = $dst";					
}

// Pseudo instruction
class PROL16PseudoInstruction<dag outs, dag ins, string asmString, list<dag> pattern>
	: PROL16Instruction<0x0, outs, ins, asmString, pattern> {

//	let isCodeGenOnly = 1;
  	let isPseudo = 1;
  	let instruction{15-0} = 0x0;
}
