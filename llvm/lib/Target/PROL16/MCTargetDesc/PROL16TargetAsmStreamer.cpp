//===-- PROL16TargetAsmStreamer.cpp - PROL16 Target Streamer Methods -----===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file provides PROL16 specific target asm streamer methods.
//
//===----------------------------------------------------------------------===//

#include "PROL16TargetAsmStreamer.h"

using namespace llvm;

PROL16TargetAsmStreamer::PROL16TargetAsmStreamer(MCStreamer &mcStreamer/*, formatted_raw_ostream &os*/)
: MCTargetStreamer(mcStreamer)/*, os(os)*/ {}

void PROL16TargetAsmStreamer::changeSection(const MCSection *CurSection, MCSection *Section, const MCExpr *SubSection, raw_ostream &OS) {
	// XXX PROL16: The standard PROL16 assembler isn't aware of sections (e.g. .text or .data)
	// -> do not emit such section directives
	// Otherwise, separation of .text and .data is meaningful, so there are 2 possibilities:
	// 	1. modify the assembler so that it understands these directives
	//	2. replace .text and .data with an appropriate ORG directive (e.g. ORG 0000h for .text and ORG 8000h for .data)
}

void PROL16TargetAsmStreamer::emitLabel(MCSymbol *Symbol) {

}
