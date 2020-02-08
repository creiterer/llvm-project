//===-- PROL16InstrInfo.h - PROL16 Instruction Information --------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the PROL16 implementation of the TargetInstrInfo class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_PROL16_PROL16INSTRINFO_H
#define LLVM_LIB_TARGET_PROL16_PROL16INSTRINFO_H

#include "llvm/CodeGen/TargetInstrInfo.h"

#include "PROL16RegisterInfo.h"

#define GET_INSTRINFO_HEADER
#include "PROL16GenInstrInfo.inc"

namespace llvm {

class PROL16InstrInfo final : public PROL16GenInstrInfo {
public:
	explicit PROL16InstrInfo();

	/// getRegisterInfo - TargetInstrInfo is a superset of MRegister info.  As
	/// such, whenever a client has an instance of instruction info, it should
	/// always be able to get register info as well (through this method).
	TargetRegisterInfo const& getRegisterInfo() const;

	/// Store the specified register of the given register class to the specified
	/// stack frame index. The store instruction is to be added to the given
	/// machine basic block before the specified machine instruction. If isKill
	/// is true, the register operand is the last use and must be marked kill.
	void storeRegToStackSlot(MachineBasicBlock &MBB, MachineBasicBlock::iterator MI,
							 unsigned SrcReg, bool isKill, int FrameIndex,
							 TargetRegisterClass const *RC, TargetRegisterInfo const *TRI) const override;

	/// Load the specified register of the given register class from the specified
	/// stack frame index. The load instruction is to be added to the given
	/// machine basic block before the specified machine instruction.
	void loadRegFromStackSlot(MachineBasicBlock &MBB, MachineBasicBlock::iterator MI,
							unsigned DestReg, int FrameIndex,
							TargetRegisterClass const *RC, TargetRegisterInfo const *TRI) const override;

	/// Emit instructions to copy a pair of physical registers.
	///
	/// This function should support copies within any legal register class as
	/// well as any cross-class copies created during instruction selection.
	///
	/// The source and destination registers may overlap, which may require a
	/// careful implementation when multiple copy instructions are required for
	/// large registers. See for example the ARM target.
	void copyPhysReg(MachineBasicBlock &MBB, MachineBasicBlock::iterator MI, const DebugLoc &DL,
					 unsigned DestReg, unsigned SrcReg, bool KillSrc) const override;

	/// This function is called for all pseudo instructions
	/// that remain after register allocation. Many pseudo instructions are
	/// created to help register allocation. This is the place to convert them
	/// into real instructions. The target can edit MI in place, or it can insert
	/// new instructions and erase MI. The function should return true if
	/// anything was changed.
	bool expandPostRAPseudo(MachineInstr &MI) const override;

	uint64_t getFramePoppedByCallee(MachineInstr const &machineInstruction) const;

private:
	PROL16RegisterInfo const registerInfo;

	void emitJumpCall(MachineInstr &machineInstruction) const;

	void emitJumpConditional(MachineInstr &machineInstruction) const;

	void emitImplicitDef(MachineInstr &machineInstruction) const;
};

}

#endif
