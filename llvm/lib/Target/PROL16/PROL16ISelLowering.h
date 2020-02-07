//===-- PROL16ISelLowering.h - PROL16 DAG Lowering Interface ------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the interfaces that PROL16 uses to lower LLVM code into a
// selection DAG.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_PROL16_PROL16ISELLOWERING_H
#define LLVM_LIB_TARGET_PROL16_PROL16ISELLOWERING_H

#include "PROL16ConditionCodes.h"

#include "llvm/ADT/SmallVector.h"

#include "llvm/CodeGen/ISDOpcodes.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/SelectionDAG.h"
#include "llvm/CodeGen/SelectionDAGNodes.h"
#include "llvm/CodeGen/TargetLowering.h"

#include "llvm/IR/CallingConv.h"

#include "llvm/Target/TargetMachine.h"

namespace llvm {
class PROL16Subtarget;	// prevent circular include

namespace PROL16ISD {
enum NodeType : unsigned {
	FIRST_NUMBER = ISD::BUILTIN_OP_END,

	/// CALL - These operations represent an abstract call
	/// instruction, which includes a bunch of information.
	CALL,

	/// RETURN - Abstract return instruction
	RETURN,

	/// COMP - Compare instruction
	COMP,

	/// JUMPC - Jump if carry flag is set
	JUMPC,

	/// JUMPZ - Jump if zero flag is set
	JUMPZ,

	/// JUMPCZ - Jump if carry or zero flag is set
	JUMPCZ,

	/// JUMPNZ - Jump if zero flag is not set
	JUMPNZ,

	/// SELECT_CC - Abstract select based on condition code instruction
	SELECT_CC,
};
}

class PROL16TargetLowering final : public TargetLowering {
public:
	PROL16TargetLowering(TargetMachine const &targetMachine, PROL16Subtarget const &subtarget);

	/// This method returns the name of a target specific DAG node.
	char const* getTargetNodeName(unsigned Opcode) const override;

	// Lowering methods - These methods must be implemented by targets so that
	// the SelectionDAGBuilder code knows how to lower these.

	/// This callback is invoked for operations that are unsupported by the
	/// target, which are registered to use 'custom' lowering, and whose defined
	/// values are all legal.  If the target has no operations that require custom
	/// lowering, it need not implement this.  The default implementation of this
	/// aborts.
	SDValue LowerOperation(SDValue Op, SelectionDAG &DAG) const override;

	/// This callback is invoked when a node result type is illegal for the
	/// target, and the operation was registered to use 'custom' lowering for that
	/// result type.  The target places new result values for the node in Results
	/// (their number and types must exactly match those of the original return
	/// values of the node), or leaves Results empty, which indicates that the
	/// node is not to be custom lowered after all.
	///
	/// If the target has no operations that require custom lowering, it need not
	/// implement this.  The default implementation aborts.
/*
	void ReplaceNodeResults(SDNode *node,
							SmallVectorImpl<SDValue> &results,
							SelectionDAG &dag) const override;
*/

	/// This hook must be implemented to lower the incoming (formal) arguments,
	/// described by the inputArguments array, into the specified DAG. The implementation
	/// should fill in the inVals array with legal-type argument values, and
	/// return the resulting token chain value.
	SDValue LowerFormalArguments(SDValue chain, CallingConv::ID callingConvention, bool isVarArg,
								 SmallVectorImpl<ISD::InputArg> const &inputArguments, SDLoc const &debugLocation,
								 SelectionDAG &dag, SmallVectorImpl<SDValue> &inVals) const override;

	/// This hook must be implemented to lower calls into the specified
	/// DAG. The outgoing arguments to the call are described by the Outs array,
	/// and the values to be returned by the call are described by the Ins
	/// array. The implementation should fill in the inVals array with legal-type
	/// return values from the call, and return the resulting token chain value.
	SDValue	LowerCall(CallLoweringInfo &callLoweringInfo,
					  SmallVectorImpl<SDValue> &inVals) const override;

	/// This hook must be implemented to lower outgoing return values, described
	/// by the outs array, into the specified DAG. The implementation should
	/// return the resulting token chain value.
	SDValue LowerReturn(SDValue chain, CallingConv::ID callingConvention, bool isVarArg,
						SmallVectorImpl<ISD::OutputArg> const &outs, SmallVectorImpl<SDValue> const &outVals,
						SDLoc const &debugLocation, SelectionDAG &dag) const override;

	/// This method should be implemented by targets that mark instructions with
	/// the 'usesCustomInserter' flag.  These instructions are special in various
	/// ways, which require special support to insert.  The specified MachineInstr
	/// is created but not inserted into any basic blocks, and this method is
	/// called to expand it into a sequence of instructions, potentially also
	/// creating new basic blocks and control flow.
	/// As long as the returned basic block is different (i.e., we created a new
	/// one), the custom inserter is free to modify the rest of \p MBB.
	MachineBasicBlock* EmitInstrWithCustomInserter(MachineInstr &MI, MachineBasicBlock *MBB) const override;

private:
	/**
	 * Lower \p ISD::BR_CC to an appropriate jump instruction based on the given condition code.
	 * @param operation - The concrete \p ISD::BR_CC operation to lower.
	 * 					  The operands in order are chain, cc, lhs, rhs, block to branch to if condition is true.
	 * @param dag
	 * @return
	 */
	SDValue lowerConditionalBranch(SDValue operation, SelectionDAG &dag) const;

	/**
	 * Lower \p ISD::SELECT_CC appropriately.
	 * @param operation - The concrete \p ISD::SELECT_CC operation to lower.
	 * 					  The operands in order are lhs, rhs, true value, false value, condition code.
	 * @param dag
	 * @return
	 */
	SDValue lowerConditionalSelect(SDValue operation, SelectionDAG &dag) const;

	/**
	 * Lower \p ISD::SETCC appropriately.
	 * @param operation - The concrete \p ISD::SETCC operation to lower.
	 * 					  The operands in order are lhs, rhs, condition code.
	 * @param dag
	 * @return
	 */
	SDValue lowerConditionalSet(SDValue operation, SelectionDAG &dag) const;

	SDValue lowerConditionOperand(SDValue const &operation, unsigned const operandNumber, SelectionDAG &dag) const;

	SDValue emitCompare(SDValue &lhs, SDValue &rhs, PROL16CC::ConditionCode &targetConditionCode,
						ISD::CondCode const conditionCode,
						SDLoc const &debugLocation, SelectionDAG &dag) const;

	PROL16CC::ConditionCode evalTargetConditionCode(SDValue &lhs, SDValue &rhs, ISD::CondCode const conditionCode) const;

	/**
	 * Lower global address symbols such as string constants.
	 * @param operation
	 * @param dag
	 * @return
	 */
	SDValue lowerGlobalAddress(SDValue operation, SelectionDAG &dag) const;

	/**
	 * Lower the arguments of a C-style function call.
	 * @return
	 */
	SDValue lowerCArguments(SDValue chain, CallingConv::ID callingConvention, bool isVarArg,
							SmallVectorImpl<ISD::InputArg> const &inputArguments, SDLoc const &debugLocation,
							SelectionDAG &dag, SmallVectorImpl<SDValue> &inVals) const;

	/**
	 * Lower a C-style function call.
	 * @param callLoweringInfo
	 * @return
	 */
	SDValue lowerCCall(CallLoweringInfo &callLoweringInfo, SmallVectorImpl<SDValue> &inVals) const;

	/**
	 * Lower the result of a function call.
	 * @param callLoweringInfo
	 * @return
	 */
	SDValue lowerCallResult(SDValue chain, CallLoweringInfo &callLoweringInfo,
							SmallVectorImpl<SDValue> &inVals) const;

	MachineBasicBlock* emitStoreWithDisplacement(MachineInstr &machineInstruction,
												MachineBasicBlock *machineBasicBlock) const;

	MachineBasicBlock* emitLoadWithDisplacement(MachineInstr &machineInstruction,
												MachineBasicBlock *machineBasicBlock) const;

	MachineBasicBlock* emitCallImmediate(MachineInstr &machineInstruction,
										 MachineBasicBlock *machineBasicBlock) const;

	MachineBasicBlock* emitCallRegister(MachineInstr &machineInstruction,
										MachineBasicBlock *machineBasicBlock) const;

	MachineBasicBlock* emitReturn(MachineInstr &machineInstruction,
								  MachineBasicBlock *machineBasicBlock) const;

	MachineBasicBlock* emitDirectBranch(MachineInstr &machineInstruction,
										MachineBasicBlock *machineBasicBlock) const;

	MachineBasicBlock* emitConditionalBranch(MachineInstr &machineInstruction,
											 MachineBasicBlock *machineBasicBlock) const;

	MachineBasicBlock* emitJumpcz(MachineInstr &machineInstruction,
								  MachineBasicBlock *machineBasicBlock) const;

	MachineBasicBlock* emitJumpnz(MachineInstr &machineInstruction,
								  MachineBasicBlock *machineBasicBlock) const;

	MachineBasicBlock* emitShiftRightArithmetical(MachineInstr &machineInstruction,
												  MachineBasicBlock *machineBasicBlock) const;

	MachineBasicBlock* emitShiftWithImmediateCount(MachineInstr &machineInstruction,
												   MachineBasicBlock *machineBasicBlock) const;

	MachineBasicBlock* emitShiftWithRegisterCount(MachineInstr &machineInstruction,
												  MachineBasicBlock *machineBasicBlock) const;

	MachineBasicBlock* emitSelect(MachineInstr &machineInstruction,
								  MachineBasicBlock *machineBasicBlock) const;
};

}

#endif
