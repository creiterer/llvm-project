//===- PROL16MCAsmInfo.cpp - PROL16 asm properties --------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the declarations of the PROL16MCAsmInfo properties.
//
//===----------------------------------------------------------------------===//

#include "PROL16MCAsmInfo.h"

using namespace llvm;

PROL16MCAsmInfo::PROL16MCAsmInfo(Triple const &targetTriple) {
	CommentString = ";";

	/// True if the target has .type and .size directives, this is true for most
	/// ELF targets.  Defaults to true.
	HasDotTypeDotSizeDirective = false;

	/// True if the target has a single parameter .file directive, this is true
	/// for ELF targets.  Defaults to true.
	HasSingleParameterDotFile = false;

	/// This directive allows emission of an ascii string with the standard C
	/// escape characters embedded into it.  If a target doesn't support this, it
	/// can be set to null. Defaults to "\t.ascii\t"
	AsciiDirective = nullptr;

	/// If not null, this allows for special handling of zero terminated strings
	/// on this target.  This is commonly supported as ".asciz".  If a target
	/// doesn't support this, it can be set to null.  Defaults to "\t.asciz\t"
	AscizDirective = "db\t";
}
