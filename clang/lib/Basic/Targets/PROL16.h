//===--- PROL16.h - Declare PROL16 target feature support -------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file declares PROL16 TargetInfo objects.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_LIB_BASIC_TARGETS_PROL16_H
#define LLVM_CLANG_LIB_BASIC_TARGETS_PROL16_H

#include "clang/Basic/TargetInfo.h"
#include "clang/Basic/TargetOptions.h"
#include "llvm/ADT/Triple.h"
#include "llvm/Support/Compiler.h"

namespace clang {
namespace targets {

class LLVM_LIBRARY_VISIBILITY PROL16TargetInfo : public TargetInfo {
	static constexpr char const * const GCCRegNames[]{
			"rpc", "rra", "rsp",  "rfp",  "r4",  "r5",  "r6",  "r7",
			"r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15"
	};

public:
	PROL16TargetInfo(llvm::Triple const &Triple, TargetOptions const&);

	void getTargetDefines(LangOptions const &Opts,
						  MacroBuilder &Builder) const override;

	/// Returns a string of target-specific clobbers, in LLVM format.
	char const* getClobbers() const override;

	ArrayRef<const char*> getGCCRegNames() const override;
	ArrayRef<GCCRegAlias> getGCCRegAliases() const override;

	/// Return information about target-specific builtins for
	/// the current primary target, and info about which builtins are non-portable
	/// across the current set of primary and secondary targets.
	ArrayRef<Builtin::Info> getTargetBuiltins() const override;

	/// Returns the kind of __builtin_va_list type that should be used
	/// with this target.
	BuiltinVaListKind getBuiltinVaListKind() const override;

	bool validateAsmConstraint(const char *&Name,
							   TargetInfo::ConstraintInfo &info) const override;

	/// Determine whether the given target has the given feature.
	bool hasFeature(StringRef Feature) const override;
};

} // namespace targets
} // namespace clang

#endif // LLVM_CLANG_LIB_BASIC_TARGETS_PROL16_H
