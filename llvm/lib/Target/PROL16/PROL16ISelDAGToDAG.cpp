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

#include "llvm/Pass.h"
#include "llvm/CodeGen/SelectionDAGISel.h"
#include "llvm/Support/Casting.h"

#include "PROL16.h"
#include "PROL16TargetMachine.h"
#include "PROL16ISelLowering.h"

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
	int64_t calcGlobalAddressOffset(int64_t const offset) const noexcept;

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

	switch (address.getOpcode()) {
	case ISD::FrameIndex:
		base = CurDAG->getTargetFrameIndex(cast<FrameIndexSDNode>(address)->getIndex(),
										   getTargetLowering()->getPointerTy(CurDAG->getDataLayout()));
		displacement = CurDAG->getTargetConstant(0, SDLoc(address), MVT::i16);
		return true;

	case ISD::OR:
	case ISD::ADD:
		base = CurDAG->getTargetFrameIndex(cast<FrameIndexSDNode>(address.getOperand(0))->getIndex(),
												   getTargetLowering()->getPointerTy(CurDAG->getDataLayout()));
		displacement = CurDAG->getTargetConstant(cast<ConstantSDNode>(address.getOperand(1))->getSExtValue(),
												 SDLoc(address), MVT::i16);
		return true;

	case ISD::GlobalAddress:
		base = CurDAG->getTargetGlobalAddress(cast<GlobalAddressSDNode>(address)->getGlobal(),
											  SDLoc(address),
											  getTargetLowering()->getPointerTy(CurDAG->getDataLayout()));

		displacement = CurDAG->getTargetConstant(calcGlobalAddressOffset(cast<GlobalAddressSDNode>(address)->getOffset()),
												 SDLoc(address), MVT::i16);

		return true;

	default:
		address.dump();
		errs() << "opcode: " << address.getOpcode() << '\n';
		llvm_unreachable("Unexpected opcode when selecting address (PROL16DAGToDAGISel::SelectAddressWithDisplacement())");
	}

	return false;
}

int64_t PROL16DAGToDAGISel::calcGlobalAddressOffset(int64_t const offset) const noexcept {
	// https://stackoverflow.com/questions/7594508/modulo-operator-with-negative-values
	return offset / 2 + offset % 2;
}

}

/// createPROL16ISelDag - This pass converts a legalized DAG into a
/// PROL16-specific DAG, ready for instruction scheduling.
FunctionPass* llvm::createPROL16ISelDag(PROL16TargetMachine &targetMachine) {
	return new PROL16DAGToDAGISel(targetMachine);
}
