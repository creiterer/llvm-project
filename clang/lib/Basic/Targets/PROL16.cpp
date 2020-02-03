//===--- PROL16.cpp - Implement PROL16 target feature support -------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements PROL16 TargetInfo objects.
//
//===----------------------------------------------------------------------===//

#include "PROL16.h"
#include "clang/Basic/MacroBuilder.h"

namespace clang {
namespace targets {

constexpr char const * const PROL16TargetInfo::GCCRegNames[];

PROL16TargetInfo::PROL16TargetInfo(llvm::Triple const &Triple, TargetOptions const&)
: TargetInfo(Triple) {
	TLSSupported = false;

	IntWidth = 16;
	IntAlign = 16;

	LongWidth = 32;
	LongAlign = 16;

	LongLongWidth = 64;
	LongLongAlign = 16;

	FloatWidth = 32;
	FloatAlign = 16;

	DoubleWidth = LongDoubleWidth = 64;
	DoubleAlign = LongDoubleAlign = 16;

	PointerWidth = 16;
	PointerAlign = 16;
	SuitableAlign = 16;

	SizeType = UnsignedInt;
	IntMaxType = SignedLongLong;
	IntPtrType = SignedInt;
	PtrDiffType = SignedInt;
	SigAtomicType = SignedLong;

	// keep in sync with PROL16TargetMachine
	resetDataLayout("e-S16-p:16:16-i32:16-i64:16-f32:16-f64:16-n16");
}

void PROL16TargetInfo::getTargetDefines(LangOptions const &Opts,
										MacroBuilder &Builder) const {
	Builder.defineMacro("PROL16");
	Builder.defineMacro("__PROL16__");
}

char const* PROL16TargetInfo::getClobbers() const {
	return "";
}

ArrayRef<const char*> PROL16TargetInfo::getGCCRegNames() const {
	return llvm::makeArrayRef(GCCRegNames);
}

ArrayRef<TargetInfo::GCCRegAlias> PROL16TargetInfo::getGCCRegAliases() const {
	// no aliases
	return None;
}

ArrayRef<Builtin::Info> PROL16TargetInfo::getTargetBuiltins() const {
	return None;
}

TargetInfo::BuiltinVaListKind PROL16TargetInfo::getBuiltinVaListKind() const {
	return TargetInfo::CharPtrBuiltinVaList;
}

bool PROL16TargetInfo::validateAsmConstraint(const char *&Name,
											 TargetInfo::ConstraintInfo &info) const {
	return false;
}

bool PROL16TargetInfo::hasFeature(StringRef Feature) const {
	return Feature == "prol16";
}

} // namespace targets
} // namespace clang
