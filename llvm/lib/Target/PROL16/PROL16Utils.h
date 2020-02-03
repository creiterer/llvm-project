//===-- PROL16Utils.h - PROL16 utility functions -------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// PROL16 utility functions.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_PROJECT_LLVM_LIB_TARGET_PROL16_PROL16UTILS_H_
#define LLVM_PROJECT_LLVM_LIB_TARGET_PROL16_PROL16UTILS_H_

#include "llvm/CodeGen/MachineInstrBuilder.h"

#include <cstdint>
#include <type_traits>

namespace llvm {
namespace prol16 {
namespace util {

template <typename T>
inline T calcOffset(T const offset) noexcept {
	static_assert(std::is_integral<T>::value, "calcOffset requires an integral value type.");

	// https://stackoverflow.com/questions/7594508/modulo-operator-with-negative-values
	return offset / 2 + offset % 2;
}

inline bool isKill(unsigned const flags) noexcept {
	return flags & RegState::Kill;
}

}	// end namespace util
}	// end namespace prol16
} 	// end namespace llvm

#endif /* LLVM_PROJECT_LLVM_LIB_TARGET_PROL16_PROL16UTILS_H_ */
