set(LLVM_TARGET_DEFINITIONS PROL16.td)

#tablegen(LLVM PROL16GenAsmMatcher.inc -gen-asm-matcher)
tablegen(LLVM PROL16GenAsmWriter.inc -gen-asm-writer)
tablegen(LLVM PROL16GenCallingConv.inc -gen-callingconv)
tablegen(LLVM PROL16GenDAGISel.inc -gen-dag-isel)
#tablegen(LLVM PROL16GenDisassemblerTables.inc -gen-disassembler)
tablegen(LLVM PROL16GenInstrInfo.inc -gen-instr-info)
#tablegen(LLVM PROL16GenMCCodeEmitter.inc -gen-emitter)
tablegen(LLVM PROL16GenRegisterInfo.inc -gen-register-info)
tablegen(LLVM PROL16GenSubtargetInfo.inc -gen-subtarget)
tablegen(LLVM PROL16GenMCPseudoLowering.inc -gen-pseudo-lowering)

add_public_tablegen_target(PROL16CommonTableGen)

add_llvm_target(PROL16CodeGen
	PROL16AsmPrinter.cpp
	PROL16InstrInfo.cpp
	PROL16ISelDAGToDAG.cpp
	PROL16ISelLowering.cpp
	PROL16FrameLowering.cpp
#	PROL16MachineFunctionInfo.cpp
	PROL16RegisterInfo.cpp
	PROL16Subtarget.cpp
	PROL16TargetMachine.cpp
	PROL16MachineInstrLowering.cpp
#	PROL16TargetObjectFile.cpp
)

#add_subdirectory(AsmParser)
#add_subdirectory(Disassembler)
add_subdirectory(InstPrinter)
add_subdirectory(MCTargetDesc)
add_subdirectory(TargetInfo)
