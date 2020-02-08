//===-- PROL16TargetMachine.h - Define TargetMachine for PROL16 ---*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file declares the PROL16 specific subclass of TargetMachine.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_PROL16_PROL16TARGETMACHINE_H
#define LLVM_LIB_TARGET_PROL16_PROL16TARGETMACHINE_H

#include <memory>

#include "llvm/Target/TargetMachine.h"

#include "PROL16Subtarget.h"

namespace llvm {

class PROL16TargetMachine final : public LLVMTargetMachine {
public:
	PROL16TargetMachine(Target const &target,
						Triple const &targetTriple,
						StringRef cpu,
						StringRef featureString,
						TargetOptions const &targetOptions,
						Optional<Reloc::Model> relocationModel,
						Optional<CodeModel::Model> codeModel,
						CodeGenOpt::Level optimizationLevel,
						bool jit);

	~PROL16TargetMachine() = default;

	/// Virtual method implemented by subclasses that returns a reference to that
	/// target's TargetSubtargetInfo-derived member variable.
	PROL16Subtarget const* getSubtargetImpl(Function const&) const override;

	/// Create a pass configuration object to be used by addPassToEmitX methods
	/// for generating a pipeline of CodeGen passes.
	TargetPassConfig* createPassConfig(PassManagerBase &PM) override;

	TargetLoweringObjectFile* getObjFileLowering() const override;

private:
	PROL16Subtarget subtarget;
	std::unique_ptr<TargetLoweringObjectFile> targetLoweringObjectFile;
};

} // end namespace llvm

#endif
