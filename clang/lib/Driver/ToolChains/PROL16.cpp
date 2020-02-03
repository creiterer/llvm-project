//===--- PROL16.cpp - PROL16 Helpers for Tools ------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "PROL16.h"

#include "CommonArgs.h"
#include "Gnu.h"
#include "InputInfo.h"
#include "clang/Driver/Compilation.h"
#include "clang/Driver/Multilib.h"
#include "clang/Driver/Options.h"
#include "llvm/Option/ArgList.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Path.h"

using namespace clang::driver;
using namespace clang::driver::toolchains;
using namespace clang::driver::tools;
using namespace clang;
using namespace llvm::opt;

static bool isMCUSupported(StringRef const mcu) {
	return llvm::StringSwitch<bool>(mcu)
#define PROL16_MCU(NAME) .Case(NAME, true)
#include "clang/Basic/PROL16Target.def"
      .Default(false);
}

/// PROL16 Toolchain
PROL16ToolChain::PROL16ToolChain(Driver const &driver, llvm::Triple const &triple,
								 llvm::opt::ArgList const &args)
: Generic_ELF(driver, triple, args) {
	StringRef multilibSuffix = "";

	GCCInstallation.init(triple, args);

	if (GCCInstallation.isValid()) {
		multilibSuffix = GCCInstallation.getMultilib().gccSuffix();

		SmallString<128> gccBinPath;
		llvm::sys::path::append(gccBinPath,
								GCCInstallation.getParentLibPath(), "..", "bin");

		addPathIfExists(driver, gccBinPath, getProgramPaths());

		SmallString<128> gccRootPath;
		llvm::sys::path::append(gccRootPath,
								GCCInstallation.getInstallPath(), multilibSuffix);
		addPathIfExists(driver, gccRootPath, getFilePaths());
	}

	SmallString<128> SysRootDir(computeSysRoot());
	llvm::sys::path::append(SysRootDir, "lib", multilibSuffix);
	addPathIfExists(driver, SysRootDir, getFilePaths());
}

void PROL16ToolChain::AddClangSystemIncludeArgs(llvm::opt::ArgList const &DriverArgs,
												llvm::opt::ArgStringList &CC1Args) const {
	if (DriverArgs.hasArg(options::OPT_nostdinc) || DriverArgs.hasArg(options::OPT_nostdlibinc)) {
		return;
	}

	SmallString<128> sysRootDir(computeSysRoot());
	llvm::sys::path::append(sysRootDir, "include");
	addSystemInclude(DriverArgs, CC1Args, sysRootDir.str());
}

void PROL16ToolChain::addClangTargetOptions(llvm::opt::ArgList const &DriverArgs,
											llvm::opt::ArgStringList &CC1Args,
											Action::OffloadKind DeviceOffloadKind) const {
	CC1Args.push_back("-nostdsysteminc");
}


std::string PROL16ToolChain::computeSysRoot() const {
	if (!getDriver().SysRoot.empty()) {
		return getDriver().SysRoot;
	}

	SmallString<128> sysRootDir;
	if (GCCInstallation.isValid()) {
		llvm::sys::path::append(sysRootDir, GCCInstallation.getParentLibPath(), "..",
								GCCInstallation.getTriple().str());
	} else {
		llvm::sys::path::append(sysRootDir, getDriver().Dir, "..", getTriple().str());
	}

	return sysRootDir.str();
}

void prol16::getPROL16TargetFeatures(Driver const &driver, llvm::opt::ArgList const &args,
									 std::vector<llvm::StringRef> &features) {
	Arg const * const mcu = args.getLastArg(options::OPT_mmcu_EQ);
	if (mcu != nullptr && !isMCUSupported(mcu->getValue())) {
		driver.Diag(diag::err_drv_clang_unsupported) << mcu->getValue();
		return;
	}
}
