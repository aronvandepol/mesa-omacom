/*
 * Copyright 2026 Aurora Silicon
 * SPDX-License-Identifier: MIT
 *
 * AGX3 transcendental and rounding instructions. The f43 rounding mode is
 * 0=rint, 1=trunc, 2=ceil, 3=floor. AGX3c supports native saturation; AGX3b
 * rejects saturating forms. Operands are dest=f2, src=f18, abs=f30, neg=f31.
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "asahi/lib/agx_chip.h"

#ifdef __cplusplus
extern "C" {
#endif

/* All funops are 10 bytes on both encodings. */
#define AGX3_FUNOP_MAX_LENGTH 10u

enum agx3_funop {
   AGX3_FUNOP_FLOOR,     /* f32rint mode 3 */
   AGX3_FUNOP_CEIL,      /* f32rint mode 2 */
   AGX3_FUNOP_TRUNC,     /* f32rint mode 1 */
   AGX3_FUNOP_RINT,      /* f32rint mode 0 (roundeven) */
   AGX3_FUNOP_LOG2,      /* f32log2 */
   AGX3_FUNOP_EXP2,      /* f32exp2 */
   AGX3_FUNOP_SINC,      /* f32sinc: encoding grounded; see agx3_pack_binary
                          * for why no Mesa opcode routes here yet. */
   AGX3_FUNOP_DFDX,      /* f32dfdx, fragment-only gradient */
   AGX3_FUNOP_DFDY,      /* f32dfdy, fragment-only gradient */
   AGX3_FUNOP_COUNT,
};

/* */
bool agx3_funop_encode(enum agx_isa_encoding enc, enum agx3_funop op,
                       uint16_t dest, uint16_t src, bool src_abs, bool src_neg,
                       bool saturate, uint16_t wait_mask,
                       uint8_t out[AGX3_FUNOP_MAX_LENGTH], uint8_t *len);

/* Runtime G16/G17P lowering preserves the allocator's actual cache lifetime. */
struct agx3_funop_runtime {
   bool dest_cache;
   bool src_cache;
   bool src_discard;
};

bool agx3_funop_encode_runtime(enum agx_isa_encoding enc, enum agx3_funop op,
                               uint16_t dest, uint16_t src, bool src_abs,
                               bool src_neg, bool saturate, uint16_t wait_mask,
                               const struct agx3_funop_runtime *runtime,
                               uint8_t out[AGX3_FUNOP_MAX_LENGTH], uint8_t *len);

#ifdef __cplusplus
}
#endif
