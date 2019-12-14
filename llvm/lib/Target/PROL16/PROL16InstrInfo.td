//===-- PROL16InstrInfo.td - Target Description for PROL16 Target -----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file describes the PROL16 instructions in TableGen format.
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// Instruction format superclass
//===----------------------------------------------------------------------===//

include "PROL16InstrFormats.td"

//===----------------------------------------------------------------------===//
// PROL16 specific Type Profiles
//===----------------------------------------------------------------------===//

def SDTPROL16Call			: SDTypeProfile<0, -1, [SDTCisVT<0, iPTR>]>;
def SDTPROL16CallSeqStart	: SDCallSeqStart<[SDTCisVT<0, i16>, SDTCisVT<1, i16>]>;
def SDTPROL16CallSeqEnd		: SDCallSeqEnd<[SDTCisVT<0, i16>, SDTCisVT<1, i16>]>;

def SDTPROL16Comp          	: SDTypeProfile<0, 2, [SDTCisSameAs<0, 1>]>;
def SDTPROL16Jumpc			: SDTypeProfile<0, 1, [SDTCisVT<0, i16>]>;
def SDTPROL16Jumpz			: SDTypeProfile<0, 1, [SDTCisVT<0, i16>]>;
def SDTPROL16Jumpcz			: SDTypeProfile<0, 1, [SDTCisVT<0, i16>]>;
def SDTPROL16Jumpnz			: SDTypeProfile<0, 1, [SDTCisVT<0, i16>]>;

def SDTPROL16SelectCC		: SDTypeProfile<1, 5, [SDTCisSameAs<0, 1>, SDTCisSameAs<2, 3>, SDTCisVT<4, i16>]>;

//===----------------------------------------------------------------------===//
// PROL16 specific DAG Nodes
//===----------------------------------------------------------------------===//

def PROL16Call			: SDNode<"PROL16ISD::CALL", SDTPROL16Call,
							 	 [SDNPHasChain, SDNPOutGlue, SDNPOptInGlue, SDNPVariadic]>;
							 	 
def PROL16Return		: SDNode<"PROL16ISD::RETURN", SDTNone,
								 [SDNPHasChain, SDNPOptInGlue, SDNPVariadic]>;

def PROL16Comp     		: SDNode<"PROL16ISD::COMP", SDTPROL16Comp, [SDNPOutGlue]>;

def PROL16Jumpc			: SDNode<"PROL16ISD::JUMPC", SDTPROL16Jumpc, [SDNPHasChain, SDNPInGlue]>;

def PROL16Jumpz			: SDNode<"PROL16ISD::JUMPZ", SDTPROL16Jumpz, [SDNPHasChain, SDNPInGlue]>;

def PROL16Jumpcz		: SDNode<"PROL16ISD::JUMPCZ", SDTPROL16Jumpcz, [SDNPHasChain, SDNPInGlue]>;

def PROL16Jumpnz		: SDNode<"PROL16ISD::JUMPNZ", SDTPROL16Jumpnz, [SDNPHasChain, SDNPInGlue]>;

def PROL16SelectCC		: SDNode<"PROL16ISD::SELECT_CC", SDTPROL16SelectCC, [SDNPInGlue]>;

//  These are target-independent nodes, but have target-specific formats.
def PROL16CallSeqStart	: SDNode<"ISD::CALLSEQ_START", SDTPROL16CallSeqStart, 
								 [SDNPHasChain, SDNPOutGlue]>;

def PROL16CallSeqEnd	: SDNode<"ISD::CALLSEQ_END", SDTPROL16CallSeqEnd,
								 [SDNPHasChain, SDNPOptInGlue, SDNPOutGlue]>;

//===----------------------------------------------------------------------===//
// PROL16 specific Operand Definitions
//===----------------------------------------------------------------------===//

// memory operand
// register (GR16) for base address and immediate (i16imm) for displacement
def memory : Operand<i16> {
	let MIOperandInfo = (ops GR16, i16imm);
}

// direct jump target operand
def jumptarget : Operand<OtherVT> {
}

//===----------------------------------------------------------------------===//
// PROL16 specific Complex Pattern Definitions
//===----------------------------------------------------------------------===//

def addr : ComplexPattern<iPTR, 1, "SelectAddress">;
def addrdisp : ComplexPattern<iPTR, 2, "SelectAddressWithDisplacement">;

//===----------------------------------------------------------------------===//
// Pseudo Instructions
//===----------------------------------------------------------------------===//

let Defs = [RSP], Uses = [RSP] in {
	def ADJCALLSTACKDOWN : PROL16PseudoInstruction<(outs), (ins i16imm:$amt1, i16imm:$amt2),
	                               			 	   "#ADJCALLSTACKDOWN $amt1, $amt2",
	                               			 	   [(PROL16CallSeqStart timm:$amt1, timm:$amt2)]>;
	                               			 
	def ADJCALLSTACKUP : PROL16PseudoInstruction<(outs), (ins i16imm:$amt1, i16imm:$amt2),
	                            		   		 "#ADJCALLSTACKUP $amt1",
	                            		   		 [(PROL16CallSeqEnd timm:$amt1, timm:$amt2)]>;
}
	
let usesCustomInserter = 1 in {
	let mayStore = 1 in {
		// pseudo instruction for storing an immediate into memory, 
		// where the memory destination is given through a register containing the base address
		// and an immediate specifying the displacement 
		def STOREim : PROL16PseudoInstruction<(outs), (ins memory:$dst, i16imm:$src),
									   		 "# STOREim PSEUDO!",
									   		 [(store imm:$src, addrdisp:$dst)]>;
									   		 
		// pseudo instruction for storing a register into memory, 
		// where the memory destination is given through a register containing the base address
		// and an immediate specifying the displacement 
		def STORErm : PROL16PseudoInstruction<(outs), (ins memory:$dst, GR16:$src),
									   		 "# STORErm PSEUDO!",
									   		 [(store GR16:$src, addrdisp:$dst)]>;						
	}
	
	let mayLoad = 1 in {
		// pseudo instruction for loading a value from memory into a register,
		// where the memory source is given through a register containing the base address
		// and an immediate specifying the displacement 
		def LOADrm : PROL16PseudoInstruction<(outs GR16:$dst), (ins memory:$src),
									 		 "# LOADrm PSEUDO!",
									 		 [(set GR16:$dst, (load addrdisp:$src))]>;
	}
	
	let isBranch = 1, isTerminator = 1, isBarrier = 1 in {
		def BR : PROL16PseudoInstruction<(outs), (ins jumptarget:$dst),
										 "# BR PSEUDO!",
										 [(br bb:$dst)]>;
										 
		let isIndirectBranch = 1 in {
				def JUMPCZ : PROL16PseudoInstruction<(outs), (ins GR16:$brdst),
										  			 "# JUMPCZ PSEUDO!",
										  			 [(PROL16Jumpcz GR16:$brdst)]>;
										  			 
				def JUMPNZ : PROL16PseudoInstruction<(outs), (ins GR16:$brdst),
										  			 "# JUMPNZ PSEUDO!",
										  			 [(PROL16Jumpnz GR16:$brdst)]>;
		}
	}
	
	let Constraints = "$src = $dst" in {
			def SRA : PROL16PseudoInstruction<(outs GR16:$dst), (ins GR16:$src),
											  "# SRA PSEUDO!",
											  [(set GR16:$dst, (sra GR16:$src, 1))]>;

			def SHLi : PROL16PseudoInstruction<(outs GR16:$dst), (ins GR16:$src, i16imm:$cnt),
											   "# SHLi PSEUDO!",
											   [(set GR16:$dst, (shl GR16:$src, imm:$cnt))]>;
											   
			def SHRi : PROL16PseudoInstruction<(outs GR16:$dst), (ins GR16:$src, i16imm:$cnt),
											   "# SHRi PSEUDO!",
											   [(set GR16:$dst, (srl GR16:$src, imm:$cnt))]>;
											   
			def SRAi : PROL16PseudoInstruction<(outs GR16:$dst), (ins GR16:$src, i16imm:$cnt),
											   "# SRAi PSEUDO!",
											   [(set GR16:$dst, (sra GR16:$src, imm:$cnt))]>;
											  
			def SHLr : PROL16PseudoInstruction<(outs GR16:$dst), (ins GR16:$src, GR16:$cnt),
											   "# SHLr PSEUDO!",
											   [(set GR16:$dst, (shl GR16:$src, GR16:$cnt))]>;
											   
			def SHRr : PROL16PseudoInstruction<(outs GR16:$dst), (ins GR16:$src, GR16:$cnt),
											   "# SHRr PSEUDO!",
											   [(set GR16:$dst, (srl GR16:$src, GR16:$cnt))]>;
											   
			def SRAr : PROL16PseudoInstruction<(outs GR16:$dst), (ins GR16:$src, GR16:$cnt),
											   "# SRAr PSEUDO!",
											   [(set GR16:$dst, (sra GR16:$src, GR16:$cnt))]>;
	}
	
	def SELECT : PROL16PseudoInstruction<(outs GR16:$dst), 
										 (ins GR16:$lhs, GR16:$rhs, GR16:$trueSrc, GR16:$falseSrc, i16imm:$cc),
										 "# SELECT PSEUDO!",
										 [(set GR16:$dst, (PROL16SelectCC GR16:$lhs, GR16:$rhs, GR16:$trueSrc, GR16:$falseSrc, imm:$cc))]>;
}

//===----------------------------------------------------------------------===//
//  Miscellaneous Instructions
//===----------------------------------------------------------------------===//

let hasSideEffects = 0 in {
	def NOP : PROL16Instruction<0x0, (outs), (ins), "nop", []>;
	def SLEEP : PROL16Instruction<0x1, (outs), (ins), "sleep", []>;
}

//===----------------------------------------------------------------------===//
//  Load & Store Instructions
//===----------------------------------------------------------------------===//

let hasSideEffects = 0 in {
	def LOADI : PROL16Instruction<0x2,
								  (outs GR16:$dst), (ins i16imm:$src),
								  "loadi\t{$dst, $src}",
								  [(set GR16:$dst, imm:$src)]>;
	
	let mayLoad = 1 in {
		def LOAD : PROL16Instruction<0x3,
									 (outs GR16:$dst), (ins GR16:$src),
									 "load\t{$dst, $src}",
									 [(set GR16:$dst, (load addr:$src))]>;
	}
	
	let mayStore = 1 in {
		def STORE : PROL16Instruction<0x4,
									  (outs), (ins GR16:$src, GR16:$dst),
									  "store\t{$src, $dst}",
									  [(store GR16:$src, addr:$dst)]>;
	}
}

//===----------------------------------------------------------------------===//
//  Move Instructions
//===----------------------------------------------------------------------===//

let hasSideEffects = 0, isMoveReg = 1 in {
	def MOVE : PROL16Instruction<0xC, 
								 (outs GR16:$dst), (ins GR16:$src),
								 "move\t{$dst, $src}", []>;
}

//===----------------------------------------------------------------------===//
//  Jump Instructions (indirect branches)
//===----------------------------------------------------------------------===//

let isBranch = 1, isTerminator = 1, isIndirectBranch = 1, isBarrier = 1 in {
	// Unconditional indirect branches
	def JUMP : PROL16Instruction<0x8,
								 (outs), (ins GR16:$brdst),
								 "jump\t{$brdst}",
								 [(brind GR16:$brdst)]>;
	
	let usesCustomInserter = 1 in {
		// Conditional indirect branches
		def JUMPC : PROL16Instruction<0xA,
									  (outs), (ins GR16:$brdst),
									  "jumpc\t{$brdst}",
									  [(PROL16Jumpc GR16:$brdst)]>;
									  
		def JUMPZ : PROL16Instruction<0xB,
									  (outs), (ins GR16:$brdst),
									  "jumpz\t{$brdst}",
									  [(PROL16Jumpz GR16:$brdst)]>;
	
		def JUMPcc : PROL16PseudoInstruction<(outs), 
										 	 (ins GR16:$dst, GR16:$lhs, GR16:$rhs, i16imm:$cc),
										 	 "# JUMPcc PSEUDO!",
										 	 []>;
	}
}

let isCall = 1, Uses = [RPC, RRA, RSP] in {
	// jump call: pseudo call instruction that simply jumps to the target call address
	def JUMPcall : PROL16PseudoInstruction<(outs), (ins GR16:$dst),
										   "# JUMPcall PSEUDO!",
										   []>;
				   
//	def CALLr : PROL16PseudoInstruction<(outs), (ins GR16:$dst),
//										"# CALLr PSEUDO!",
//										[(PROL16Call GR16:$dst)]>,
//				PseudoInstExpansion<(JUMP GR16:$dst)>;

	let usesCustomInserter = 1 in {				
		def CALLi : PROL16PseudoInstruction<(outs), (ins i16imm:$dst),
											"# CALLi PSEUDO!",
											[(PROL16Call imm:$dst)]>;
	}
}

let isReturn = 1, isTerminator = 1, isBarrier = 1, Uses = [RRA] in {
	let usesCustomInserter = 1 in {
		def RETURN : PROL16PseudoInstruction<(outs), (ins),
											 "# RETURN PSEUDO!",
											 [(PROL16Return)]>;
	}
	
	// jump return: pseudo return instruction that simply jumps to the address in rra
	def JUMPret : PROL16PseudoInstruction<(outs), (ins),
										  "# JUMPret PSEUDO!",
										[]>,
				PseudoInstExpansion<(JUMP RRA)>;
}

//===----------------------------------------------------------------------===//
//  ALU Instructions
//===----------------------------------------------------------------------===//

	let isCommutable = 1 in { // (X = OPERATION Y, Z) == (X = OPERATION Z, Y)
		def AND : PROL16ALUInstruction2Registers<0x10, and, "and">;
		def OR : PROL16ALUInstruction2Registers<0x11, or, "or">;
		def XOR : PROL16ALUInstruction2Registers<0x12, xor, "xor">;
		
		def ADD : PROL16ALUInstruction2Registers<0x14, add, "add">;
		def ADDC : PROL16ALUInstruction2Registers<0x15, adde, "addc">;
	}

	def SUB : PROL16ALUInstruction2Registers<0x16, sub, "sub">;
	def SUBC : PROL16ALUInstruction2Registers<0x17, sube, "subc">;

let Constraints = "$src = $dst" in {

	def INC : PROL16Instruction<0x1A,
								(outs GR16:$dst), (ins GR16:$src),
								"inc\t{$src}",
								[(set GR16:$dst, (add GR16:$src, 1))]>;

	def DEC : PROL16Instruction<0x1B,
								(outs GR16:$dst), (ins GR16:$src),
								"dec\t{$src}",
								[(set GR16:$dst, (add GR16:$src, -1))]>;

	def SHL : PROL16Instruction<0x1C,
								(outs GR16:$dst), (ins GR16:$src),
								"shl\t{$src}",
								[(set GR16:$dst, (shl GR16:$src, 1))]>;
								
	def SHR : PROL16Instruction<0x1D,
								(outs GR16:$dst), (ins GR16:$src),
								"shr\t{$src}",
								[(set GR16:$dst, (srl GR16:$src, 1))]>;

	def SHLC : PROL16Instruction<0x1E,
								(outs GR16:$dst), (ins GR16:$src),
								"shlc\t{$src}",
								[]>;
								
	def SHRC : PROL16Instruction<0x1F,
								(outs GR16:$dst), (ins GR16:$src),
								"shrc\t{$src}",
								[]>;

	def NOT : PROL16Instruction<0x13,
								(outs GR16:$dst), (ins GR16:$src),
								"not\t{$src}",
								[(set GR16:$dst, (not GR16:$src))]>;
}


let isCompare = 1 in {
	def COMP : PROL16Instruction<0x18,
								 (outs), (ins GR16:$src, GR16:$src2),
								 "comp\t{$src, $src2}",
								 [(PROL16Comp GR16:$src, GR16:$src2)]>;
}

//===----------------------------------------------------------------------===//
//  Non-Instruction Patterns
//===----------------------------------------------------------------------===//
// calls
def : Pat<(PROL16Call (i16 tglobaladdr:$dst)),
          (CALLi tglobaladdr:$dst)>;
def : Pat<(PROL16Call (i16 texternalsym:$dst)),
          (CALLi texternalsym:$dst)>;
          
// truncating store operations
def : Pat<(truncstorei8 GR16:$src, GR16:$dst),
		  (STORE GR16:$src, GR16:$dst)>;
def : Pat<(truncstorei8 GR16:$src, addrdisp:$dst),
		  (STORErm addrdisp:$dst, GR16:$src)>;
		  
// extending load operations
def anyextload : PatFrag<(ops node:$ptr), (unindexedload node:$ptr), [{
  return cast<LoadSDNode>(N)->getExtensionType() != ISD::NON_EXTLOAD;
}]>;
def anyextloadi8 : PatFrag<(ops node:$ptr), (anyextload node:$ptr), [{
  return cast<LoadSDNode>(N)->getMemoryVT() == MVT::i8;
}]>;

def : Pat<(anyextloadi8 GR16:$src),
		  (LOAD GR16:$src)>;
def : Pat<(anyextloadi8 addrdisp:$src),
		  (LOADrm addrdisp:$src)>;
