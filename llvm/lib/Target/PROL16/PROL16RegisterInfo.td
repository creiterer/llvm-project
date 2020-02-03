//===-- PROL16RegisterInfo.td - PROL16 Register Definitions ---------*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//  Declarations that describe the PROL16 register file.
//
//===----------------------------------------------------------------------===//

class PROL16Register<bits<5> num, string name, list<Register> aliases = []> : Register<name> {
	field bits<5> number = num;
	
	let Namespace = "PROL16";
	let Aliases = aliases;
}

//===----------------------------------------------------------------------===//
//  Registers
//===----------------------------------------------------------------------===//

def RPC	: PROL16Register<0, "rpc">;			// program counter register
def RRA	: PROL16Register<1, "rra">;			// return address register
def RSP	: PROL16Register<2, "rsp">;			// stack pointer register
def RFP	: PROL16Register<3, "rfp">;			// frame pointer register
def R4	: PROL16Register<4, "r4">;
def R5	: PROL16Register<5, "r5">;
def R6	: PROL16Register<6, "r6">;
def R7	: PROL16Register<7, "r7">;
def R8	: PROL16Register<8, "r8">;
def R9	: PROL16Register<9, "r9">;
def R10	: PROL16Register<10, "r10">;
def R11	: PROL16Register<11, "r11">;
def R12	: PROL16Register<12, "r12">;
def R13	: PROL16Register<13, "r13">;
def R14	: PROL16Register<14, "r14">;
def R15	: PROL16Register<15, "r15">;
def R16 : PROL16Register<16, "r16">;
def R17 : PROL16Register<17, "r17">;
def R18 : PROL16Register<18, "r18">;
def R19 : PROL16Register<19, "r19">;
def R20 : PROL16Register<20, "r20">;
def R21 : PROL16Register<21, "r21">;
def R22 : PROL16Register<22, "r22">;
def R23 : PROL16Register<23, "r23">;
def R24 : PROL16Register<24, "r24">;
def R25 : PROL16Register<25, "r25">;
def R26 : PROL16Register<26, "r26">;
def R27 : PROL16Register<27, "r27">;
def R28 : PROL16Register<28, "r28">;
def R29 : PROL16Register<29, "r29">;
def R30 : PROL16Register<30, "r30">;
def R31 : PROL16Register<31, "r31">;

//===----------------------------------------------------------------------===//
//  Register Classes
//===----------------------------------------------------------------------===//

def GR16 : RegisterClass<"PROL16", [i16], 16, (add RPC, RRA, RSP, RFP, (sequence "R%u", 4, 31))>;
