//===-- PROL16RegisterInfo.h - PROL16 Register Information ---*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the PROL16 implementation of the TargetRegisterInfo class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_PROL16_PROL16REGISTERINFO_H
#define LLVM_LIB_TARGET_PROL16_PROL16REGISTERINFO_H

#include "llvm/CodeGen/TargetRegisterInfo.h"

#define GET_REGINFO_HEADER
#include "PROL16GenRegisterInfo.inc"

namespace llvm {

class PROL16RegisterInfo final : public PROL16GenRegisterInfo {
public:
	PROL16RegisterInfo();

	/// Return a null-terminated list of all of the callee-saved registers on
	/// this target. The register should be in the order of desired callee-save
	/// stack frame offset. The first register is closest to the incoming stack
	/// pointer if stack grows down, and vice versa.
	/// Notice: This function does not take into account disabled CSRs.
	///         In most cases you will want to use instead the function
	///         getCalleeSavedRegs that is implemented in MachineRegisterInfo.
	MCPhysReg const* getCalleeSavedRegs(MachineFunction const *MF) const override;

	/// Returns a bitset indexed by physical register number indicating if a
	/// register is a special register that has particular uses and should be
	/// considered unavailable at all times, e.g. stack pointer, return address.
	/// A reserved register:
	/// - is not allocatable
	/// - is considered always live
	/// - is ignored by liveness tracking
	/// It is often necessary to reserve the super registers of a reserved
	/// register as well, to avoid them getting allocated indirectly. You may use
	/// markSuperRegs() and checkAllSuperRegsMarked() in this case.
	BitVector getReservedRegs(MachineFunction const &MF) const override;

	/// This method must be overriden to eliminate abstract frame indices from
	/// instructions which may use them. The instruction referenced by the
	/// iterator contains an MO_FrameIndex operand which must be eliminated by
	/// this method. This method may modify or replace the specified instruction,
	/// as long as it keeps the iterator pointing at the finished product.
	/// SPAdj is the SP adjustment due to call frame setup instruction.
	/// FIOperandNum is the FI operand number.
	void eliminateFrameIndex(MachineBasicBlock::iterator MI,
	                         int SPAdj, unsigned FIOperandNum,
	                         RegScavenger *RS = nullptr) const override;

	/// getFrameRegister - This method should return the register used as a base
	/// for values allocated in the current stack frame.
	Register getFrameRegister(MachineFunction const &MF) const override;

	/// Returns true if the target requires (and can make use of) the register
	/// scavenger.
	bool requiresRegisterScavenging(MachineFunction const &MF) const override;

	/// Returns true if the target requires post PEI scavenging of registers for
	/// materializing frame index constants.
	bool requiresFrameIndexScavenging(MachineFunction const &MF) const override;

	unsigned calcAbsoluteOffset(int const offset) const;
};

} // end namespace llvm

#endif
