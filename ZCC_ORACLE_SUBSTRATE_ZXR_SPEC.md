# 🔱 ZCC ORACLE RUNTIME SUBSTRATE & EXECUTION RECORD (.ZXR) SPECIFICATION

> **Doctrine**: All compiler invariants are queryable; all compilation decisions are deterministic, recordable, and replayable. We transition the compiler from a stateless transformer into a verifiable, time-travel computational substrate.

---

## 🔱 1. THE ARCHITECTURAL FLOW

The Oracle Runtime Substrate provides the shared execution fabric, telemetry dispatchers, invariant collectors, and stable fingerprinting engine. Every compilation run can be logged into a strictly deterministic, append-only **ZCC Execution Record (`.zxr`)** file.

```text
       Source File (.c) ──► [ ZCC Parser & Codegen ] ──► [ ASM Emission (.s) ]
                                   │
                                   ▼
                   ╔═══════════════════════════════╗
                   ║    ORACLE RUNTIME SUBSTRATE   ║
                   ╚═══════════════╦═══════════════╝
                                   │
         ┌─────────────────────────┼─────────────────────────┐
         ▼                         ▼                         ▼
 ┌───────────────┐         ┌───────────────┐         ┌───────────────┐
 │ Invariant     │         │ stable        │         │ execution     │
 │ Collectors    │         │ Fingerprints  │         │ Record (.zxr) │
 └───────┬───────┘         └───────┬───────┘         └───────┬───────┘
         │                         │                         │
         ▼                         ▼                         ▼
   --oracle-stack            [CFG/ABI/Stack]           zcc --replay
```

---

## 🔱 2. THE ORACLE RUNTIME SUBSTRATE C INTERFACE

To ensure absolute portability and zero external dependency, the Substrate is built natively in pure C:

```c
/* ================================================================= */
/* ZCC ORACLE RUNTIME SUBSTRATE ENGINE                               */
/* ================================================================= */

#ifndef ZCC_ORACLE_SUBSTRATE_H
#define ZCC_ORACLE_SUBSTRATE_H

#include <stdio.h>
#include <stdint.h>

#define MAX_FINDINGS 1024
#define FINGERPRINT_LEN 64

typedef struct {
    char type[64];
    char message[256];
    int line_number;
    int critical;
} InvariantFinding;

typedef struct {
    const char *oracle_name;
    const char *status; /* "VERIFIED" or "DIVERGENCE_DETECTED" */
    char fingerprint[FINGERPRINT_LEN + 1];
    InvariantFinding findings[MAX_FINDINGS];
    int num_findings;
} OracleReport;

typedef struct {
    const char *name;
    int (*run)(void *cc, OracleReport *report);
} InvariantOracle;

/* Shared Oracle Registry */
typedef struct {
    InvariantOracle oracles[32];
    int num_oracles;
} OracleRegistry;

/* ================================================================= */
/* STABLE COMPILER FINGERPRINTS                                      */
/* ================================================================= */

typedef struct {
    uint64_t cfg_topology_hash;      /* Structural control-flow complexity */
    uint64_t abi_lowering_hash;      /* Calling convention register maps */
    uint64_t stack_geometry_hash;    /* Local/spill alignments and offsets */
    uint64_t regalloc_sequence_hash; /* Graph coloring allocation order */
    uint64_t pass_pipeline_hash;     /* Active optimization pass traces */
    uint64_t entropy_seed_hash;      /* Environmental baseline signature */
} CompilerFingerprint;

#endif /* ZCC_ORACLE_SUBSTRATE_H */
```

---

## 🔱 3. THE FOUNDATIONAL ORACLE: `--oracle-stack`

Stack topology unifies calling convention registers, aggregate spills, stack adjustments, and alignment invariants. The Stack Oracle validates frame integrity at every callsite:

### 🔱 3.1 Stack Frame Invariants
1. **RSP 16-Byte Realignment**: Verifies that the stack pointer is aligned to 16 bytes immediately prior to executing a `call` instruction.
2. **Push/Pop Balance**: Ensures any caller-saved GPR pushes are exactly popped before exiting the active block or function.
3. **Spill Boundary Protections**: Asserts that local variable alignments do not overlap with spill slots or parameters passed on the stack.

### 🔱 3.2 Stack Oracle Telemetry Output
```json
{
  "oracle": "stack",
  "status": "VERIFIED",
  "fingerprint": "8fa3c0eb01ffac3e2849de748b0a1122efcc7a88998b417c8d9e6e8e2c0db56e7",
  "inspections": [
    {
      "function": "consume_aggregate",
      "rsp_alignment_before_call": 16,
      "rsp_alignment_after_call": 16,
      "callee_saved_registers_preserved": true,
      "spill_headroom_bytes": 32,
      "max_frame_bytes": 96,
      "findings": []
    }
  ]
  "divergences": []
}
```

---

## 🔱 4. ZCC EXECUTION RECORDS (`.ZXR`) SPECIFICATION

Instead of throwing away compiler decision state after code-generation, ZCC records every optimization, register coloring choice, and lowering pass into a **ZCC Execution Record (`.zxr`)** file.

### 🔱 4.1 CLI Interface
* **Emit Execution Record**:
  ```bash
  zcc --emit-exec-record <source.c> -o <artifact.zxr>
  ```
* **Replay Execution Record**:
  ```bash
  zcc --replay-record <artifact.zxr>
  ```
* **Diff Compilation History**:
  ```bash
  zcc --diff-record <run_a.zxr> <run_b.zxr>
  ```

### 🔱 4.2 Structural Serialization Constraints
To ensure absolute, cross-platform reproducibility, the `.zxr` binary format enforces the following strict constraints:
1. **No Raw Pointer Leakage**: All node relations must be serialized as relative integer indices or canonical symbol identifiers.
2. **ASLR Independence**: Heap allocator memory locations must not influence node iteration orders.
3. **Append-Only Sequence**: All compiler pass events, CFG transforms, and register allocations are recorded as a linear time-series.
4. **Stable Versioning**: Includes a strict compiler version and runtime ABI check header.

### 🔱 4.3 `.zxr` JSON Equivalent Schema
```json
{
  "zxr_version": "1.0.0",
  "compiler_version": "zcc_prime_1.1.0",
  "source_identity": {
    "filename": "kernel.c",
    "sha256": "4f92a34eb..."
  },
  "fingerprint": {
    "cfg_topology_hash": "0x8fa372eb001a",
    "abi_lowering_hash": "0xd320be44ac81",
    "stack_geometry_hash": "0x2849de748b0a",
    "regalloc_sequence_hash": "0xefcc7a88998b",
    "pass_pipeline_hash": "0x0db56e7f8fa3"
  },
  "pass_records": [
    {
      "pass": "constant_folding",
      "nodes_folded": 12,
      "dce_eliminated": 4
    },
    {
      "pass": "regalloc",
      "allocation_map": [
        {"vreg": 1, "preg": "rdi", "spilled": false},
        {"vreg": 2, "preg": "rsi", "spilled": false},
        {"vreg": 3, "preg": "stack", "spill_offset": -8}
      ]
    }
  ],
  "final_binary": {
    "size_bytes": 10240,
    "sha256": "8f92b7405e3692a188f6c4a30a1122efcc7a88998b417c8d9e6e8e2c0db56e7f"
  }
}
```

---

## 🔱 5. THE ROADMAP TO CIVILIZATION-SCALE VERIFIABILITY

```text
  [Self-Host Compiler]
           │
           ▼
  [Deterministic Compiler] (Phase 19 Sealed)
           │
           ▼
  [Invariant-Proving Substrate] (Oracle Registry & --oracle-stack)
           │
           ▼
  [Replayable Compilation Records] (.zxr Emission & Time-Travel Replay)
           │
           ▼
  [Sovereign ELF Object Toolchain] (zld Sovereign Linker integration)
           │
           ▼
  [Verifiable Infrastructure Substrate] (Sovereign OS + AI Attestation)
```

---

*🔱 ZKAEDI COMPILER FORGE — ZCC Oracle Substrate & Execution Record Specs*
*Establishing mathematically provable domains inside sovereign toolchains.*
