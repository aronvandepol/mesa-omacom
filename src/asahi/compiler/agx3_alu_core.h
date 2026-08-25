/*
 * Copyright 2026 Aurora Silicon
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include <stdbool.h>
#include <stdint.h>
#include "asahi/lib/agx_chip.h"
#include "agx3_alu_table.h"
#ifdef __cplusplus
extern "C" {
#endif
#define AGX3_ALU_MAX_LENGTH 16u
/* Ops that alu_compose can carry a wait on (fmin/fmax's tied-operand fsel form
 * is not modeled here; the backend keeps those on the compact packer). */
bool agx3_alu_wait_supported(enum agx3_alu_op op);
bool agx3_alu_compose(enum agx_isa_encoding enc, enum agx3_alu_op op,
                      uint16_t dest, uint16_t src0, uint16_t src1,
                      uint16_t src2, uint32_t imm, uint16_t wait_mask,
                      uint8_t out[AGX3_ALU_MAX_LENGTH], uint8_t *len);
#ifdef __cplusplus
}
#endif
