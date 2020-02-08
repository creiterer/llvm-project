//===-- PROL16TargetAsmStreamer.h - PROL16 Target Asm Streamer --*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
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
