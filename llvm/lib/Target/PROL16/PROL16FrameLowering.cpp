//===-- PROL16FrameLowering.cpp - PROL16 Frame Information ------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the PROL16 implementation of TargetFrameLowering class.
//
//===----------------------------------------------------------------------===//

#include "PROL16FrameLowering.h"

#include "PROL16TargetMachine.h"
#include "PROL16Utils.h"

#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"

#include "llvm/Target/TargetOptions.h"

#include <cassert>

using namespace llvm;

#define DEBUG_TYPE "prol16"

// Defines symbolic names for the PROL16 instructions.
#define GET_INSTRINFO_ENUM
#include "PROL16GenInstrInfo.inc"

#define GET_REGINFO_ENUM
#include "PROL16GenRegisterInfo.inc"

// LAO = -1 because frame pointer is pushed to stack
PROL16FrameLowering::PROL16FrameLowering() : TargetFrameLowering(TargetFrameLowering::StackGrowsDown, 1, 0) {}

void PROL16FrameLowering::emitPrologue(MachineFunction &MF, MachineBasicBlock &MBB) const {
	LLVM_DEBUG(dbgs() << "PROL16FrameLowering::emitPrologue()\n");

	MachineFrameInfo &machineFrameInfo = MF.getFrameInfo();
	TargetInstrInfo const &targetInstrInfo = *(MF.getSubtarget().getInstrInfo());
	MachineRegisterInfo &machineRegisterInfo = MF.getRegInfo();

	MachineBasicBlock::iterator MBBItor = MBB.begin();

	// get the number of bytes to allocate from the frame info
	uint64_t const stackSize = machineFrameInfo.getStackSize();
	LLVM_DEBUG(dbgs() << "stack size = " << stackSize << '\n');
	if (stackSize == 0) { return; }

	uint64_t stackPointerAdjustment = 0;

	if (hasFP(MF)) {
		// calculate the required stack adjustment if the frame pointer is used
		// frame pointer is pushed to the stack -> reserve an appropriate stack slot for it
		uint64_t const frameSize = stackSize - 1;
		stackPointerAdjustment = frameSize;

		// update the frame offset adjustment
		machineFrameInfo.setOffsetAdjustment(-frameSize);

		pushAndUpdateFramePointer(MF, MBB, MBBItor);

		// mark the frame pointer as live-in in every block except the entry
		for (auto itor = std::next(MF.begin()); itor != MF.end(); ++itor) {
			itor->addLiveIn(PROL16::RFP);
		}
	} else {
		stackPointerAdjustment = stackSize;
	}

	LLVM_DEBUG(dbgs() << "calculated stack pointer adjustment = " << stackPointerAdjustment << '\n');

	DebugLoc debugLocation = MBBItor != MBB.end() ? MBBItor->getDebugLoc() : DebugLoc();

	// adjust stack pointer for the current call frame
	if (stackPointerAdjustment != 0) {
		unsigned const offsetRegister = machineRegisterInfo.createVirtualRegister(&PROL16::GR16RegClass);

		BuildMI(MBB, MBBItor, debugLocation, targetInstrInfo.get(PROL16::LOADI), offsetRegister)
			.addImm(prol16::util::calcOffset(stackPointerAdjustment));

		BuildMI(MBB, MBBItor, debugLocation, targetInstrInfo.get(PROL16::SUB), PROL16::RSP)
			.addReg(PROL16::RSP).addReg(offsetRegister, RegState::Kill);
	}
}

void PROL16FrameLowering::emitEpilogue(MachineFunction &MF, MachineBasicBlock &MBB) const {
	LLVM_DEBUG(dbgs() << "PROL16FrameLowering::emitEpilogue()\n");

	MachineFrameInfo &machineFrameInfo = MF.getFrameInfo();
	TargetInstrInfo const &targetInstrInfo = *(MF.getSubtarget().getInstrInfo());
	MachineRegisterInfo &machineRegisterInfo = MF.getRegInfo();

	MachineBasicBlock::iterator MBBItor = MBB.getLastNonDebugInstr();
	DebugLoc debugLocation = MBBItor->getDebugLoc();

	assert(MBBItor->getOpcode() == PROL16::JUMPret);
	assert(MBBItor->getOperand(0).getReg() == machineRegisterInfo.getTargetRegisterInfo()->getRARegister());

	// get the number of bytes to allocate from the frame info
	uint64_t const stackSize = machineFrameInfo.getStackSize();
	if (stackSize == 0) { return; }

	uint64_t stackPointerAdjustment = 0;

	if (hasFP(MF)) {
		uint64_t const frameSize = stackSize - 1;
		stackPointerAdjustment = frameSize;
	} else {
		stackPointerAdjustment = stackSize;
	}

	debugLocation = MBBItor->getDebugLoc();

	if (machineFrameInfo.hasVarSizedObjects()) {
		llvm_unreachable("Encountered unexpected variable sized objects while PROL16FrameLowering::emitEpilogue()");
	} else {
		// adjust stack pointer back to the caller's stack frame
		if (stackPointerAdjustment != 0) {
			unsigned const offsetRegister = machineRegisterInfo.createVirtualRegister(&PROL16::GR16RegClass);

			BuildMI(MBB, MBBItor, debugLocation, targetInstrInfo.get(PROL16::LOADI), offsetRegister)
				.addImm(prol16::util::calcOffset(stackPointerAdjustment));

			BuildMI(MBB, MBBItor, debugLocation, targetInstrInfo.get(PROL16::ADD), PROL16::RSP)
				.addReg(PROL16::RSP).addReg(offsetRegister, RegState::Kill);
		}
	}

	if (hasFP(MF)) {
		popFramePointer(MF, MBB, MBBItor);
	}
}

// Eliminate ADJCALLSTACKDOWN and ADJCALLSTACKUP pseudo instructions.
MachineBasicBlock::iterator	PROL16FrameLowering::eliminateCallFramePseudoInstr(MachineFunction &MF,
																			   MachineBasicBlock &MBB,
																			   MachineBasicBlock::iterator MI) const {
	LLVM_DEBUG(dbgs() << "PROL16FrameLowering::eliminateCallFramePseudoInstr()\n");

	PROL16InstrInfo const &targetInstrInfo = *static_cast<PROL16InstrInfo const*>(MF.getSubtarget().getInstrInfo());

	if (!hasReservedCallFrame(MF)) {
		llvm_unreachable("eliminateCallFramePseudoInstr() not implemented for the case (!hasReservedCallFrame(MF))");
	} else if (MI->getOpcode() == targetInstrInfo.getCallFrameDestroyOpcode()) {
		uint64_t const calleeAmount = targetInstrInfo.getFramePoppedByCallee(*MI);
		if (calleeAmount != 0) {
			llvm_unreachable("eliminateCallFramePseudoInstr() not implemented for the case (MI->getOpcode() == targetInstrInfo.getCallFrameDestroyOpcode())");
		}
	}

	return MBB.erase(MI);
}

bool PROL16FrameLowering::hasFP(MachineFunction const &MF) const {
	MachineFrameInfo const &machineFrameInfo = MF.getFrameInfo();

	return (MF.getTarget().Options.DisableFramePointerElim(MF) ||	// is frame pointer elimination disabled
			machineFrameInfo.hasVarSizedObjects() ||				// does the function have variable sized allocas
			machineFrameInfo.isFrameAddressTaken());
}

bool PROL16FrameLowering::hasReservedCallFrame(const MachineFunction &MF) const {
	// Not preserve stack space within prologue for outgoing variables when the
	// function contains variable size objects and let eliminateCallFramePseudoInstr()
	// preserve stack space for it.
	return !MF.getFrameInfo().hasVarSizedObjects();
}

void PROL16FrameLowering::pushAndUpdateFramePointer(MachineFunction &MF, MachineBasicBlock &MBB,
													MachineBasicBlock::iterator MBBItor) const {
	TargetInstrInfo const &targetInstrInfo = *(MF.getSubtarget().getInstrInfo());

	DebugLoc debugLocation = MBBItor != MBB.end() ? MBBItor->getDebugLoc() : DebugLoc();

	/**
	 * push frame pointer into the appropriate stack slot reserved before
	 */
	BuildMI(MBB, MBBItor, debugLocation, targetInstrInfo.get(PROL16::DEC), PROL16::RSP)
		.addReg(PROL16::RSP);

	BuildMI(MBB, MBBItor, debugLocation, targetInstrInfo.get(PROL16::STORE), PROL16::RFP)
		.addReg(PROL16::RSP);

	/**
	 * update frame pointer to the new base value
	 * (i.e. to the beginning of the current stack frame which is given through the stack pointer)
	 */
	BuildMI(MBB, MBBItor, debugLocation, targetInstrInfo.get(PROL16::MOVE), PROL16::RFP)
		.addReg(PROL16::RSP);
}

void PROL16FrameLowering::popFramePointer(MachineFunction &MF, MachineBasicBlock &MBB,
										  MachineBasicBlock::iterator MBBItor) const {
	TargetInstrInfo const &targetInstrInfo = *(MF.getSubtarget().getInstrInfo());

	DebugLoc debugLocation = MBBItor->getDebugLoc();

	BuildMI(MBB, MBBItor, debugLocation, targetInstrInfo.get(PROL16::LOAD), PROL16::RFP)
		.addReg(PROL16::RSP);

	BuildMI(MBB, MBBItor, debugLocation, targetInstrInfo.get(PROL16::INC), PROL16::RSP)
		.addReg(PROL16::RSP);
}
