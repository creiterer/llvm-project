//===--- PROL16.h - PROL16-specific Tool Helpers ----------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_LIB_DRIVER_TOOLCHAINS_PROL16_H
#define LLVM_CLANG_LIB_DRIVER_TOOLCHAINS_PROL16_H

#include "Gnu.h"
#include "InputInfo.h"
#include "clang/Driver/Driver.h"
#include "clang/Driver/DriverDiagnostic.h"
#include "clang/Driver/Tool.h"
#include "clang/Driver/ToolChain.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Option/Option.h"

#include <string>
#include <vector>

namespace clang {
namespace driver {
namespace toolchains {

class LLVM_LIBRARY_VISIBILITY PROL16ToolChain : public Generic_ELF {
public:
	PROL16ToolChain(Driver const &driver, llvm::Triple const &triple,
					llvm::opt::ArgList const &args);

	/// Add the clang cc1 arguments for system include paths.
	///
	/// This routine is responsible for adding the necessary cc1 arguments to
	/// include headers from standard system header directories.
	void AddClangSystemIncludeArgs(llvm::opt::ArgList const &DriverArgs,
								   llvm::opt::ArgStringList &CC1Args) const override;

	/// Add options that need to be passed to cc1 for this target.
	void addClangTargetOptions(llvm::opt::ArgList const &DriverArgs,
							   llvm::opt::ArgStringList &CC1Args,
							   Action::OffloadKind DeviceOffloadKind) const override;

	bool isPICDefault() const override { return false; }
	bool isPIEDefault() const override { return false; }
	bool isPICDefaultForced() const override { return false; }

private:
	std::string computeSysRoot() const;
};

} // end namespace toolchains

namespace tools {
namespace prol16 {

void getPROL16TargetFeatures(Driver const &driver, llvm::opt::ArgList const &args,
                             std::vector<llvm::StringRef> &features);

} // end namespace prol16
} // end namespace tools
} // end namespace driver
} // end namespace clang

#endif // LLVM_CLANG_LIB_DRIVER_TOOLCHAINS_PROL16_H
