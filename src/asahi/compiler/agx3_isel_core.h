/*
 * Copyright 2026 Aurora Silicon
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "asahi/lib/agx_chip.h"

#ifdef __cplusplus
extern "C" {
#endif

#define AGX3_ISEL_MAX_LENGTH 10u

/* == enum agx_icond. Only the five Apple emits are encodable. */
enum agx3_isel_icond {
   AGX3_ISEL_ICOND_ULT = 1,
   AGX3_ISEL_ICOND_UGT = 2,
   AGX3_ISEL_ICOND_EQ = 4,
   AGX3_ISEL_ICOND_SLT = 5,
   AGX3_ISEL_ICOND_SGT = 6,
};

enum agx3_isel_operand {
   AGX3_ISEL_REGISTER = 0,
   AGX3_ISEL_IMMEDIATE = 1,
};

struct agx3_isel_desc {
   uint16_t dest;      /* half-register units */
   bool dest_16;       /* 16-bit result; both-immediates shape only */
   uint16_t cmp0;      /* 32-bit register half, compared first */
   uint16_t cmp1;      /* 32-bit register half, compared second */
   enum agx3_isel_icond cond;

   /* The selected operands: each a 32-bit register half or an 8-bit
    * immediate. sel_true is taken when (cmp0 cond cmp1) holds. */
   enum agx3_isel_operand true_kind;
   enum agx3_isel_operand false_kind;
   uint16_t sel_true;
   uint16_t sel_false;

   /* Bit i waits for scoreboard slot i + 1, as elsewhere in this backend. */
   uint16_t wait_mask;
};

bool agx3_isel_encode(enum agx_isa_encoding encoding,
                      const struct agx3_isel_desc *desc,
                      uint8_t out[AGX3_ISEL_MAX_LENGTH], uint8_t *len);

#ifdef __cplusplus
}
#endif
