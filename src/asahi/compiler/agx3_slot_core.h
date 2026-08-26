/*
 * Copyright 2026 Aurora Silicon
 * SPDX-License-Identifier: MIT
 * 
 * AGX3 scoreboard assignment. Asynchronous loads, samples, atomics and fills
 * signal slots. Consumers wait for those slots before use. A satisfied slot
 * remains satisfied until it is reallocated; ALU dependencies are interlocked.
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Normal register dependencies occupy tokens 0..255.  Static Mesa stack
 * offsets are 16-bit byte immediates; their 32768 addressable half-registers
 * occupy a separate token range so async copy/rld ordering is exact. */
#define AGX3_SLOT_GPR_TOKENS 256u
#define AGX3_SLOT_TPS_TOKEN_BASE AGX3_SLOT_GPR_TOKENS
#define AGX3_SLOT_TPS_HALF_TOKENS 32768u
#define AGX3_SLOT_NUM_TOKENS                                             \
   (AGX3_SLOT_TPS_TOKEN_BASE + AGX3_SLOT_TPS_HALF_TOKENS)

/* Up to 16 half-register or storage-token sources/dests per instruction is far
 * more than any AGX3 form uses. */
#define AGX3_SLOT_MAX_REGS 16u

struct agx3_slot_instr {
   bool is_async;   /* produces a scoreboard-tracked result */
   uint8_t nr_dsts; /* dependency tokens this instruction writes */
   uint8_t nr_srcs; /* dependency tokens this instruction reads */
   uint16_t dst[AGX3_SLOT_MAX_REGS];
   uint16_t src[AGX3_SLOT_MAX_REGS];
};

struct agx3_slot_result {
   uint8_t slot;       /* 1..nslots for async instructions, 0 otherwise */
   uint16_t wait_mask; /* bit (s-1) set => wait scoreboard slot s */
};

/*
 * Assign slots and waits for one straight-line instruction region, which may
 * span fallthrough basic blocks. `n` is the instruction count; `nslots` is 6
 * (agx3b) or 8 (agx3c). Writes one result per instruction. Returns false on a
 * malformed input (too many regs, nslots out of range, null pointers).
 *
 * GPR tokens are half-register indices; static TPS tokens use the disjoint
 * range above. A value that is not currently the live result of an async op
 * contributes no wait. The slot policy is round-robin, which the RE showed is
 * always correct (reuse only forces an earlier wait, never misses a hazard).
 */
bool agx3_slot_assign(const struct agx3_slot_instr *instrs, unsigned n,
                      unsigned nslots, struct agx3_slot_result *out);

#ifdef __cplusplus
}
#endif
