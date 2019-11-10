//===-- PROL16ConditionCodes.h - PROL16 specific condition codes -------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains an enum describing the PROL16 specific condition codes.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_7_0_0_SRC_LIB_TARGET_PROL16_PROL16CONDITIONCODES_H_
#define LLVM_7_0_0_SRC_LIB_TARGET_PROL16_PROL16CONDITIONCODES_H_


namespace llvm {
namespace PROL16CC {

enum ConditionCode {
	EQ = 0,
	NE = 1,
	LE = 2,
	LT = 3,
	INVALID = -1,
};

} // end namespace PROL16
} // end namespace llvm

#endif /* LLVM_7_0_0_SRC_LIB_TARGET_PROL16_PROL16CONDITIONCODES_H_ */
