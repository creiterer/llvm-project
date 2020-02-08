//===-- PROL16FrameLowering.h - Define frame lowering for PROL16 --*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_PROL16_PROL16FRAMELOWERING_H
#define LLVM_LIB_TARGET_PROL16_PROL16FRAMELOWERING_H

#include "llvm/CodeGen/TargetFrameLowering.h"

namespace llvm {

class PROL16FrameLowering final : public TargetFrameLowering {
public:
	PROL16FrameLowering();

	/// emitProlog/emitEpilog - These methods insert prolog and epilog code into
	/// the function.
	void emitPrologue(MachineFunction &MF, MachineBasicBlock &MBB) const override;
	void emitEpilogue(MachineFunction &MF, MachineBasicBlock &MBB) const override;

	/// This method is called during prolog/epilog code insertion to eliminate
	/// call frame setup and destroy pseudo instructions (but only if the Target
	/// is using them).  It is responsible for eliminating these instructions,
	/// replacing them with concrete instructions.  This method need only be
	/// implemented if using call frame setup/destroy pseudo instructions.
	/// Returns an iterator pointing to the instruction after the replaced one.
	MachineBasicBlock::iterator	eliminateCallFramePseudoInstr(MachineFunction &MF,
															  MachineBasicBlock &MBB,
															  MachineBasicBlock::iterator MI) const override;

	/// hasFP - Return true if the specified function should have a dedicated
	/// frame pointer register. For most targets this is true only if the function
	/// has variable sized allocas or if frame pointer elimination is disabled.
	bool hasFP(MachineFunction const &MF) const override;

	/// hasReservedCallFrame - Under normal circumstances, when a frame pointer is
	/// not required, we reserve argument space for call sites in the function
	/// immediately on entry to the current function. This eliminates the need for
	/// add/sub sp brackets around call sites. Returns true if the call frame is
	/// included as part of the stack frame.
	bool hasReservedCallFrame(const MachineFunction &MF) const override;

private:
	void pushAndUpdateFramePointer(MachineFunction &MF, MachineBasicBlock &MBB,
								   MachineBasicBlock::iterator MBBItor) const;

	void popFramePointer(MachineFunction &MF, MachineBasicBlock &MBB,
						 MachineBasicBlock::iterator MBBItor) const;
};

} // End llvm namespace

#endif
