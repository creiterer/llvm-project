//===-- PROL16InstrInfo.cpp - PROL16 Instruction Information ----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the PROL16 implementation of the TargetInstrInfo class.
//
//===----------------------------------------------------------------------===//

#include "PROL16InstrInfo.h"

#include "PROL16ConditionCodes.h"

#include "llvm/IR/DebugLoc.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineMemOperand.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"

using namespace llvm;

#define DEBUG_TYPE "prol16"

#define GET_INSTRINFO_CTOR_DTOR
#define GET_INSTRINFO_ENUM
#include "PROL16GenInstrInfo.inc"

PROL16InstrInfo::PROL16InstrInfo() : PROL16GenInstrInfo(PROL16::ADJCALLSTACKDOWN, PROL16::ADJCALLSTACKUP) {}

TargetRegisterInfo const& PROL16InstrInfo::getRegisterInfo() const {
	return registerInfo;
}

void PROL16InstrInfo::storeRegToStackSlot(MachineBasicBlock &MBB, MachineBasicBlock::iterator MI,
										  unsigned SrcReg, bool isKill, int FrameIndex,
										  TargetRegisterClass const *RC, TargetRegisterInfo const *TRI) const {
    LLVM_DEBUG(dbgs() << "PROL16InstrInfo::storeRegToStackSlot()\n");

	DebugLoc debugLocation;
	if (MI != MBB.end()) {
		debugLocation = MI->getDebugLoc();
	}

	MachineFunction &machineFunction = *MBB.getParent();
	MachineFrameInfo &machineFrameInfo = machineFunction.getFrameInfo();

	MachineMemOperand *machineMemoryOperand = machineFunction.getMachineMemOperand(MachinePointerInfo::getFixedStack(machineFunction, FrameIndex),
																				   MachineMemOperand::MOStore,
																				   machineFrameInfo.getObjectSize(FrameIndex),
																				   machineFrameInfo.getObjectAlignment(FrameIndex));

	if (RC == &PROL16::GR16RegClass) {
		BuildMI(MBB, MI, debugLocation, get(PROL16::STORE))
			.addReg(SrcReg, getKillRegState(isKill))
			.addFrameIndex(FrameIndex)
			.addMemOperand(machineMemoryOperand);
	} else {
		llvm_unreachable("PROL16InstrInfo::storeRegToStackSlot: cannot store this register to stack slot!");
	}
}

void PROL16InstrInfo::loadRegFromStackSlot(MachineBasicBlock &MBB, MachineBasicBlock::iterator MI,
										   unsigned DestReg, int FrameIndex,
										   TargetRegisterClass const *RC, TargetRegisterInfo const *TRI) const {
	LLVM_DEBUG(dbgs() << "PROL16InstrInfo::loadRegFromStackSlot()\n");

	DebugLoc debugLocation;
	if (MI != MBB.end()) {
		debugLocation = MI->getDebugLoc();
	}

	MachineFunction &machineFunction = *MBB.getParent();
	MachineFrameInfo &machineFrameInfo = machineFunction.getFrameInfo();

	MachineMemOperand *machineMemoryOperand = machineFunction.getMachineMemOperand(MachinePointerInfo::getFixedStack(machineFunction, FrameIndex),
																				   MachineMemOperand::MOLoad,
																				   machineFrameInfo.getObjectSize(FrameIndex),
																				   machineFrameInfo.getObjectAlignment(FrameIndex));

	if (RC == &PROL16::GR16RegClass) {
		BuildMI(MBB, MI, debugLocation, get(PROL16::LOAD))
			.addReg(DestReg, getDefRegState(true))
			.addFrameIndex(FrameIndex)
			.addMemOperand(machineMemoryOperand);
	} else {
		llvm_unreachable("PROL16InstrInfo::loadRegFromStackSlot: cannot load this register from stack slot!");
	}
}

void PROL16InstrInfo::copyPhysReg(MachineBasicBlock &MBB, MachineBasicBlock::iterator MI, const DebugLoc &DL,
								  unsigned DestReg, unsigned SrcReg, bool KillSrc) const {
	BuildMI(MBB, MI, DL, get(PROL16::MOVE), DestReg).addReg(SrcReg, getKillRegState(KillSrc));
}

bool PROL16InstrInfo::expandPostRAPseudo(MachineInstr &MI) const {
	LLVM_DEBUG(dbgs() << "PROL16InstrInfo::expandPostRAPseudo()\n");
	LLVM_DEBUG(MI.dump());

	switch (MI.getOpcode()) {
	case PROL16::JUMPcall:
		emitJumpCall(MI);
		return true;
	case PROL16::JUMPcc:
		emitJumpConditional(MI);
		return true;
	case PROL16::IMPLICIT_DEF:
		emitImplicitDef(MI);
		return true;
	}

	return false;
}

uint64_t PROL16InstrInfo::getFramePoppedByCallee(MachineInstr const &machineInstruction) const {
	assert(isFrameInstr(machineInstruction) && "Not a frame instruction");
	assert(machineInstruction.getOperand(1).getImm() >= 0 && "Size must not be negative");

	return machineInstruction.getOperand(1).getImm();
}

void PROL16InstrInfo::emitJumpCall(MachineInstr &machineInstruction) const {
	LLVM_DEBUG(dbgs() << "PROL16InstrInfo::emitJumpCall()\n");
	LLVM_DEBUG(machineInstruction.dump());

	DebugLoc debugLocation = machineInstruction.getDebugLoc();
	MachineBasicBlock *machineBasicBlock = machineInstruction.getParent();
	TargetInstrInfo const &targetInstrInfo = *(machineBasicBlock->getParent()->getSubtarget().getInstrInfo());

	unsigned const targetAddressRegister = machineInstruction.getOperand(0).getReg();
	unsigned const programCounterRegister = machineInstruction.getOperand(1).getReg();
	unsigned const returnAddressRegister = machineInstruction.getOperand(2).getReg();
	unsigned const stackPointerRegister = machineInstruction.getOperand(3).getReg();

	/*******************************************
	 * push the return address register to stack
	 *******************************************/

	// decrement stack pointer to point to a free slot
	BuildMI(*machineBasicBlock, machineInstruction, debugLocation, targetInstrInfo.get(PROL16::DEC), stackPointerRegister)
		.addReg(stackPointerRegister);

	// save the current value of the return address register to the stack
	BuildMI(*machineBasicBlock, machineInstruction, debugLocation, targetInstrInfo.get(PROL16::STORE), returnAddressRegister)
		.addReg(stackPointerRegister);

	/*******************************************************************************************
	 * let the return address register point to the next instruction after the JUMP instruction,
	 * that is, the next instruction that should be executed after the function call
	 *******************************************************************************************/

	BuildMI(*machineBasicBlock, machineInstruction, debugLocation, targetInstrInfo.get(PROL16::LOADI), returnAddressRegister)
		.addImm(1);

	BuildMI(*machineBasicBlock, machineInstruction, debugLocation, targetInstrInfo.get(PROL16::ADD), returnAddressRegister)
		.addReg(returnAddressRegister).addReg(programCounterRegister);

	/************************************************************
	 * do the actual function call -> jump to the target function
	 ************************************************************/
	BuildMI(*machineBasicBlock, machineInstruction, debugLocation, targetInstrInfo.get(PROL16::JUMP))
		.addReg(targetAddressRegister, RegState::Kill)
		.add(ArrayRef<MachineOperand>(machineInstruction.operands_begin() + 4, machineInstruction.operands_end()));

	/********************************************
	 * pop the return address register from stack
	 ********************************************/

	// reload the return address register with the appropriate return address for the current frame
	BuildMI(*machineBasicBlock, machineInstruction, debugLocation, targetInstrInfo.get(PROL16::LOAD), returnAddressRegister)
		.addReg(stackPointerRegister);

	// let the stack pointer point to the last filled value
	BuildMI(*machineBasicBlock, machineInstruction, debugLocation, targetInstrInfo.get(PROL16::INC), stackPointerRegister)
		.addReg(stackPointerRegister);

	machineInstruction.eraseFromParent();
}

void PROL16InstrInfo::emitJumpConditional(MachineInstr &machineInstruction) const {
	LLVM_DEBUG(dbgs() << "PROL16InstrInfo::emitJumpConditional()\n");
	LLVM_DEBUG(machineInstruction.dump());

	DebugLoc debugLocation = machineInstruction.getDebugLoc();
	MachineBasicBlock *machineBasicBlock = machineInstruction.getParent();
	TargetInstrInfo const &targetInstrInfo = *(machineBasicBlock->getParent()->getSubtarget().getInstrInfo());

	unsigned const targetAddressRegister = machineInstruction.getOperand(0).getReg();
	unsigned const lhsRegister = machineInstruction.getOperand(1).getReg();
	unsigned const rhsRegister = machineInstruction.getOperand(2).getReg();
	PROL16CC::ConditionCode const targetConditionCode = static_cast<PROL16CC::ConditionCode>(machineInstruction.getOperand(3).getImm());

	BuildMI(*machineBasicBlock, machineInstruction, debugLocation, targetInstrInfo.get(PROL16::COMP))
		.addReg(lhsRegister)
		.addReg(rhsRegister);

	switch (targetConditionCode) {
	case PROL16CC::EQ:
		BuildMI(*machineBasicBlock, machineInstruction, debugLocation, targetInstrInfo.get(PROL16::JUMPZ))
			.addReg(targetAddressRegister);
		break;

	case PROL16CC::NE: {
		/// @see PROL16TargetLowering::emitJumpnz()
		auto nextInstructionItor = std::next(MachineBasicBlock::iterator(machineInstruction));
		assert(nextInstructionItor != machineBasicBlock->end() && "jumpnz expansion: no instruction after jumpnz");
		assert((nextInstructionItor->getOpcode() == PROL16::BR || nextInstructionItor->getOpcode() == PROL16::JUMP) && "jumpnz expansion: unexpected non-br instruction after jumpnz");

		BuildMI(*machineBasicBlock, machineInstruction, debugLocation, targetInstrInfo.get(PROL16::JUMPZ))
			.add(nextInstructionItor->getOperand(0));

		nextInstructionItor->getOperand(0).ChangeToRegister(targetAddressRegister, false, false, true);

		break;
	}

	case PROL16CC::LT:
		BuildMI(*machineBasicBlock, machineInstruction, debugLocation, targetInstrInfo.get(PROL16::JUMPC))
			.addReg(targetAddressRegister);
		break;

	case PROL16CC::LE:
		BuildMI(*machineBasicBlock, machineInstruction, debugLocation, targetInstrInfo.get(PROL16::JUMPC), targetAddressRegister);

		BuildMI(*machineBasicBlock, machineInstruction, debugLocation, targetInstrInfo.get(PROL16::JUMPZ))
			.addReg(targetAddressRegister, RegState::Kill);

		break;

	default:
		llvm_unreachable("Invalid PROL16 condition code");
	}

	machineInstruction.eraseFromParent();
}

void PROL16InstrInfo::emitImplicitDef(MachineInstr &machineInstruction) const {
	LLVM_DEBUG(dbgs() << "PROL16InstrInfo::emitImplicitDef()\n");
	LLVM_DEBUG(machineInstruction.dump());

	DebugLoc debugLocation = machineInstruction.getDebugLoc();
	MachineBasicBlock *machineBasicBlock = machineInstruction.getParent();
	TargetInstrInfo const &targetInstrInfo = *(machineBasicBlock->getParent()->getSubtarget().getInstrInfo());

	unsigned const reg = machineInstruction.getOperand(0).getReg();

	// Load 0 into implicitly defined registers
	BuildMI(*machineBasicBlock, machineInstruction, debugLocation, targetInstrInfo.get(PROL16::LOADI), reg)
		.addImm(0);

	machineInstruction.eraseFromParent();
}
