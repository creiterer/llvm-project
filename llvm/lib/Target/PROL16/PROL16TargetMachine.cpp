//===-- PROL16TargetMachine.cpp - Define TargetMachine for PROL16 -----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Implements the info about PROL16 target spec.
//
//===----------------------------------------------------------------------===//

#include "PROL16TargetMachine.h"
#include "PROL16TargetLoweringObjectFile.h"

#include "llvm/Support/TargetRegistry.h"
#include "llvm/CodeGen/TargetPassConfig.h"

#include "PROL16.h"
#include "MCTargetDesc/PROL16MCTargetDesc.h"

using namespace llvm;

extern "C" void LLVMInitializePROL16Target() {
	// Register the target.
	RegisterTargetMachine<PROL16TargetMachine> X(getThePROL16Target());
}

namespace {
	std::string computeDataLayoutString() {
		/// https://releases.llvm.org/7.0.0/docs/LangRef.html#data-layout
		return "e-S16-p:16:16-i32:16-i64:16-f32:16-f64:16-n16";
	}

	Reloc::Model getEffectiveRelocationModel(Optional<Reloc::Model> const &relocationModel) {
		if (relocationModel) {
			return *relocationModel;
		}

		return Reloc::Static;
	}
}

PROL16TargetMachine::PROL16TargetMachine(Target const &target,
										 Triple const &targetTriple,
										 StringRef cpu,
										 StringRef featureString,
										 TargetOptions const &targetOptions,
										 Optional<Reloc::Model> relocationModel,
										 Optional<CodeModel::Model> codeModel,
										 CodeGenOpt::Level optimizationLevel,
										 bool jit)
: LLVMTargetMachine(target, computeDataLayoutString(), targetTriple, cpu, featureString, targetOptions,
		getEffectiveRelocationModel(relocationModel), getEffectiveCodeModel(codeModel, CodeModel::Small), optimizationLevel),
  subtarget(targetTriple, cpu, featureString, *this),
  // FIXME(PROL16): ELF probably doesn't fit for prol16
  targetLoweringObjectFile(make_unique<PROL16TargetLoweringObjectFile>()) {
	initAsmInfo();
}

PROL16Subtarget const* PROL16TargetMachine::getSubtargetImpl(Function const&) const {
	return &subtarget;
}

TargetLoweringObjectFile* PROL16TargetMachine::getObjFileLowering() const {
	return targetLoweringObjectFile.get();
}

namespace {

/// PROL16 Code Generator Pass Configuration Options
class PROL16PassConfig final : public TargetPassConfig {
public:
	PROL16PassConfig(PROL16TargetMachine &targetMachine, PassManagerBase &passManager);

	PROL16TargetMachine& getPROL16TargetMachine() const;

	/// addInstSelector - This method should install an instruction selector pass,
	/// which converts from LLVM code to machine instructions.
	bool addInstSelector() override;
};

PROL16PassConfig::PROL16PassConfig(PROL16TargetMachine &targetMachine, PassManagerBase &passManager)
	: TargetPassConfig(targetMachine, passManager) {}

PROL16TargetMachine& PROL16PassConfig::getPROL16TargetMachine() const {
	return getTM<PROL16TargetMachine>();
}

bool PROL16PassConfig::addInstSelector() {
	// install an instruction selector
	addPass(createPROL16ISelDag(getPROL16TargetMachine()));

	return false;
}

}

TargetPassConfig* PROL16TargetMachine::createPassConfig(PassManagerBase &PM) {
	return new PROL16PassConfig(*this, PM);
}
