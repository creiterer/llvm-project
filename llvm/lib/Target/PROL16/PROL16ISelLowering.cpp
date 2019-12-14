//===-- PROL16ISelLowering.cpp - PROL16 DAG Lowering Implementation ---------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the interfaces that PROL16 uses to lower LLVM code into a
// selection DAG.
//
//===----------------------------------------------------------------------===//

#include "PROL16ISelLowering.h"

#include "PROL16Subtarget.h"
#include "PROL16RegisterInfo.h"

#include "llvm/ADT/ArrayRef.h"

#include "llvm/CodeGen/CallingConvLower.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineOperand.h"

#include "llvm/IR/GlobalValue.h"

#include "llvm/Support/ErrorHandling.h"

#include <utility>

using namespace llvm;

#define DEBUG_TYPE "prol16"

// Defines symbolic names for the PROL16 instructions.
#define GET_INSTRINFO_ENUM
#include "PROL16GenInstrInfo.inc"

//===----------------------------------------------------------------------===//
// Calling Convention Implementation
//===----------------------------------------------------------------------===//

#define GET_REGINFO_ENUM
#include "PROL16GenRegisterInfo.inc"
#include "PROL16GenCallingConv.inc"

//===----------------------------------------------------------------------===//

PROL16TargetLowering::PROL16TargetLowering(TargetMachine const &targetMachine, PROL16Subtarget const &subtarget)
: TargetLowering(targetMachine) {
	// set up the register classes
	addRegisterClass(MVT::i16, &PROL16::GR16RegClass);

	// compute derived properties from the register classes
	// e.g. how many registers are needed for a certain type
	// (the results are used i.a. by TargetLoweringBase::getNumRegisters().)
	computeRegisterProperties(subtarget.getRegisterInfo());

	setStackPointerRegisterToSaveRestore(PROL16::RSP);

	setBooleanContents(ZeroOrOneBooleanContent);

	/* TODO(PROL16):
	 * You should examine the node types in the ISD namespace (include/llvm/CodeGen/SelectionDAGNodes.h)
	 * and determine which operations the target natively supports. For operations that do not have native support,
	 * add a callback to the constructor for the XXXTargetLowering class, so the instruction selection process knows what to do.
	 * The TargetLowering class callback methods (declared in llvm/Target/TargetLowering.h)
	 */

//	setTruncStoreAction(MVT::i16, MVT::i8, Expand);

	setOperationAction(ISD::BR_CC, MVT::i8, Custom);
	setOperationAction(ISD::BR_CC, MVT::i16, Custom);
	setOperationAction(ISD::BRCOND, MVT::Other, Expand);
	setOperationAction(ISD::SELECT_CC, MVT::i8, Custom);
	setOperationAction(ISD::SELECT_CC, MVT::i16, Custom);
	setOperationAction(ISD::SETCC, MVT::i8, Custom);
	setOperationAction(ISD::SETCC, MVT::i16, Custom);

	setOperationAction(ISD::GlobalAddress, MVT::i16, Custom);

	for (MVT const valueType : MVT::integer_valuetypes()) {
		setOperationAction(ISD::MUL, valueType, Expand);
		setOperationAction(ISD::MULHS, valueType, Expand);
		setOperationAction(ISD::MULHU, valueType, Expand);
		setOperationAction(ISD::SMUL_LOHI, valueType, Expand);
		setOperationAction(ISD::UMUL_LOHI, valueType, Expand);

		setOperationAction(ISD::UDIV, valueType, Expand);
		setOperationAction(ISD::UREM, valueType, Expand);
		setOperationAction(ISD::UDIVREM, valueType, Expand);
		setOperationAction(ISD::SDIV, valueType, Expand);
		setOperationAction(ISD::SREM, valueType, Expand);
		setOperationAction(ISD::SDIVREM, valueType, Expand);
	}

	setOperationAction(ISD::MUL, MVT::i16, LibCall);

	setOperationAction(ISD::UDIV, MVT::i16, LibCall);
	setOperationAction(ISD::UREM, MVT::i16, LibCall);
	setOperationAction(ISD::SDIV, MVT::i16, LibCall);
	setOperationAction(ISD::SREM, MVT::i16, LibCall);

	setLibcallName(RTLIB::MUL_I16, "__prol16_mul_i16");
	setLibcallName(RTLIB::UDIV_I16, "__prol16_udiv_i16");
	setLibcallName(RTLIB::SDIV_I16, "__prol16_sdiv_i16");
	setLibcallName(RTLIB::UREM_I16, "__prol16_urem_i16");
	setLibcallName(RTLIB::SREM_I16, "__prol16_srem_i16");

	setLibcallName(RTLIB::MUL_I32, "__prol16_mul_i32");
	setLibcallName(RTLIB::UDIV_I32, "__prol16_udiv_i32");
	setLibcallName(RTLIB::SDIV_I32, "__prol16_sdiv_i32");
	setLibcallName(RTLIB::UREM_I32, "__prol16_urem_i32");
	setLibcallName(RTLIB::SREM_I32, "__prol16_srem_i32");
}

char const* PROL16TargetLowering::getTargetNodeName(unsigned Opcode) const {
	switch (static_cast<PROL16ISD::NodeType>(Opcode)) {
	case PROL16ISD::FIRST_NUMBER:	break;
	case PROL16ISD::CALL: 			return "PROL16ISD::CALL";
	case PROL16ISD::RETURN:			return "PROL16ISD::RETURN";
	case PROL16ISD::COMP:			return "PROL16ISD::COMP";
	case PROL16ISD::JUMPC:			return "PROL16ISD::JUMPC";
	case PROL16ISD::JUMPZ: 			return "PROL16ISD::JUMPZ";
	case PROL16ISD::JUMPCZ:			return "PROL16ISD::JUMPCZ";
	case PROL16ISD::JUMPNZ:			return "PROL16ISD::JUMPNZ";
	case PROL16ISD::SELECT_CC:		return "PROL16ISD::SELECT_CC";
	}

	llvm_unreachable("Encountered invalid PROL16 node opcode while getting target node name!");
}

SDValue PROL16TargetLowering::LowerOperation(SDValue Op, SelectionDAG &DAG) const {
	switch (Op.getOpcode()) {
	case ISD::BR_CC:
		return lowerConditionalBranch(Op, DAG);
	case ISD::SELECT_CC:
		return lowerConditionalSelect(Op, DAG);
	case ISD::SETCC:
		return lowerConditionalSet(Op, DAG);
	case ISD::GlobalAddress:
		return lowerGlobalAddress(Op, DAG);
	default:
		report_fatal_error("Unsupported operation lowering");
	}
}

SDValue PROL16TargetLowering::LowerFormalArguments(SDValue chain, CallingConv::ID callingConvention, bool isVarArg,
												   SmallVectorImpl<ISD::InputArg> const &inputArguments, SDLoc const &debugLocation,
												   SelectionDAG &dag, SmallVectorImpl<SDValue> &inVals) const {
	switch (callingConvention) {
	case CallingConv::C:
	case CallingConv::Fast:
		return lowerCArguments(chain, callingConvention, isVarArg, inputArguments, debugLocation, dag, inVals);
	default:
		report_fatal_error("Unsupported calling convention");
	}
}

SDValue	PROL16TargetLowering::LowerCall(CallLoweringInfo &callLoweringInfo,
										SmallVectorImpl<SDValue> &inVals) const {
	// PROL16 does not support tail call optimization
	callLoweringInfo.IsTailCall = false;

	switch (callLoweringInfo.CallConv) {
	case CallingConv::C:
	case CallingConv::Fast:
		return lowerCCall(callLoweringInfo, inVals);
	default:
		report_fatal_error("Unsupported calling convention");
	}
}

SDValue PROL16TargetLowering::LowerReturn(SDValue chain, CallingConv::ID callingConvention, bool isVarArg,
										  SmallVectorImpl<ISD::OutputArg> const &outs, SmallVectorImpl<SDValue> const &outVals,
										  SDLoc const &debugLocation, SelectionDAG &dag) const {
	LLVM_DEBUG(dbgs() << "DAG at beginning of PROL16TargetLowering::LowerReturn():\n");
	LLVM_DEBUG(dag.dump());

	// CCValAssign - represent the assignment of the return value to locations.
	SmallVector<CCValAssign, 16> returnValueLocations;

	// CCState - info about the registers and stack slot.
	CCState ccState(callingConvention, isVarArg, dag.getMachineFunction(), returnValueLocations, *dag.getContext());

	// Analyze return values.
	ccState.AnalyzeReturn(outs, RetCC_PROL16);

	SmallVector<SDValue, 4> returnOperands(1, chain);

	SDValue flag;

	// Copy the result values into the output registers.
	for (unsigned i = 0; i != returnValueLocations.size(); ++i) {
	    CCValAssign &ccValAssign = returnValueLocations[i];
	    assert(ccValAssign.isRegLoc() && "Can only return in registers!");

	    chain = dag.getCopyToReg(chain, debugLocation, ccValAssign.getLocReg(), outVals[i], flag);

	    // Guarantee that all emitted copies are stuck together,
	    // avoiding something bad.
	    flag = chain.getValue(1);
	    returnOperands.push_back(dag.getRegister(ccValAssign.getLocReg(), ccValAssign.getLocVT()));
	}

	returnOperands[0] = chain;	// update chain

	// add the flag if we have it
	if (flag.getNode() != nullptr) {
		returnOperands.push_back(flag);
	}

	SDValue node = dag.getNode(PROL16ISD::RETURN, debugLocation, MVT::Other, returnOperands);

	LLVM_DEBUG(dbgs() << "DAG at end of PROL16TargetLowering::LowerReturn():\n");
	LLVM_DEBUG(dag.dump());

	return node;
}

MachineBasicBlock* PROL16TargetLowering::EmitInstrWithCustomInserter(MachineInstr &MI, MachineBasicBlock *MBB) const {
	switch (MI.getOpcode()) {
	case PROL16::STOREim:
	case PROL16::STORErm:
		return emitStoreWithDisplacement(MI, MBB);
	case PROL16::LOADrm:
		return emitLoadWithDisplacement(MI, MBB);
	case PROL16::CALLi:
		return emitCallImmediate(MI, MBB);
	case PROL16::RETURN:
		return emitReturn(MI, MBB);
	case PROL16::BR:
		return emitDirectBranch(MI, MBB);
	case PROL16::JUMPZ:
	case PROL16::JUMPC:
		return emitConditionalBranch(MI, MBB);
	case PROL16::JUMPCZ:
		return emitJumpcz(MI, MBB);
	case PROL16::JUMPNZ:
		return emitJumpnz(MI, MBB);
	case PROL16::SRA:
		return emitShiftRightArithmetical(MI, MBB);
	case PROL16::SHLi:
	case PROL16::SHRi:
	case PROL16::SRAi:
		return emitShiftWithImmediateCount(MI, MBB);
	case PROL16::SHLr:
	case PROL16::SHRr:
	case PROL16::SRAr:
		return emitShiftWithRegisterCount(MI, MBB);
	case PROL16::SELECT:
		return emitSelect(MI, MBB);
	default:
		MI.print(errs());
		llvm_unreachable("Unexpected instruction type for custom insertion!");
	}

	return nullptr;
}

SDValue PROL16TargetLowering::lowerConditionalBranch(SDValue operation, SelectionDAG &dag) const {
	SDValue chain = operation.getOperand(0);
	ISD::CondCode conditionCode = cast<CondCodeSDNode>(operation.getOperand(1))->get();
	SDValue lhs = operation.getOperand(2);
	SDValue rhs = operation.getOperand(3);
	SDValue destinationLabel = operation.getOperand(4);
	SDLoc debugLocation(operation);

	LLVM_DEBUG(dbgs() << "PROL16TargetLowering::lowerConditionalBranch():\n");
	LLVM_DEBUG(operation.dump());

	PROL16CC::ConditionCode targetConditionCode = PROL16CC::INVALID;

	SDValue flag = emitCompare(lhs, rhs, targetConditionCode, conditionCode, debugLocation, dag);

	switch (targetConditionCode) {
	case PROL16CC::EQ:
		return dag.getNode(PROL16ISD::JUMPZ, debugLocation, operation.getValueType(),
						   chain, destinationLabel, flag);
	case PROL16CC::NE:
		return dag.getNode(PROL16ISD::JUMPNZ, debugLocation, operation.getValueType(),
						   chain, destinationLabel, flag);
	case PROL16CC::LT:
		return dag.getNode(PROL16ISD::JUMPC, debugLocation, operation.getValueType(),
						   chain, destinationLabel, flag);
	case PROL16CC::LE:
		return dag.getNode(PROL16ISD::JUMPCZ, debugLocation, operation.getValueType(),
						   chain, destinationLabel, flag);
	default:
		llvm_unreachable("Invalid PROL16 condition code");
	}
}

SDValue PROL16TargetLowering::lowerConditionalSelect(SDValue operation, SelectionDAG &dag) const {
	SDValue lhs = operation.getOperand(0);
	SDValue rhs = operation.getOperand(1);
	SDValue trueValue = operation.getOperand(2);
	SDValue falseValue = operation.getOperand(3);
	ISD::CondCode conditionCode = cast<CondCodeSDNode>(operation.getOperand(4))->get();
	SDLoc debugLocation(operation);

	PROL16CC::ConditionCode const targetConditionCode = evalTargetConditionCode(lhs, rhs, conditionCode);
	SDValue targetConditionCodeValue = dag.getConstant(targetConditionCode, debugLocation, MVT::i16);

	SDValue operands[] = {lhs, rhs, trueValue, falseValue, targetConditionCodeValue};

	return dag.getNode(PROL16ISD::SELECT_CC, debugLocation, operation.getValueType(), operands);
}

SDValue PROL16TargetLowering::lowerConditionalSet(SDValue operation, SelectionDAG &dag) const {
	SDValue lhs = operation.getOperand(0);
	SDValue rhs = operation.getOperand(1);
	ISD::CondCode conditionCode = cast<CondCodeSDNode>(operation.getOperand(2))->get();
	SDLoc debugLocation(operation);

	EVT valueType = operation.getValueType();
	SDValue one = dag.getConstant(1, debugLocation, valueType);
	SDValue zero = dag.getConstant(0, debugLocation, valueType);

	PROL16CC::ConditionCode const targetConditionCode = evalTargetConditionCode(lhs, rhs, conditionCode);
	SDValue targetConditionCodeValue = dag.getConstant(targetConditionCode, debugLocation, MVT::i16);

	SDValue operands[] = {lhs, rhs, one, zero, targetConditionCodeValue};

	return dag.getNode(PROL16ISD::SELECT_CC, debugLocation, valueType, operands);
}

SDValue PROL16TargetLowering::emitCompare(SDValue &lhs, SDValue &rhs, PROL16CC::ConditionCode &targetConditionCode,
										  ISD::CondCode const conditionCode,
										  SDLoc const &debugLocation, SelectionDAG &dag) const {
	targetConditionCode = evalTargetConditionCode(lhs, rhs, conditionCode);
	return dag.getNode(PROL16ISD::COMP, debugLocation, MVT::Glue, lhs, rhs);
}

PROL16CC::ConditionCode PROL16TargetLowering::evalTargetConditionCode(SDValue &lhs, SDValue &rhs,
																	  ISD::CondCode const conditionCode) const {
	assert(!lhs.getValueType().isFloatingPoint() && "PROL16 does not support floating point operations yet");

	switch (conditionCode) {
	case ISD::SETEQ:
		return PROL16CC::EQ;
	case ISD::SETNE:
		return PROL16CC::NE;
	case ISD::SETGT:
	case ISD::SETUGT:
		std::swap(lhs, rhs);
		LLVM_FALLTHROUGH;
	case ISD::SETLT:
	case ISD::SETULT:
		return PROL16CC::LT;
	case ISD::SETGE:
	case ISD::SETUGE:
		std::swap(lhs, rhs);
		LLVM_FALLTHROUGH;
	case ISD::SETLE:
	case ISD::SETULE:
		return PROL16CC::LE;
	default:
		llvm_unreachable("Unsupported integer condition");
	}

	return PROL16CC::ConditionCode::INVALID;
}

SDValue PROL16TargetLowering::lowerGlobalAddress(SDValue operation, SelectionDAG &dag) const {
	GlobalAddressSDNode const * const globalAddressNode = cast<GlobalAddressSDNode>(operation);
	GlobalValue const * const globalValue = globalAddressNode->getGlobal();
	int64_t const offset = globalAddressNode->getOffset();
	auto pointerValueType = getPointerTy(dag.getDataLayout());
	SDLoc debugLocation(operation);

	SDValue targetGlobalAddress = dag.getTargetGlobalAddress(globalValue, debugLocation, pointerValueType, offset);

	return SDValue(dag.getMachineNode(PROL16::LOADI, debugLocation, operation.getValueType(), targetGlobalAddress), 0);
}

SDValue PROL16TargetLowering::lowerCArguments(SDValue chain, CallingConv::ID callingConvention, bool isVarArg,
											  SmallVectorImpl<ISD::InputArg> const &inputArguments, SDLoc const &debugLocation,
											  SelectionDAG &dag, SmallVectorImpl<SDValue> &inVals) const {
	LLVM_DEBUG(dbgs() << "DAG at beginning of PROL16TargetLowering::lowerCArguments():\n");
	LLVM_DEBUG(dag.dump());

	MachineFunction &machineFunction = dag.getMachineFunction();
	MachineRegisterInfo &registerInfo = machineFunction.getRegInfo();

	// Assign locations to all of the incoming arguments.
	SmallVector<CCValAssign, 16> argumentLocations;
	CCState ccState(callingConvention, isVarArg, dag.getMachineFunction(), argumentLocations, *dag.getContext());

	ccState.AnalyzeFormalArguments(inputArguments, CC_PROL16);

	for (unsigned i = 0; i != argumentLocations.size(); ++i) {
		CCValAssign &ccValAssign = argumentLocations[i];

		if (ccValAssign.isRegLoc()) {
			// Arguments passed in registers
			EVT registerValueType = ccValAssign.getLocVT();
			if (registerValueType == MVT::i16) {
				unsigned const virtualRegister = registerInfo.createVirtualRegister(&PROL16::GR16RegClass);
				registerInfo.addLiveIn(ccValAssign.getLocReg(), virtualRegister);
				SDValue argumentValue = dag.getCopyFromReg(chain, debugLocation, virtualRegister, registerValueType);

				inVals.push_back(argumentValue);
			} else {
				errs() << "Encountered unexpected argument type '" << registerValueType.getEVTString()
					   << "' while lowering formal arguments\n";
				llvm_unreachable(nullptr);
			}
		}
	}

	LLVM_DEBUG(dbgs() << "DAG at end of PROL16TargetLowering::lowerCArguments():\n");
	LLVM_DEBUG(dag.dump());

	return chain;
}

SDValue PROL16TargetLowering::lowerCCall(CallLoweringInfo &callLoweringInfo, SmallVectorImpl<SDValue> &inVals) const {
	SelectionDAG &dag = callLoweringInfo.DAG;
	SDLoc &debugLocation = callLoweringInfo.DL;
	SmallVectorImpl<ISD::OutputArg> &outputArguments = callLoweringInfo.Outs;
	SDValue chain = callLoweringInfo.Chain;
	SDValue callee = callLoweringInfo.Callee;

	LLVM_DEBUG(dbgs() << "DAG at beginning of PROL16TargetLowering::lowerCCall():\n");
	LLVM_DEBUG(dbgs() << "callee: ");
	LLVM_DEBUG(callee.dump());
	LLVM_DEBUG(dag.dump());

	// Analyze operands of the call, assigning locations to each operand.
	SmallVector<CCValAssign, 16> argumentLocations;
	CCState ccState(callLoweringInfo.CallConv, callLoweringInfo.IsVarArg, callLoweringInfo.DAG.getMachineFunction(),
			argumentLocations, *callLoweringInfo.DAG.getContext());

	ccState.AnalyzeCallOperands(outputArguments, CC_PROL16);

	// Get the size of the outgoing arguments stack space requirement.
	// (Get a count of how many bytes are to be pushed on the stack.)
	unsigned const argumentsSize = ccState.getNextStackOffset();

	// Create the CALLSEQ_START node.
	chain = dag.getCALLSEQ_START(chain, argumentsSize, 0, debugLocation);

	SmallVector<std::pair<unsigned, SDValue>, 4> argumentRegisters;

	// Walk the register/memloc assignments, inserting copies/loads.
	for (unsigned i = 0; i != argumentLocations.size(); ++i) {
		CCValAssign &ccValAssign = argumentLocations[i];
		SDValue argument = callLoweringInfo.OutVals[i];

		// Promote the value if needed.
		switch (ccValAssign.getLocInfo()) {
		case CCValAssign::Full: break;
		case CCValAssign::SExt:
			argument = dag.getNode(ISD::SIGN_EXTEND, debugLocation, ccValAssign.getLocVT(), argument);
			break;
		case CCValAssign::ZExt:
			argument = dag.getNode(ISD::ZERO_EXTEND, debugLocation, ccValAssign.getLocVT(), argument);
			break;
		case CCValAssign::AExt:
			argument = dag.getNode(ISD::ANY_EXTEND, debugLocation, ccValAssign.getLocVT(), argument);
			break;
		case CCValAssign::BCvt:
			argument = dag.getNode(ISD::BITCAST, debugLocation, ccValAssign.getLocVT(), argument);
			break;
		default:
			llvm_unreachable("Invalid loc info!");
		}

		if (ccValAssign.isRegLoc()) {
			argumentRegisters.emplace_back(ccValAssign.getLocReg(), argument);
		} else {
			llvm_unreachable("Encountered unexpected argument location while lowering function call");
		}
	}

			// Build a sequence of copy-to-reg nodes chained together with token chain and
			// flag operands which copy the outgoing args into registers.  The InFlag in
			// necessary since all emitted instructions must be stuck together.
	SDValue inFlag;
	for (auto const &argumentRegister : argumentRegisters) {
		chain = dag.getCopyToReg(chain, debugLocation, argumentRegister.first, argumentRegister.second, inFlag);
			inFlag = chain.getValue(1);
	}

	// If the callee is a GlobalAddress node (quite common, every direct call is)
	// turn it into a TargetGlobalAddress node so that legalize doesn't hack it.
	// Likewise ExternalSymbol -> TargetExternalSymbol.
	if (GlobalAddressSDNode *globalAddressNode = dyn_cast<GlobalAddressSDNode>(callee)) {
		callee = dag.getTargetGlobalAddress(globalAddressNode->getGlobal(), debugLocation, MVT::i16);
	} else if (ExternalSymbolSDNode *externalSymbolNode = dyn_cast<ExternalSymbolSDNode>(callee)) {
		callee = dag.getTargetExternalSymbol(externalSymbolNode->getSymbol(), MVT::i16);
	}

	SDVTList nodeTypes = dag.getVTList(MVT::Other , MVT::Glue);
	SmallVector<SDValue, 8> operands;
	operands.push_back(chain);
	operands.push_back(callee);

	// Add argument registers to the end of the operand list so that they are
	// known live into the call!
	for (auto const &argumentRegister : argumentRegisters) {
		operands.push_back(dag.getRegister(argumentRegister.first, argumentRegister.second.getValueType()));
	}

	if (inFlag.getNode()) {
		operands.push_back(inFlag);
	}

	chain = dag.getNode(PROL16ISD::CALL, debugLocation, nodeTypes, operands);
	inFlag = chain.getValue(1);

	MVT const dataLayoutPointerType = getPointerTy(dag.getDataLayout());

	// Create the CALLSEQ_END node.
	chain = dag.getCALLSEQ_END(chain,
							   dag.getConstant(argumentsSize, debugLocation, dataLayoutPointerType, true),
							   dag.getConstant(0, debugLocation, dataLayoutPointerType, true),
							   inFlag, debugLocation);

	LLVM_DEBUG(dbgs() << "DAG at end of PROL16TargetLowering::lowerCCall():\n");
	LLVM_DEBUG(dbgs() << "callee: ");
	LLVM_DEBUG(callee.dump());
	LLVM_DEBUG(dag.dump());

	// Handle result values, copying them out of physical registers into virtual registers that we return
	return lowerCallResult(chain, callLoweringInfo, inVals);
}

SDValue PROL16TargetLowering::lowerCallResult(SDValue chain, CallLoweringInfo &callLoweringInfo,
											  SmallVectorImpl<SDValue> &inVals) const {
	SelectionDAG &dag = callLoweringInfo.DAG;

	LLVM_DEBUG(dbgs() << "DAG at beginning of PROL16TargetLowering::lowerCallResult():\n");
	LLVM_DEBUG(dbgs() << "callee: ");
	LLVM_DEBUG(callLoweringInfo.Callee.dump());
	LLVM_DEBUG(dag.dump());

	// Assign locations to each value returned by this call.
	SmallVector<CCValAssign, 16> resultValueLocations;
	CCState ccState(callLoweringInfo.CallConv, callLoweringInfo.IsVarArg, callLoweringInfo.DAG.getMachineFunction(),
					resultValueLocations, *callLoweringInfo.DAG.getContext());

	ccState.AnalyzeCallResult(callLoweringInfo.Ins, RetCC_PROL16);

	SDValue inFlag = chain.getValue(1);

	// Copy all of the result registers out of their specified physical registers.
	for (unsigned i = 0; i != resultValueLocations.size(); ++i) {
		chain = dag.getCopyFromReg(chain, callLoweringInfo.DL, resultValueLocations[i].getLocReg(),
								   resultValueLocations[i].getValVT(), inFlag).getValue(1);

		inFlag = chain.getValue(2);
		inVals.push_back(chain.getValue(0));
	}

	LLVM_DEBUG(dbgs() << "DAG at end of PROL16TargetLowering::lowerCallResult():\n");
	LLVM_DEBUG(dbgs() << "callee: ");
	LLVM_DEBUG(callLoweringInfo.Callee.dump());
	LLVM_DEBUG(dag.dump());

	return chain;
}

MachineBasicBlock* PROL16TargetLowering::emitStoreWithDisplacement(MachineInstr &machineInstruction,
																   MachineBasicBlock *machineBasicBlock) const {
	DebugLoc debugLocation = machineInstruction.getDebugLoc();
	MachineRegisterInfo &registerInfo = machineBasicBlock->getParent()->getRegInfo();
	TargetInstrInfo const &targetInstrInfo = *(machineBasicBlock->getParent()->getSubtarget().getInstrInfo());

	// machine instruction operand 0 = base address
	MachineOperand const &base = machineInstruction.getOperand(0);

	// machine instruction operand 1 = displacement
	MachineOperand const &displacement = machineInstruction.getOperand(1);

	LLVM_DEBUG(dbgs() << "PROL16TargetLowering::emitStoreWithDisplacementInstruction()\n");
	LLVM_DEBUG(machineInstruction.dump());

	unsigned const baseRegister = registerInfo.createVirtualRegister(&PROL16::GR16RegClass);
	BuildMI(*machineBasicBlock, machineInstruction, debugLocation, targetInstrInfo.get(PROL16::MOVE), baseRegister)
		.add(base);

	if (displacement.isImm()) {
		int64_t const offset = displacement.getImm();
//		if (offset != 0) {
			unsigned const displacementRegister = registerInfo.createVirtualRegister(&PROL16::GR16RegClass);

			BuildMI(*machineBasicBlock, machineInstruction, debugLocation, targetInstrInfo.get(PROL16::LOADI), displacementRegister)
				.addImm(offset);

			BuildMI(*machineBasicBlock, machineInstruction, debugLocation, targetInstrInfo.get(PROL16::SUB), baseRegister)
				.addReg(baseRegister).addReg(displacementRegister, RegState::Kill);

//		}
	} else {
		llvm_unreachable("Encountered unexpected machine instruction operand 1 while lowering STOREI");
	}

	unsigned sourceRegister = 0;

	bool const isStoreim = machineInstruction.getOpcode() == PROL16::STOREim;

	if (isStoreim) {
		// machine instruction operand 2 = immediate to store
		sourceRegister = registerInfo.createVirtualRegister(&PROL16::GR16RegClass);

		BuildMI(*machineBasicBlock, machineInstruction, debugLocation, targetInstrInfo.get(PROL16::LOADI), sourceRegister)
			.addImm(machineInstruction.getOperand(2).getImm());
	} else {
		// // machine instruction operand 2 = register to store
		sourceRegister = machineInstruction.getOperand(2).getReg();
	}

	BuildMI(*machineBasicBlock, machineInstruction, debugLocation, targetInstrInfo.get(PROL16::STORE))
		.addReg(sourceRegister, isStoreim ? RegState::Kill : 0).addReg(baseRegister, RegState::Kill);

	// remove the pseudo instruction
	machineInstruction.eraseFromParent();

	return machineBasicBlock;
}

MachineBasicBlock* PROL16TargetLowering::emitLoadWithDisplacement(MachineInstr &machineInstruction,
																  MachineBasicBlock *machineBasicBlock) const {
	DebugLoc debugLocation = machineInstruction.getDebugLoc();
	MachineRegisterInfo &registerInfo = machineBasicBlock->getParent()->getRegInfo();
	TargetInstrInfo const &targetInstrInfo = *(machineBasicBlock->getParent()->getSubtarget().getInstrInfo());

	// machine instruction operand 0 = destination register
	MachineOperand const &destination = machineInstruction.getOperand(0);

	// machine instruction operand 1 = base address
	MachineOperand const &base = machineInstruction.getOperand(1);

	// machine instruction operand 2 = displacement
	MachineOperand const &displacement = machineInstruction.getOperand(2);

	LLVM_DEBUG(dbgs() << "PROL16TargetLowering::emitLoadWithDisplacement()\n");
	LLVM_DEBUG(machineInstruction.dump());

	unsigned const baseRegister = registerInfo.createVirtualRegister(&PROL16::GR16RegClass);
	BuildMI(*machineBasicBlock, machineInstruction, debugLocation, targetInstrInfo.get(PROL16::MOVE), baseRegister)
		.add(base);

	if (displacement.isImm()) {
		int64_t const offset = displacement.getImm();
//		if (offset != 0) {
			unsigned const displacementRegister = registerInfo.createVirtualRegister(&PROL16::GR16RegClass);

			BuildMI(*machineBasicBlock, machineInstruction, debugLocation, targetInstrInfo.get(PROL16::LOADI), displacementRegister)
				.addImm(offset);

			BuildMI(*machineBasicBlock, machineInstruction, debugLocation, targetInstrInfo.get(PROL16::SUB), baseRegister)
				.addReg(baseRegister).addReg(displacementRegister, RegState::Kill);

//		}
	} else {
		llvm_unreachable("Encountered unexpected machine instruction operand 1 while lowering STOREI");
	}

	BuildMI(*machineBasicBlock, machineInstruction, debugLocation, targetInstrInfo.get(PROL16::LOAD), destination.getReg())
		.addReg(baseRegister, RegState::Kill);

	// remove the pseudo instruction
	machineInstruction.eraseFromParent();

	return machineBasicBlock;
}

MachineBasicBlock* PROL16TargetLowering::emitCallImmediate(MachineInstr &machineInstruction,
														   MachineBasicBlock *machineBasicBlock) const {
	LLVM_DEBUG(dbgs() << "PROL16TargetLowering::emitCallImmediate()\n");
	LLVM_DEBUG(machineInstruction.dump());

	DebugLoc debugLocation = machineInstruction.getDebugLoc();
	MachineRegisterInfo &registerInfo = machineBasicBlock->getParent()->getRegInfo();
	TargetInstrInfo const &targetInstrInfo = *(machineBasicBlock->getParent()->getSubtarget().getInstrInfo());

	unsigned const targetAddressRegister = registerInfo.createVirtualRegister(&PROL16::GR16RegClass);
	BuildMI(*machineBasicBlock, machineInstruction, debugLocation, targetInstrInfo.get(PROL16::LOADI), targetAddressRegister)
		.add(machineInstruction.getOperand(0));

	BuildMI(*machineBasicBlock, machineInstruction, debugLocation, targetInstrInfo.get(PROL16::JUMPcall))
		.addReg(targetAddressRegister, RegState::Kill)
		.add(ArrayRef<MachineOperand>(machineInstruction.operands_begin() + 4, machineInstruction.operands_end()));

	machineInstruction.eraseFromParent();

	return machineBasicBlock;
}

MachineBasicBlock* PROL16TargetLowering::emitReturn(MachineInstr &machineInstruction,
													MachineBasicBlock *machineBasicBlock) const {
	LLVM_DEBUG(dbgs() << "PROL16TargetLowering::emitReturn()\n");
	LLVM_DEBUG(machineInstruction.dump());

	DebugLoc debugLocation = machineInstruction.getDebugLoc();
	MachineFunction *machineFunction = machineBasicBlock->getParent();
	TargetInstrInfo const &targetInstrInfo = *(machineFunction->getSubtarget().getInstrInfo());
	MachineRegisterInfo &registerInfo = machineFunction->getRegInfo();

	unsigned const returnAddressRegister = machineInstruction.getOperand(0).getReg();
	assert(returnAddressRegister == registerInfo.getTargetRegisterInfo()->getRARegister());

	if ((machineFunction->getName() == "main") || (machineFunction->getName() == "main.main")) {
		BuildMI(*machineBasicBlock, machineInstruction, debugLocation, targetInstrInfo.get(PROL16::SLEEP));
	} else {
		// it is important to use JUMPret here due to its "isReturn" property, which is needed by the PEI pass
		// to properly collect the restore blocks for which epilogue insertion is done
		BuildMI(*machineBasicBlock, machineInstruction, debugLocation, targetInstrInfo.get(PROL16::JUMPret))
			.add(makeArrayRef(machineInstruction.operands_begin(), machineInstruction.operands_end()));
	}

	machineInstruction.eraseFromParent();

	return machineBasicBlock;
}

MachineBasicBlock* PROL16TargetLowering::emitDirectBranch(MachineInstr &machineInstruction,
														  MachineBasicBlock *machineBasicBlock) const {
	LLVM_DEBUG(dbgs() << "PROL16TargetLowering::emitDirectBranch()\n");
	LLVM_DEBUG(machineInstruction.dump());

	DebugLoc debugLocation = machineInstruction.getDebugLoc();
	MachineFunction *machineFunction = machineBasicBlock->getParent();
	TargetInstrInfo const &targetInstrInfo = *(machineFunction->getSubtarget().getInstrInfo());
	MachineRegisterInfo &registerInfo = machineFunction->getRegInfo();

	unsigned const jumpTargetRegister = registerInfo.createVirtualRegister(&PROL16::GR16RegClass);

	BuildMI(*machineBasicBlock, machineInstruction, debugLocation, targetInstrInfo.get(PROL16::LOADI), jumpTargetRegister)
		.add(machineInstruction.getOperand(0));

	BuildMI(*machineBasicBlock, machineInstruction, debugLocation, targetInstrInfo.get(PROL16::JUMP))
		.addReg(jumpTargetRegister, RegState::Kill);

	machineInstruction.eraseFromParent();

	return machineBasicBlock;
}

MachineBasicBlock* PROL16TargetLowering::emitConditionalBranch(MachineInstr &machineInstruction,
															   MachineBasicBlock *machineBasicBlock) const {
	LLVM_DEBUG(dbgs() << "PROL16TargetLowering::emitConditionalBranch()\n");
	LLVM_DEBUG(machineInstruction.dump());

	DebugLoc debugLocation = machineInstruction.getDebugLoc();
	MachineFunction *machineFunction = machineBasicBlock->getParent();
	TargetInstrInfo const &targetInstrInfo = *(machineFunction->getSubtarget().getInstrInfo());
	MachineRegisterInfo &registerInfo = machineFunction->getRegInfo();

	unsigned const jumpTargetRegister = registerInfo.createVirtualRegister(&PROL16::GR16RegClass);

	BuildMI(*machineBasicBlock, machineInstruction, debugLocation, targetInstrInfo.get(PROL16::LOADI), jumpTargetRegister)
		.add(machineInstruction.getOperand(0));

	machineInstruction.getOperand(0).ChangeToRegister(jumpTargetRegister, false, false, true);

	return machineBasicBlock;
}

MachineBasicBlock* PROL16TargetLowering::emitJumpcz(MachineInstr &machineInstruction,
													MachineBasicBlock *machineBasicBlock) const {
	LLVM_DEBUG(dbgs() << "PROL16TargetLowering::emitJumpcz()\n");
	LLVM_DEBUG(machineInstruction.dump());

	DebugLoc debugLocation = machineInstruction.getDebugLoc();
	MachineFunction *machineFunction = machineBasicBlock->getParent();
	TargetInstrInfo const &targetInstrInfo = *(machineFunction->getSubtarget().getInstrInfo());
	MachineRegisterInfo &registerInfo = machineFunction->getRegInfo();

	unsigned jumpTargetRegister = 0;
	MachineOperand &jumpTargetOperand = machineInstruction.getOperand(0);

	if (jumpTargetOperand.isReg()) {
		jumpTargetRegister = jumpTargetOperand.getReg();
	} else {
		jumpTargetRegister = registerInfo.createVirtualRegister(&PROL16::GR16RegClass);

		BuildMI(*machineBasicBlock, machineInstruction, debugLocation, targetInstrInfo.get(PROL16::LOADI), jumpTargetRegister)
			.add(jumpTargetOperand);
	}

	BuildMI(*machineBasicBlock, machineInstruction, debugLocation, targetInstrInfo.get(PROL16::JUMPC), jumpTargetRegister);

	BuildMI(*machineBasicBlock, machineInstruction, debugLocation, targetInstrInfo.get(PROL16::JUMPZ))
		.addReg(jumpTargetRegister, RegState::Kill);

	machineInstruction.eraseFromParent();

	return machineBasicBlock;
}

MachineBasicBlock* PROL16TargetLowering::emitJumpnz(MachineInstr &machineInstruction,
													MachineBasicBlock *machineBasicBlock) const {
	LLVM_DEBUG(dbgs() << "PROL16TargetLowering::emitJumpnz()\n");
	LLVM_DEBUG(machineInstruction.dump());

	DebugLoc debugLocation = machineInstruction.getDebugLoc();
	MachineFunction *machineFunction = machineBasicBlock->getParent();
	TargetInstrInfo const &targetInstrInfo = *(machineFunction->getSubtarget().getInstrInfo());
	MachineRegisterInfo &registerInfo = machineFunction->getRegInfo();

	/**
	 * JUMPNZ expansion:
	 * =================
	 * Since the PROL16 architecture has no conditional JUMP instruction that jumps when the zero flag is not set,
	 * we have to do a little trick to implement that behavior.
	 *
	 * Before expanding JUMPNZ, the DAG looks like:
	 *       t37: glue = PROL16ISD::COMP t30, Constant:i16<0>
     *     t38: ch = PROL16ISD::JUMPNZ t0, BasicBlock:ch< 0x4eebc58>, t37
  	 *	 t16: ch = br t38, BasicBlock:ch< 0x4eebb90>
  	 *
  	 * So, the generated code should jump to 0x4eebc58 if the zero flag is not set and to 0x4eebb90 otherwise (i.e. when the
  	 * zero flag is set).
  	 *
  	 * Therefore, this behavior is equivalent to:
	 *       t37: glue = PROL16ISD::COMP t30, Constant:i16<0>
     *     t38: ch = PROL16ISD::JUMPZ t0, BasicBlock:ch< 0x4eebb90>, t37
  	 *	 t16: ch = br t38, BasicBlock:ch< 0x4eebc58>
  	 *
  	 * This code jumps to 0x4eebb90 when the zero flag is set and to 0x4eebc58 otherwise (i.e. when the zero flag
  	 * is not set).
  	 *
  	 * This is exactly what the following code does---it emits a JUMPZ instruction and swaps the target addresses.
	 */
	auto nextInstructionItor = std::next(MachineBasicBlock::iterator(machineInstruction));
	assert(nextInstructionItor->getOpcode() == PROL16::BR && "jumpnz expansion: unexpected non-br instruction after jumpnz");

	unsigned jumpTargetRegister = 0;
	MachineOperand const &jumpTargetOperand = nextInstructionItor->getOperand(0);

	if (jumpTargetOperand.isReg()) {
		jumpTargetRegister = jumpTargetOperand.getReg();
	} else {
		jumpTargetRegister = registerInfo.createVirtualRegister(&PROL16::GR16RegClass);

		BuildMI(*machineBasicBlock, machineInstruction, debugLocation, targetInstrInfo.get(PROL16::LOADI), jumpTargetRegister)
			.add(jumpTargetOperand);
	}

	BuildMI(*machineBasicBlock, machineInstruction, debugLocation, targetInstrInfo.get(PROL16::JUMPZ))
		.addReg(jumpTargetRegister, RegState::Kill);

	// swap the target addresses
	nextInstructionItor->getOperand(0) = machineInstruction.getOperand(0);

	machineInstruction.eraseFromParent();

	return machineBasicBlock;
}

MachineBasicBlock* PROL16TargetLowering::emitShiftRightArithmetical(MachineInstr &machineInstruction,
																	MachineBasicBlock *machineBasicBlock) const {
	LLVM_DEBUG(dbgs() << "PROL16TargetLowering::emitShiftRightArithmetical()\n");
	LLVM_DEBUG(machineInstruction.dump());

	DebugLoc debugLocation = machineInstruction.getDebugLoc();
	MachineFunction *machineFunction = machineBasicBlock->getParent();
	TargetInstrInfo const &targetInstrInfo = *(machineFunction->getSubtarget().getInstrInfo());
	MachineRegisterInfo &registerInfo = machineFunction->getRegInfo();

	/**
	 * SRA (Shift Right Arithmetic) expansion:
	 * =======================================
	 * From https://open4tech.com/logical-vs-arithmetic-shift/:
	 * 		"A Right Arithmetic Shift of one position moves each bit to the right by one.
	 * 		The least significant bit is discarded and the vacant MSB is filled with the value of the previous
	 * 		(now shifted one position to the right) MSB."
	 *
	 * Since the PROL16 architecture does not provide arithmetic shift instructions,
	 * we have to emulate it with the following approach:
	 * 1. Copy the source register into a temporary register.
	 * 2. Apply a logical left shift (SHL) to the temporary register to set the carry flag to the value of the MSB.
	 * 3. Use a logical right shift with carry (SHRC) to do the actual right shift, shifting the carry bit
	 * (which has the value of the previous MSB) into the MSB.
	 */
	unsigned const tmpRegister = registerInfo.createVirtualRegister(&PROL16::GR16RegClass);
	unsigned const destinationRegister = machineInstruction.getOperand(0).getReg();
	unsigned const sourceRegister = machineInstruction.getOperand(1).getReg();

	BuildMI(*machineBasicBlock, machineInstruction, debugLocation, targetInstrInfo.get(PROL16::MOVE), tmpRegister)
		.addReg(sourceRegister);

	BuildMI(*machineBasicBlock, machineInstruction, debugLocation, targetInstrInfo.get(PROL16::SHL), tmpRegister)
		.addReg(tmpRegister, RegState::Kill);

	BuildMI(*machineBasicBlock, machineInstruction, debugLocation, targetInstrInfo.get(PROL16::SHRC), destinationRegister)
		.addReg(destinationRegister);

	machineInstruction.eraseFromParent();

	return machineBasicBlock;
}

MachineBasicBlock* PROL16TargetLowering::emitShiftWithImmediateCount(MachineInstr &machineInstruction,
																	 MachineBasicBlock *machineBasicBlock) const {
	LLVM_DEBUG(dbgs() << "PROL16TargetLowering::emitShiftWithImmediateCount()\n");
	LLVM_DEBUG(machineInstruction.dump());

	DebugLoc debugLocation = machineInstruction.getDebugLoc();
	MachineFunction *machineFunction = machineBasicBlock->getParent();
	TargetInstrInfo const &targetInstrInfo = *(machineFunction->getSubtarget().getInstrInfo());

	unsigned const destinationRegister = machineInstruction.getOperand(0).getReg();
	unsigned const sourceRegister = machineInstruction.getOperand(1).getReg();
	int64_t const shiftCount = machineInstruction.getOperand(2).getImm();

	BuildMI(*machineBasicBlock, machineInstruction, debugLocation, targetInstrInfo.get(PROL16::MOVE), destinationRegister)
		.addReg(sourceRegister);

	for (int64_t i = 0; i < shiftCount; ++i) {
		switch (machineInstruction.getOpcode()) {
		case PROL16::SHLi:
			BuildMI(*machineBasicBlock, machineInstruction, debugLocation, targetInstrInfo.get(PROL16::SHL), destinationRegister)
				.addReg(destinationRegister);
			break;
		case PROL16::SHRi:
			BuildMI(*machineBasicBlock, machineInstruction, debugLocation, targetInstrInfo.get(PROL16::SHR), destinationRegister)
				.addReg(destinationRegister);
			break;
		case PROL16::SRAi:
			emitShiftRightArithmetical(machineInstruction, machineBasicBlock);
			break;
		}
	}

	machineInstruction.eraseFromParent();

	return machineBasicBlock;
}

MachineBasicBlock* PROL16TargetLowering::emitShiftWithRegisterCount(MachineInstr &machineInstruction,
																	MachineBasicBlock *machineBasicBlock) const {
	LLVM_DEBUG(dbgs() << "PROL16TargetLowering::emitShiftWithRegisterCount()\n");
	LLVM_DEBUG(machineInstruction.dump());

//	DebugLoc debugLocation = machineInstruction.getDebugLoc();
//	MachineFunction *machineFunction = machineBasicBlock->getParent();
//	TargetInstrInfo const &targetInstrInfo = *(machineFunction->getSubtarget().getInstrInfo());
//	MachineRegisterInfo &registerInfo = machineFunction->getRegInfo();

	llvm_unreachable("PROL16TargetLowering::emitShiftWithRegisterCount not implemented yet");
	machineInstruction.eraseFromParent();

	return machineBasicBlock;
}

MachineBasicBlock* PROL16TargetLowering::emitSelect(MachineInstr &machineInstruction,
													MachineBasicBlock *machineBasicBlock) const {
	LLVM_DEBUG(dbgs() << "PROL16TargetLowering::emitSelect()\n");
	LLVM_DEBUG(machineInstruction.dump());

	MachineFunction * const machineFunction = machineBasicBlock->getParent();
	MachineRegisterInfo &registerInfo = machineFunction->getRegInfo();

	BasicBlock const * const basicBlock = machineBasicBlock->getBasicBlock();
	MachineFunction::iterator itor = ++machineBasicBlock->getIterator();

	// To "insert" a SELECT instruction, we actually have to insert the diamond
	// control-flow pattern.  The incoming instruction knows the destination vreg
	// to set, the condition code register to branch on, the true/false values to
	// select between, and a branch opcode to use.

	// trueMBB:
	// ...
	// %trueValue = ...
	// comp r1, r2
	// jump? resultMBB
	// # fall through to falseMBB
	MachineBasicBlock * const trueMBB = machineBasicBlock;
	MachineBasicBlock * const falseMBB = machineFunction->CreateMachineBasicBlock(basicBlock);
	MachineBasicBlock * const resultMBB = machineFunction->CreateMachineBasicBlock(basicBlock);
	machineFunction->insert(itor, falseMBB);
	machineFunction->insert(itor, resultMBB);

	// update machine-CFG edges by transferring all successors of the current
	// block to the new block which will contain the PHI node for the select.
	resultMBB->splice(resultMBB->begin(), machineBasicBlock,
					  std::next(MachineBasicBlock::iterator(machineInstruction)),
					  machineBasicBlock->end());
	resultMBB->transferSuccessorsAndUpdatePHIs(machineBasicBlock);

	// next, add the false and result blocks as its successors.
	machineBasicBlock->addSuccessor(falseMBB);
	machineBasicBlock->addSuccessor(resultMBB);

	DebugLoc debugLocation = machineInstruction.getDebugLoc();
	TargetInstrInfo const &targetInstrInfo = *machineFunction->getSubtarget().getInstrInfo();

	unsigned const jumpTargetRegister = registerInfo.createVirtualRegister(&PROL16::GR16RegClass);

	BuildMI(machineBasicBlock, debugLocation, targetInstrInfo.get(PROL16::LOADI), jumpTargetRegister)
		.addMBB(resultMBB);

	BuildMI(machineBasicBlock, debugLocation, targetInstrInfo.get(PROL16::JUMPcc))
		.addReg(jumpTargetRegister)
		.addReg(machineInstruction.getOperand(1).getReg())
		.addReg(machineInstruction.getOperand(2).getReg())
		.addImm(machineInstruction.getOperand(5).getImm());

	// falseMBB:
	// %falseValue = ...
	// # fall through to resultMBB
	falseMBB->addSuccessor(resultMBB);	// update machine-CFG edges

	// resultMBB:
	// %result = phi [%falseValue, falseMBB], [%trueValue, trueMBB]
	// ...
	BuildMI(*resultMBB, resultMBB->begin(), debugLocation, targetInstrInfo.get(PROL16::PHI), machineInstruction.getOperand(0).getReg())
		.addReg(machineInstruction.getOperand(4).getReg())
		.addMBB(falseMBB)
		.addReg(machineInstruction.getOperand(3).getReg())
		.addMBB(trueMBB);

	machineInstruction.eraseFromParent();
	return resultMBB;
}
