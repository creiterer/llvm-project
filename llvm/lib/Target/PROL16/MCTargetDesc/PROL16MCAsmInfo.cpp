//===- PROL16MCAsmInfo.cpp - PROL16 asm properties --------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
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

	/// This should be set to the directive used to get some number of zero bytes
	/// emitted to the current section.  Common cases are "\t.zero\t" and
	/// "\t.space\t".  If this is set to null, the Data*bitsDirective's will be
	/// used to emit zero bytes.  Defaults to "\t.zero\t"
	ZeroDirective = nullptr;

	/// This directive allows emission of an ascii string with the standard C
	/// escape characters embedded into it.  If a target doesn't support this, it
	/// can be set to null. Defaults to "\t.ascii\t"
	AsciiDirective = "db\t";

	/// If not null, this allows for special handling of zero terminated strings
	/// on this target.  This is commonly supported as ".asciz".  If a target
	/// doesn't support this, it can be set to null.  Defaults to "\t.asciz\t"
	AscizDirective = "db\t";

	/// These directives are used to output some unit of integer data to the
	/// current section.  If a data directive is set to null, smaller data
	/// directives will be used to emit the large sizes.  Defaults to "\t.byte\t",
	/// "\t.short\t", "\t.long\t", "\t.quad\t"
	Data8bitsDirective = "db\t";
	Data16bitsDirective = "db\t";
	Data32bitsDirective = nullptr;
	Data64bitsDirective = nullptr;

	AlignmentDirective = nullptr;

	/// This is the directive used to declare a global entity. Defaults to
	/// ".globl".
	GlobalDirective = nullptr;

	/// Used to declare a global as being a weak symbol. Defaults to ".weak".
	WeakDirective = nullptr;
}
