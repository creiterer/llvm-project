//===-- PROL16ISelDAGToDAG.cpp - A dag to dag instruction selector for PROL16 ------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines an instruction selector for the PROL16 target.
//
//===----------------------------------------------------------------------===//

#include "PROL16.h"
#include "PROL16ISelLowering.h"
#include "PROL16TargetMachine.h"
#include "PROL16Utils.h"

#include "llvm/Pass.h"
#include "llvm/CodeGen/SelectionDAGISel.h"
#include "llvm/Support/Casting.h"

using namespace llvm;

#define DEBUG_TYPE "prol16"

// Defines symbolic names for the PROL16 instructions.
#define GET_INSTRINFO_ENUM
#include "PROL16GenInstrInfo.inc"

//===----------------------------------------------------------------------===//
// Instruction Selector Implementation
//===----------------------------------------------------------------------===//

/// PROL16DAGToDAGISel - PROL16 specific code to select PROL16 machine
/// instructions for SelectionDAG operations.
namespace {

// Insert a node into the DAG at least before the Pos node's position. This
// will reposition the node as needed, and will assign it a node ID that is <=
// the Pos node's ID. Note that this does *not* preserve the uniqueness of node
// IDs! The selection DAG must no longer depend on their uniqueness when this
// is used.
static void insertDAGNode(SelectionDAG &DAG, SDValue Pos, SDValue N) {
	if (N->getNodeId() == -1 ||
			(SelectionDAGISel::getUninvalidatedNodeId(N.getNode()) > SelectionDAGISel::getUninvalidatedNodeId(Pos.getNode()))) {
		DAG.RepositionNode(Pos->getIterator(), N.getNode());
		// Mark Node as invalid for pruning as after this it may be a successor to a
		// selected node but otherwise be in the same position of Pos.
		// Conservatively mark it with the same -abs(Id) to assure node id
		// invariant is preserved.
		N->setNodeId(Pos->getNodeId());
		SelectionDAGISel::InvalidateNodeId(N.getNode());
	}
}

class PROL16DAGToDAGISel final : public SelectionDAGISel {
public:
	explicit PROL16DAGToDAGISel(PROL16TargetMachine &targetMachine);

	/// Main hook for targets to transform nodes into machine nodes.
	void Select(SDNode *N) override;

	/**
	 * Complex Pattern Selector for an address.
	 * @details Method, corresponding to the complex pattern definition of 'addr' in PROL16InstrInfo.td
	 * @param address - The address to be selected.
	 * @param destination - The destination of the selected address.
	 * @return
	 */
	bool SelectAddress(SDValue const &address, SDValue &destination);

	/**
	 * Complex Pattern Selector for an address with a displacement
	 * @details Method, corresponding to the complex pattern definition of 'addrdisp' in PROL16InstrInfo.td
	 * @param address - The address to be selected.
	 * @param base - The destination of the selected base address.
	 * @param displacement - The destination of the selected displacement.
	 * @return
	 */
	bool SelectAddressWithDisplacement(SDValue const &address, SDValue &base, SDValue &displacement);

	/// getPassName - Return a nice clean name for a pass.  This usually
	/// implemented in terms of the name that is registered by one of the
	/// Registration templates, but can be overloaded directly.
	StringRef getPassName() const override;

private:
	void replaceDisplacementOperand(SDNode *N, SDValue const &originalDisplacement, SDValue const &newDisplacement);
	SDValue selectAddressFor16BitBytes(SDValue const &instruction) const;
	SDValue selectShlForAddressDisplacement(SDValue const &instruction) const;

public:
// include the pieces auto-generated from the target description
#include "PROL16GenDAGISel.inc"
};

PROL16DAGToDAGISel::PROL16DAGToDAGISel(PROL16TargetMachine &targetMachine) : SelectionDAGISel(targetMachine) {}

StringRef PROL16DAGToDAGISel::getPassName() const {
	return "PROL16 DAG->DAG Pattern Instruction Selection";
}

void PROL16DAGToDAGISel::Select(SDNode *N) {
	LLVM_DEBUG(dbgs() << "PROL16DAGToDAGISel::Select()\n");
	LLVM_DEBUG(N->dump());

	// if node is already a machine node (e.g. inserted by lower global address)
	// do not try to select it
	if (N->isMachineOpcode()) {
		N->setNodeId(-1);
		return;
	}

	SDLoc debugLocation(N);

	// custom selection stuff
	switch (N->getOpcode()) {
	case ISD::GlobalAddress: {
		GlobalAddressSDNode const * const globalAddressNode = cast<GlobalAddressSDNode>(N);
		GlobalValue const * const globalValue = globalAddressNode->getGlobal();
		int64_t const offset = globalAddressNode->getOffset();
		assert(offset == 0);
		auto pointerValueType = getTargetLowering()->getPointerTy(CurDAG->getDataLayout());

		SDValue targetGlobalAddress = CurDAG->getTargetGlobalAddress(globalValue, debugLocation, pointerValueType);

		CurDAG->SelectNodeTo(N, PROL16::LOADI, MVT::i16, targetGlobalAddress);

		return;
	}

	case ISD::FrameIndex:
		CurDAG->SelectNodeTo(N, PROL16::ADDframe, MVT::i16,
							 CurDAG->getTargetFrameIndex(cast<FrameIndexSDNode>(N)->getIndex(), MVT::i16),
							 CurDAG->getTargetConstant(0, debugLocation, MVT::i16));
		return;

	case ISD::ADD:
		N->getOperand(0).dump();
		N->getOperand(1).dump();

		if (auto memoryNode = dyn_cast<MemSDNode>(N->getOperand(0))) {
			/**
			 * This is a hack to know if the loaded value is itself an address,
			 * which in turn means that the second operand of the 'add' represents an offset
			 * that needs to be adapted appropriately.
			 */
			if (!memoryNode->getMemOperand()->getValue()->getName().endswith(".addr")) {
				break;
			}

			SDValue originalDisplacement = N->getOperand(1);
			SDValue newDisplacement;

			if (auto constantNode = dyn_cast<ConstantSDNode>(originalDisplacement)) {
				newDisplacement = CurDAG->getConstant(prol16::util::calcOffset(constantNode->getSExtValue()), debugLocation, MVT::i16);
			} else if (originalDisplacement.getOpcode() == ISD::SHL) {
				newDisplacement = selectShlForAddressDisplacement(originalDisplacement);
			} else {
				break;
			}

			replaceDisplacementOperand(N, originalDisplacement, newDisplacement);
		} else if (auto constantNode = dyn_cast<ConstantSDNode>(N->getOperand(1))) {
			/**
			 * If a constant is used as operand 1 of an add instruction, assume that this constant represents an offset.
			 * Since this value is not visible to PROL16DAGToDAGISel::SelectAddressWithDisplacement(), and in further consequence to
			 * PROL16TargetLowering::emitLoadWithDisplacement() and PROL16TargetLowering::emitStoreWithDisplacement(), we have to halve
			 * the offset value here.
			 *
			 * For example:
			 * The 'store' instruction in the following selection DAG uses node 't9' as base address. The node 't9' is
			 * a copy from register 'Register:i16 %2', which is defined in 'bb.8'.
			 * Optimized legalized selection DAG: %bb.9 'main.deleteNode.pN9_main.Node:'
			 * SelectionDAG has 19 nodes:
			 *   t0: ch = EntryToken
			 *       t2: i16,ch = CopyFromReg t0, Register:i16 %0
			 *     t4: i16 = add nuw t2, Constant:i16<2>
			 *   t7: i16,ch = load<(load 2 from %ir.27)> t0, t4, undef:i16
			 *         t9: i16,ch = CopyFromReg t0, Register:i16 %2
			 *       t10: ch = store<(store 2 into %ir.29)> t7:1, t7, t9, undef:i16
			 *     t13: ch,glue = callseq_start t10, TargetConstant:i16<0>, TargetConstant:i16<0>
			 *   t15: ch,glue = CopyToReg t13, Register:i16 $r4, undef:i16
			 *   t17: ch,glue = PROL16ISD::CALL t15, TargetGlobalAddress:i16<void (i8*)* @main.debugGuardEnd> 0, Register:i16 $r4, t15:1
			 *     t18: ch,glue = callseq_end t17, TargetConstant:i16<0>, TargetConstant:i16<0>, t17:1
			 *   t20: ch = br t18, BasicBlock:ch<.2.if.done 0x49eea80>
			 *
			 * The value of 'Register:i16 %2' is the value of node 't4', which is an add of node 't2' with a constant 'Constant:i16<2>'.
			 * The value of 't2' is a copy from register 'Register:i16 %1', which is defined in 'bb.5'.
			 * Optimized legalized selection DAG: %bb.8 'main.deleteNode.pN9_main.Node:'
			 * SelectionDAG has 15 nodes:
			 *   t0: ch = EntryToken
			 *       t8: i16,ch = CopyFromReg t0, Register:i16 %0
			 *           t2: i16,ch = CopyFromReg t0, Register:i16 %1
			 *         t4: i16 = add nuw t2, Constant:i16<2>			<-- 'Constant:i16<2>' is used as an offset!
			 *       t6: ch = CopyToReg t0, Register:i16 %2, t4
			 *     t18: ch = JUMPcc BasicBlock:ch< 0x49eecd8>, t8, Constant:i16<0>, TargetConstant:i16<0>, t6
			 *   t15: ch = br t18, BasicBlock:ch< 0x49eef30>
			 *
			 * The basic block 'bb.5' defines the value of 'Register:i16 %1' as the value of node 't18'. The value of 't18' is
			 * a copy from register 'r4' which holds the return value of the call to 'main.predecessor.pN9_main.Node'.
			 * As the signature 'i8* (i8*, i8*, i8*)' indicates, the return value is a pointer, that is, an address.
			 * That means, that the constant 'Constant:i16<2>' in the 'add' above is added to an address value and therefore used as an offset!
			 * Optimized legalized selection DAG: %bb.5 'main.deleteNode.pN9_main.Node:.4.if.done'
			 * SelectionDAG has 32 nodes:
			 *   t0: ch = EntryToken
			 *     t8: ch,glue = callseq_start t0, TargetConstant:i16<0>, TargetConstant:i16<0>
			 *   t10: ch,glue = CopyToReg t8, Register:i16 $r4, undef:i16
			 *     t4: i16,ch = CopyFromReg t0, Register:i16 %4
			 *   t12: ch,glue = CopyToReg t10, Register:i16 $r5, t4, t10:1
			 *     t6: i16,ch = CopyFromReg t0, Register:i16 %0
			 *   t14: ch,glue = CopyToReg t12, Register:i16 $r6, t6, t12:1
			 *   t16: ch,glue = PROL16ISD::CALL t14, TargetGlobalAddress:i16<i8* (i8*, i8*, i8*)* @main.predecessor.pN9_main.Node> 0, Register:i16 $r4, Register:i16 $r5, Register:i16 $r6, t14:1
			 *   t17: ch,glue = callseq_end t16, TargetConstant:i16<0>, TargetConstant:i16<0>, t16:1
			 *   t18: i16,ch,glue = CopyFromReg t17, Register:i16 $r4, t17:1
			 *     t22: ch,glue = callseq_start t18:1, TargetConstant:i16<0>, TargetConstant:i16<0>
			 *   t23: ch,glue = CopyToReg t22, Register:i16 $r4, undef:i16
			 *   t25: ch,glue = PROL16ISD::CALL t23, TargetGlobalAddress:i16<void (i8*)* @main.debugGuardBegin> 0, Register:i16 $r4, t23:1
			 *         t20: ch = CopyToReg t0, Register:i16 %1, t18
			 *         t26: ch,glue = callseq_end t25, TargetConstant:i16<0>, TargetConstant:i16<0>, t25:1
			 *       t32: ch = TokenFactor t20, t26
			 *     t42: ch = JUMPcc BasicBlock:ch< 0x49eee68>, t18, Constant:i16<0>, TargetConstant:i16<1>, t32
			 *   t36: ch = br t42, BasicBlock:ch< 0x49eecd8>
			 *
			 * The example above shows, that, at the time we encounter the 'add' instruction in 'bb.8', there is no easy way to determine
			 * if the constant is used as an offset (and thus has to be halved) because:
			 * 		- we do not know that the resulting value is used later on as a base address operand of a load/store instruction,
			 * 		- we do not know that the non-constant operand is an address.
			 *
			 * Therefore, for now we assume that constant operands of an 'add' instruction represent an offset. Of course, this is not always true
			 * as the following example shows:
			 *   ...
			 *   t8: ch,glue = PROL16ISD::CALL t6, TargetGlobalAddress:i16<i16 (i8*)* @main.getSomeValue> 0, Register:i16 $r4, t6:1
			 *   ...
			 *   t10: i16,ch,glue = CopyFromReg t9, Register:i16 $r4, t9:1
			 *   ...
			 *   t12: i16 = add t10, Constant:i16<11>
			 *   ...
			 *
			 * Here, the constant 'Constant:i16<11>' is added to the node 't10', which in turn is a copy from register 'r4'. The value of 'r4' is
			 * the return value of the call to 'main.getSomeValue', which returns some other i16 value (i.e. no address). Therefore, halving the constant
			 * is wrong in this case.
			 */
			SDValue const originalDisplacement = N->getOperand(1);
			SDValue const newDisplacement = CurDAG->getConstant(prol16::util::calcOffset(constantNode->getSExtValue()), debugLocation, MVT::i16);

			replaceDisplacementOperand(N, originalDisplacement, newDisplacement);
		}

		break;
	}

	// delegate to auto-generated instruction selection
	SelectCode(N);
}

bool PROL16DAGToDAGISel::SelectAddress(SDValue const &address, SDValue &destination) {
	LLVM_DEBUG(dbgs() << "PROL16DAGToDAGISel::SelectAddress()\n");
	LLVM_DEBUG(address.dump());

	switch (address.getOpcode()) {
	case ISD::FrameIndex:
		destination = CurDAG->getTargetFrameIndex(cast<FrameIndexSDNode>(address)->getIndex(),
										   getTargetLowering()->getPointerTy(CurDAG->getDataLayout()));
		return true;

	default:
		address.dump();
		llvm_unreachable("Unexpected opcode when selecting address (PROL16DAGToDAGISel::SelectAddress())");
	}

	return false;
}

bool PROL16DAGToDAGISel::SelectAddressWithDisplacement(SDValue const &address, SDValue &base, SDValue &displacement) {
	LLVM_DEBUG(dbgs() << "PROL16DAGToDAGISel::SelectAddressWithDisplacement()\n");
	LLVM_DEBUG(address.dump());

	for (unsigned i = 0; i < address.getNumOperands(); ++i) {
		LLVM_DEBUG(dbgs() << "address operand " << i << ": ");
		LLVM_DEBUG(address.getOperand(i).dump());
	}

	switch (address.getOpcode()) {
	case ISD::FrameIndex:
		base = CurDAG->getTargetFrameIndex(cast<FrameIndexSDNode>(address)->getIndex(),
										   getTargetLowering()->getPointerTy(CurDAG->getDataLayout()));
		displacement = CurDAG->getTargetConstant(0, SDLoc(address), MVT::i16);
		return true;

	case ISD::OR:
	case ISD::ADD: {
		SDValue const value = selectAddressFor16BitBytes(address);
		base = value.getOperand(0);
		displacement = value.getOperand(1);

		return true;
	}

	case ISD::GlobalAddress:
		base = CurDAG->getTargetGlobalAddress(cast<GlobalAddressSDNode>(address)->getGlobal(),
											  SDLoc(address),
											  getTargetLowering()->getPointerTy(CurDAG->getDataLayout()));

		displacement = CurDAG->getTargetConstant(cast<GlobalAddressSDNode>(address)->getOffset(), SDLoc(address), MVT::i16);

		return true;

	default:
		LLVM_DEBUG(dbgs() << "Using default address selection---generate base as a register and displacement as 0\n");

		base = address;
		displacement = CurDAG->getTargetConstant(0, SDLoc(address), MVT::i16);

		return true;
	}

	return false;
}

void PROL16DAGToDAGISel::replaceDisplacementOperand(SDNode *N, SDValue const &originalDisplacement, SDValue const &newDisplacement) {
	// Place in a correct topological ordering.
	insertDAGNode(*CurDAG, originalDisplacement, newDisplacement);

	SDNode *updatedNode = CurDAG->UpdateNodeOperands(N, N->getOperand(0), newDisplacement);

	if (updatedNode != N) {
		// If we found an existing node, we should replace ourselves with that node
		// and wait for it to be selected after its other users.
		ReplaceNode(N, updatedNode);
		return;
	}

	// If the original shift amount is now dead, delete it so that we don't run
	// it through isel.
	if (originalDisplacement.getNode()->use_empty()) {
		CurDAG->RemoveDeadNode(originalDisplacement.getNode());
	}
}

SDValue PROL16DAGToDAGISel::selectAddressFor16BitBytes(SDValue const &instruction) const {
	LLVM_DEBUG(dbgs() << "selected ");
	LLVM_DEBUG(instruction.dump());

	SDValue selectedValue;

	switch (instruction.getOpcode()) {
	case ISD::FrameIndex:
		selectedValue = CurDAG->getTargetFrameIndex(cast<FrameIndexSDNode>(instruction)->getIndex(),
													getTargetLowering()->getPointerTy(CurDAG->getDataLayout()));
		break;

	case ISD::Constant:
		selectedValue =  CurDAG->getTargetConstant(cast<ConstantSDNode>(instruction)->getSExtValue(), SDLoc(instruction), MVT::i16);
		break;

	case ISD::SHL: {
		selectedValue = selectShlForAddressDisplacement(instruction);
		break;
	}

	case ISD::OR:
	case ISD::ADD: {
		unsigned opcode = 0;
		SDValue const selectedOperand0 = selectAddressFor16BitBytes(instruction.getOperand(0));
		SDValue const selectedOperand1 = selectAddressFor16BitBytes(instruction.getOperand(1));

		if (selectedOperand0.getOpcode() == ISD::TargetFrameIndex) {
			opcode = PROL16::ADDframe;
		} else if (instruction.getOpcode() == ISD::OR) {
			opcode = PROL16::OR;
		} else {
			opcode = PROL16::ADD;
		}

		selectedValue = SDValue(CurDAG->getMachineNode(opcode, SDLoc(instruction), MVT::i16,
													   selectedOperand0,
													   selectedOperand1),
								0);
		break;
	}

	default:
		selectedValue = instruction;
	}

	LLVM_DEBUG(dbgs() << "to ");
	LLVM_DEBUG(selectedValue.dump());

	return selectedValue;
}

SDValue PROL16DAGToDAGISel::selectShlForAddressDisplacement(SDValue const &instruction) const {
	/**
	 * If a shl (shift left) instruction is used for offset calculation (i.e. the result of the shl instruction
	 * is used as offset), we need to halve the result (i.e. offset) due to our 16-bit bytes. The simplest way to do this
	 * is to decrease the shift amount by 1, which equals a division by 2.
	 */
	auto shiftAmountNode = cast<ConstantSDNode>(instruction.getOperand(1));
	uint64_t const shiftAmount = shiftAmountNode->getZExtValue();

	if (shiftAmount == 0 || shiftAmount == 1) {
		return instruction.getOperand(0);
	}

	return SDValue(CurDAG->getMachineNode(PROL16::SHLi, SDLoc(instruction), MVT::i16,
										  instruction.getOperand(0),
										  CurDAG->getTargetConstant(shiftAmount - 1, SDLoc(instruction), MVT::i16)),
				   0);
}

}

/// createPROL16ISelDag - This pass converts a legalized DAG into a
/// PROL16-specific DAG, ready for instruction scheduling.
FunctionPass* llvm::createPROL16ISelDag(PROL16TargetMachine &targetMachine) {
	return new PROL16DAGToDAGISel(targetMachine);
}
