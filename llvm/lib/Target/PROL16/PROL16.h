//===-- PROL16.h - Top-level interface for PROL16 representation --*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the entry points for global functions defined in the LLVM
// PROL16 back-end.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_PROL16_PROL16_H
#define LLVM_LIB_TARGET_PROL16_PROL16_H

#include "llvm/Pass.h"

#include "PROL16TargetMachine.h"

namespace llvm {

/// createPROL16ISelDag - This pass converts a legalized DAG into a
/// PROL16-specific DAG, ready for instruction scheduling.
FunctionPass* createPROL16ISelDag(PROL16TargetMachine &targetMachine);

}

#endif
