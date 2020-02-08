//===-- PROL16TargetAsmStreamer.cpp - PROL16 Target Streamer Methods -----===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
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
