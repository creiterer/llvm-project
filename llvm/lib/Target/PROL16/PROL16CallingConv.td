//===-- PROL16CallingConv.td - Calling Conventions PROL16 ----*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This describes the calling conventions for the PROL16 architecture.
//
//===----------------------------------------------------------------------===//

def CC_PROL16 : CallingConv<[
  // Promote i8 arguments to i16.
  CCIfType<[i8], CCPromoteToType<i16>>,
  
  // i16 arguments get passed in registers if there is space.
  CCIfType<[i16], CCAssignToReg<[R4, R5, R6, R7, R8, R9, 
  								 R10, R11, R12, R13, R14, R15]>>,

  // Alternatively, they get stored in stack slots that are 2 bytes in
  // size and 2-byte aligned.
  CCAssignToStack<2, 2>
]>;

def RetCC_PROL16 : CallingConv<[
  CCIfType<[i16], CCAssignToReg<[R4, R5, R6, R7, R8, R9, 
  								 R10, R11, R12, R13, R14, R15]>>
]>;
