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

#include "llvm/MC/MCSectionELF.h"
#include "llvm/Support/Casting.h"

#include <cassert>

using namespace llvm;

PROL16TargetAsmStreamer::PROL16TargetAsmStreamer(MCStreamer &mcStreamer/*, formatted_raw_ostream &os*/)
: MCTargetStreamer(mcStreamer)/*, os(os)*/ {}

void PROL16TargetAsmStreamer::changeSection(const MCSection *CurSection, MCSection *Section, const MCExpr *SubSection, raw_ostream &OS) {
	assert(Section != nullptr);

	MCSectionELF const * const elfSection = cast<MCSectionELF>(Section);
	if (elfSection->getSectionName() == ".text" || elfSection->getSectionName() == ".data") {
		MCTargetStreamer::changeSection(CurSection, Section, SubSection, OS);
	}
}

void PROL16TargetAsmStreamer::emitLabel(MCSymbol *Symbol) {

}
