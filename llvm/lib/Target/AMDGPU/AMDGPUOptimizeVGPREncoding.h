//===- AMDGPUOptimizeVGPREncoding.h -----------------------------*- C++- *-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_AMDGPU_AMDGPUOPTIMIZEVGPRENCODING_H
#define LLVM_LIB_TARGET_AMDGPU_AMDGPUOPTIMIZEVGPRENCODING_H

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/CodeGen/MachinePassManager.h"
#include "llvm/CodeGen/Register.h"

namespace llvm {
class MachineFunction;

class AMDGPUOptimizeVGPREncodingPass
    : public OptionalPassInfoMixin<AMDGPUOptimizeVGPREncodingPass> {
public:
  PreservedAnalyses run(MachineFunction &MF,
                        MachineFunctionAnalysisManager &MFAM);
};

namespace AMDGPU {

/// Per-virtual-register summary of MSB-group "neighbors": the registers it
/// shares an operand dial (src0/src1/src2/dst) with in an adjacent MODE-using
/// instruction, i.e. the registers it would want to share an MSB group with to
/// avoid a MODE-setting instruction. Derived purely from instruction operands,
/// so this is valid *before* physical-register assignment and can drive
/// register-allocation hints.
struct MSBGroupHints {
  /// Neighbor registers (virtual or physical), accumulated with multiplicity
  /// across all dials and occurrences so that frequent neighbors weigh more.
  SmallVector<Register, 4> Neighbors;
  /// Number of occurrences adjacent to a basic-block boundary, which pins the
  /// default MSB group; i.e. a vote for the virtual register to live in the
  /// default group (group 0).
  unsigned DefaultGroupVotes = 0;
  /// Whether the virtual register cannot change MSB group (confined to the
  /// first 256 VGPRs, or a VOPD operand).
  bool Pinned = false;
};

/// Collects per-virtual-register \ref MSBGroupHints for \p MF. Phase-agnostic:
/// only reads instruction operands, not register assignments.
void collectMSBGroupHints(const MachineFunction &MF,
                          DenseMap<Register, MSBGroupHints> &Out);

} // namespace AMDGPU
} // namespace llvm

#endif // LLVM_LIB_TARGET_AMDGPU_AMDGPUOPTIMIZEVGPRENCODING_H
