###############################################################################
## author:      creiterer
## brief:       This script initializes the LLVM cmake build.
## usage:       ./initLLVMBuild.sh LLVM_EXPERIMENTAL_TARGETS_TO_BUILD LLVM_TARGETS_TO_BUILD LLVM_PROJECTS_TO_BUILD
## example:     ./initLLVMBuild.sh "PROL16" "X86;Sparc;MSP430" "clang;llgo"
###############################################################################
#!/bin/bash

readonly LLVM_SOURCE_DIR="llvm-project/llvm"
readonly LLVM_INSTALL_DIR="/tmp/llvm"

readonly C_COMPILER="clang"
readonly CPP_COMPILER="clang++"
readonly GENERATOR="Ninja"

function buildLLVM {
    # check if experimental build targets are provided
    if [ "$1" ]; then
        readonly LLVM_EXPERIMENTAL_TARGETS_TO_BUILD="$1"
    else
        readonly LLVM_EXPERIMENTAL_TARGETS_TO_BUILD="PROL16"
    fi
    echo "The following experimental targets are going to be built: ${LLVM_EXPERIMENTAL_TARGETS_TO_BUILD}"

    readonly LLVM_TARGETS_TO_BUILD="$2"
    if [ ${LLVM_TARGETS_TO_BUILD} ]; then
        echo "The following targets are going to be built: ${LLVM_TARGETS_TO_BUILD}"
    fi

    pushd ${LLVM_SOURCE_DIR}
    readonly LLVM_BRANCH=$(git rev-parse --abbrev-ref HEAD | tr "/" "-")
    popd

    readonly LLVM_PROJECTS_TO_BUILD="$3"
    if [ ${LLVM_PROJECTS_TO_BUILD} ]; then
        echo "The following projects are going to be built: ${LLVM_PROJECTS_TO_BUILD}"

        readonly LLVM_PROJECTS=$(echo ${LLVM_PROJECTS_TO_BUILD} | tr ";" "-")
        readonly LLVM_BUILD_DIR="build_llvm-${LLVM_PROJECTS}-${LLVM_BRANCH}"
    else
        readonly LLVM_BUILD_DIR="build_llvm-${LLVM_BRANCH}"
    fi
    
    if [ ! -d "$LLVM_BUILD_DIR" ]; then
        echo "Build directory \"${LLVM_BUILD_DIR}\" doesn't exist -> initializing LLVM cmake build"
        mkdir ${LLVM_BUILD_DIR}
        cd ${LLVM_BUILD_DIR}

        if [ ${LLVM_TARGETS_TO_BUILD} ]; then
            if [ ${LLVM_PROJECTS_TO_BUILD} ]; then
                cmake -G ${GENERATOR} -DCMAKE_C_COMPILER=${C_COMPILER} -DCMAKE_CXX_COMPILER=${CPP_COMPILER} -DCMAKE_INSTALL_PREFIX=${LLVM_INSTALL_DIR} -DLLVM_EXPERIMENTAL_TARGETS_TO_BUILD="${LLVM_EXPERIMENTAL_TARGETS_TO_BUILD}" -DLLVM_TARGETS_TO_BUILD="${LLVM_TARGETS_TO_BUILD}" -DLLVM_ENABLE_PROJECTS="${LLVM_PROJECTS_TO_BUILD}" "../${LLVM_SOURCE_DIR}/"
            else
                cmake -G ${GENERATOR} -DCMAKE_C_COMPILER=${C_COMPILER} -DCMAKE_CXX_COMPILER=${CPP_COMPILER} -DCMAKE_INSTALL_PREFIX=${LLVM_INSTALL_DIR} -DLLVM_EXPERIMENTAL_TARGETS_TO_BUILD="${LLVM_EXPERIMENTAL_TARGETS_TO_BUILD}" -DLLVM_TARGETS_TO_BUILD="${LLVM_TARGETS_TO_BUILD}" "../${LLVM_SOURCE_DIR}/"
            fi
        else
            if [ ${LLVM_PROJECTS_TO_BUILD} ]; then
                cmake -G ${GENERATOR} -DCMAKE_C_COMPILER=${C_COMPILER} -DCMAKE_CXX_COMPILER=${CPP_COMPILER} -DCMAKE_INSTALL_PREFIX=${LLVM_INSTALL_DIR} -DLLVM_EXPERIMENTAL_TARGETS_TO_BUILD="${LLVM_EXPERIMENTAL_TARGETS_TO_BUILD}" -DLLVM_ENABLE_PROJECTS="${LLVM_PROJECTS_TO_BUILD}" "../${LLVM_SOURCE_DIR}/"
            else
                cmake -G ${GENERATOR} -DCMAKE_C_COMPILER=${C_COMPILER} -DCMAKE_CXX_COMPILER=${CPP_COMPILER} -DCMAKE_INSTALL_PREFIX=${LLVM_INSTALL_DIR} -DLLVM_EXPERIMENTAL_TARGETS_TO_BUILD="${LLVM_EXPERIMENTAL_TARGETS_TO_BUILD}" "../${LLVM_SOURCE_DIR}/"
            fi
        fi
        cd ..
    else
        echo "Build directory \"${LLVM_BUILD_DIR}\" exists -> LLVM cmake build is already initialized -> nothing to do"
    fi
}

# set up build of the PROL16 LLVM backend
buildLLVM $1 $2 $3
