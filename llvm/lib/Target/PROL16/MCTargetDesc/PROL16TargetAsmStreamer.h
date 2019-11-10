//===-- PROL16TargetAsmStreamer.h - PROL16 Target Asm Streamer --*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_PROL16_MCTARGETDESC_PROL16TARGETASMSTREAMER_H
#define LLVM_LIB_TARGET_PROL16_MCTARGETDESC_PROL16TARGETASMSTREAMER_H

#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSection.h"
#include "llvm/MC/MCExpr.h"

#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/FormattedStream.h"

namespace llvm {

class PROL16TargetAsmStreamer final : public MCTargetStreamer {
public:
	PROL16TargetAsmStreamer(MCStreamer &mcStreamer/*, formatted_raw_ostream &os*/);

	/// Update streamer for a new active section.
	///
	/// This is called by PopSection and SwitchSection, if the current
	/// section changes.
	void changeSection(MCSection const *CurSection, MCSection *Section, MCExpr const *SubSection, raw_ostream &OS) override;

	void emitLabel(MCSymbol *Symbol) override;

private:
//	formatted_raw_ostream &os;
};

} // end namespace llvm

#endif /* LLVM_LIB_TARGET_PROL16_MCTARGETDESC_PROL16TARGETASMSTREAMER_H */
