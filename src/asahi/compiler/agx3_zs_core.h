/*
 * Copyright 2026 Aurora Silicon
 * SPDX-License-Identifier: MIT
 * 
 * AGX3 coverage and depth/stencil output use one staged register operand.
 * f5 names its half-register index, f13 marks last use, and f12 selects the
 * scoreboard slot. f17 onward holds waits. depthf f25/f26 enable depth/stencil;
 * AGX3c f35 selects depth, stencil or both. Unsupported IR shapes are rejected.
 */



#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "asahi/lib/agx_chip.h"

#ifdef __cplusplus
extern "C" {
#endif

/* movmsk and depthf are 6 bytes on AGX3b and 8 on AGX3c. */
#define AGX3_ZS_MAX_LENGTH 8u

/*
 * The 32-bit word Apple's compiler stages for an unconditional kill of every
 * sample -- `discard_fragment()` with no surviving coverage, which is what
 * Mesa's `sample_mask(target = ALL_SAMPLES, live = 0)` means.
 *
 * Directly witnessed, not derived: three independent probes (an unconditional
 * `discard_fragment()`, the same with a void fragment function, and
 * `[[sample_mask]] = 0`) all compile to `ldimm r0, 0xFFFFFF00` immediately
 * followed by `movmsk r0`, on g15g/g16g/g17s alike, and the corpus
 * `discard_msaa4` shader stages the same constant for its first movmsk. No
 * other movmsk operand value is witnessed as a constant.
 */
#define AGX3_ZS_KILL_ALL_SAMPLES 0xFFFFFF00u

struct agx3_zs_desc {
   /* Half-register index of the staged writeout word. 32-bit, so even. */
   uint16_t value;
   /* The staged word is dead after this instruction. */
   bool value_kill;
   /* Scoreboard slot this instruction takes: 1..6 (AGX3b) / 1..8 (AGX3c). */
   uint8_t slot;
   /* Bit i waits scoreboard slot i+1. */
   uint16_t wait_mask;
};

/* `movmsk`: write the staged coverage word. */
bool agx3_zs_encode_sample_mask(enum agx_isa_encoding enc,
                                const struct agx3_zs_desc *d,
                                uint8_t out[AGX3_ZS_MAX_LENGTH], uint8_t *len);

/*
 * `depthf`: trigger the depth/stencil writeout for the staged word. At least
 * one of `depth`/`stencil` must be set; all three combinations are witnessed.
 */
bool agx3_zs_encode_depth_stencil(enum agx_isa_encoding enc,
                                  const struct agx3_zs_desc *d, bool depth,
                                  bool stencil,
                                  uint8_t out[AGX3_ZS_MAX_LENGTH],
                                  uint8_t *len);

#ifdef __cplusplus
}
#endif
