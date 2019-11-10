//===-- PROL16RegisterInfo.cpp - PROL16 Register Information ----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the PROL16 implementation of the TargetRegisterInfo class.
//
//===----------------------------------------------------------------------===//

#include "PROL16RegisterInfo.h"

#include <cstdlib>

#include "llvm/CodeGen/MachineFunction.h"		// also needed in PROL16GenRegisterInfo.inc
#include "llvm/CodeGen/TargetSubtargetInfo.h"	// also needed in PROL16GenRegisterInfo.inc
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/RegisterScavenging.h"
#include "llvm/CodeGen/TargetInstrInfo.h"

#include "PROL16FrameLowering.h"				// also needed in  PROL16GenRegisterInfo.inc

using namespace llvm;

#define DEBUG_TYPE "prol16"

#define GET_REGINFO_TARGET_DESC
#define GET_REGINFO_ENUM
#include "PROL16GenRegisterInfo.inc"

// Defines symbolic names for the PROL16 instructions.
#define GET_INSTRINFO_ENUM
#include "PROL16GenInstrInfo.inc"

PROL16RegisterInfo::PROL16RegisterInfo() : PROL16GenRegisterInfo(PROL16::RRA, 0, 0, PROL16::RPC) {}

MCPhysReg const* PROL16RegisterInfo::getCalleeSavedRegs(MachineFunction const *MF) const {
	static MCPhysReg const calleeSavedRegisters[] = { 0 };

	return calleeSavedRegisters;
}

BitVector PROL16RegisterInfo::getReservedRegs(MachineFunction const &MF) const {
	BitVector reservedRegisters(getNumRegs());

	// Mark the special registers as reserved
	reservedRegisters.set(getProgramCounter());
	reservedRegisters.set(getRARegister());
	reservedRegisters.set(PROL16::RSP);
	reservedRegisters.set(PROL16::RFP);		// for simplicity (at least at the beginning), always reserve rfp

//	if (getFrameLowering(MF)->hasFP(MF)) {
//		reservedRegisters.set(PROL16::RFP);
//	}

	return reservedRegisters;
}

/**
 * Replace the abstract frame index, denoted by %stack.* (e.g. %stack.1), with the corresponding stack address.
 */
void PROL16RegisterInfo::eliminateFrameIndex(MachineBasicBlock::iterator MI, int SPAdj, unsigned FIOperandNum,
											 RegScavenger *RS) const {
	assert(SPAdj == 0 && "unexpected stack pointer adjustment");

	LLVM_DEBUG(dbgs() << "PROL16RegisterInfo::eliminateFrameIndex()\n");
	LLVM_DEBUG(MI->dump());

	MachineInstr &machineInstruction = *MI;
	MachineBasicBlock &machineBasicBlock = *(machineInstruction.getParent());
	MachineFunction &machineFunction = *(machineBasicBlock.getParent());
	TargetInstrInfo const &targetInstrInfo = *(machineFunction.getSubtarget().getInstrInfo());

	// get the frame index from the frame index operand
	int const frameIndex = machineInstruction.getOperand(FIOperandNum).getIndex();

	unsigned const frameRegister = getFrameRegister(machineFunction);

	// get the frame offset (i.e. the offset relative to the frame pointer) of the object denoted by the frame index
	int frameOffset = machineFunction.getFrameInfo().getObjectOffset(frameIndex);

	/**
	 * It is possible to do post-RA register scavenging after frame index elimination
	 * if requiresRegisterScavenging() and requiresFrameIndexScavenging() return true
	 *
	 * @deprecated: the following statement results in "Assertion `RegNo && RegNo < 19 && "Invalid register number!"' failed."
	 * because register allocation is already done at this point so the created virtual register doesn't get
	 * mapped to a physical register!
	 */

	/**
	 * Compute the actual stack address of the object denoted by the frame index and replace the abstract
	 * frame index with the real stack address of that object.
	 *
	 * General Approach:
	 * 1. Load the frame offset of the object corresponding to the frame index into some temporary register.
	 * 2. Copy the frame pointer into some temporary register.
	 * 3. Compute the effective stack address by subtracting the frame offset from the frame pointer.
	 * 4. Replace the frame index operand with the actual stack address.
	 */

	// check if frame index elimination is done for an expanded LOADrm/STORErm instruction
	if (machineInstruction.getOpcode() == PROL16::MOVE) {
		/**
		 * Frame Index Elimination for an Expanded LOADrm/STORErm Instruction
		 * ==================================================================
		 * Approach:
		 * Since expanded LOADrm/STORErm instructions already consist of a MOVE and LOADI instruction,
		 * reuse them for frame index elimination instead of inserting new ones like the conventional approach
		 * below does.
		 *
		 * For example:
		 * 		STORErm %stack.1, 0, killed %0:gr16 :: (store 1 into %ir.2)
		 *
		 * is expanded to
		 * 		%12:gr16 = MOVE %stack.1
		 * 		%13:gr16 = LOADI 0
		 * 		%12:gr16 = SUB %12:gr16, killed %13:gr16
		 * 		STORE %0:gr16, killed %12:gr16
		 *
		 * which becomes
		 * 		$r5 = MOVE $rfp
		 * 		$r6 = LOADI 9
		 * 		$r5 = SUB $r5, killed $r6
		 * 		STORE killed $r4, killed $r5
		 *
		 * after frame index elimination.
		 */

		machineInstruction.getOperand(FIOperandNum).ChangeToRegister(frameRegister, false);

		auto nextMachineInstruction = std::next(MI);
		assert(nextMachineInstruction->getOpcode() == PROL16::LOADI && "unexpected non-loadi instruction for frame index elimination");
		frameOffset += nextMachineInstruction->getOperand(1).getImm();

		if (std::abs(frameOffset) % 2 != 0) {
			llvm_unreachable("Encountered uneven frame offset during frame index elimination!");
		}

		nextMachineInstruction->getOperand(1).setImm(std::abs(frameOffset) / 2);
	} else if ((machineInstruction.getOpcode() == PROL16::STORE) || (machineInstruction.getOpcode() == PROL16::LOAD)) {
		/**
		 * Frame Index Elimination for Conventional LOAD/STORE Instructions
		 * ================================================================
		 * For example:
		 * 		$r4 = LOAD %stack.2 :: (load 2 from %stack.2)
		 *
		 * becomes
		 * 		$r6 = LOADI 14
		 * 		$r4 = MOVE $rfp
		 * 		$r4 = SUB $r4, killed $r6
		 * 		$r4 = LOAD killed $r4 :: (load 2 from %stack.2)
		 *
		 * after frame index elimination.
		 * Therefore, it follows exactly the general approach described above.
		 */

		unsigned const offsetRegister = machineFunction.getRegInfo().createVirtualRegister(&PROL16::GR16RegClass);

		if (std::abs(frameOffset) % 2 != 0) {
			llvm_unreachable("Encountered uneven frame offset during frame index elimination!");
		}

		// insert instruction to load the offset into a register
		BuildMI(machineBasicBlock, MI, machineInstruction.getDebugLoc(), targetInstrInfo.get(PROL16::LOADI), offsetRegister)
			.addImm(std::abs(frameOffset) / 2);

		// since the stack grows down, the offset is usually negative!
		if (frameOffset < 0) {
			unsigned const tmpBaseRegister = machineFunction.getRegInfo().createVirtualRegister(&PROL16::GR16RegClass);

			// insert instruction to copy the frame register into some free register
			BuildMI(machineBasicBlock, MI, machineInstruction.getDebugLoc(), targetInstrInfo.get(PROL16::MOVE), tmpBaseRegister)
				.addReg(frameRegister);

			// insert instruction to subtract the offset from the base register copy
			BuildMI(machineBasicBlock, MI, machineInstruction.getDebugLoc(), targetInstrInfo.get(PROL16::SUB), tmpBaseRegister)
				.addReg(tmpBaseRegister).addReg(offsetRegister, RegState::Kill);

			// change frame index operand to use the register holding the computed stack address
			machineInstruction.getOperand(FIOperandNum).ChangeToRegister(tmpBaseRegister, false, false, true);
		} else {
			// insert instruction to add the offset to the base address
			BuildMI(machineBasicBlock, MI, machineInstruction.getDebugLoc(), targetInstrInfo.get(PROL16::ADD), offsetRegister)
				.addReg(offsetRegister).addReg(frameRegister);

			// change frame index operand to use the register holding the computed stack address
			machineInstruction.getOperand(FIOperandNum).ChangeToRegister(offsetRegister, false, false, true);
		}
	} else {
		llvm_unreachable("PROL16RegisterInfo::eliminateFrameIndex: encountered unexpected instruction for frame index elimination");
	}
}

unsigned PROL16RegisterInfo::getFrameRegister(MachineFunction const &MF) const {
	return getFrameLowering(MF)->hasFP(MF) ? PROL16::RFP : PROL16::RSP;
}

bool PROL16RegisterInfo::requiresRegisterScavenging(const MachineFunction &MF) const {
	// needed, so that eliminateFrameIndex() can use the RegScavenger (otherwise it would be nullptr)
	return true;
}

bool PROL16RegisterInfo::requiresFrameIndexScavenging(MachineFunction const &MF) const {
	// needed to scavenge virtual registers inserted by frame index elimination
	return true;
}
