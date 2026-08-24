/*
 * Copyright 2026 Aurora Silicon
 * SPDX-License-Identifier: MIT
 * 
 * AGX3 instruction selection maps normalized IR operands to encoder fields.
 * Unsupported operations, operand sizes and modifiers are rejected rather
 * than emitted with an incompatible encoding.
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "asahi/lib/agx_chip.h"

#ifdef __cplusplus
extern "C" {
#endif

/* */
enum agx3_sel_op {
   AGX3_SEL_OP_UNSUPPORTED = 0,
   AGX3_SEL_OP_END,       /* IR stop / shader terminator */
   AGX3_SEL_OP_BARRIER,   /* IR threadgroup_barrier */
   AGX3_SEL_OP_MOV_IMM,   /* IR mov_imm */
   AGX3_SEL_OP_FADD,      /* IR fadd (f32) */
   AGX3_SEL_OP_FMUL,      /* IR fmul (f32) */
   AGX3_SEL_OP_FFMA,      /* IR ffma (f32) */
   AGX3_SEL_OP_IADD,      /* IR iadd (u32), shift == 0 */
   AGX3_SEL_OP_IMAD,      /* IR imad (u32), shift == 0 */
   AGX3_SEL_OP_ASR,       /* IR asr (i32) */
   AGX3_SEL_OP_BITOP,     /* IR bitop; truth_table selects and/or/xor/inv */
};

/* */
struct agx3_sel_src {
   uint16_t reg;
   bool is_reg;
   bool abs;
   bool neg;
   uint8_t size; /* AGX size: 0 = 16-bit, 1 = 32-bit, 2 = 64-bit */
};

struct agx3_sel_instr {
   enum agx3_sel_op op;
   uint8_t nr_srcs;
   bool has_dest;
   uint16_t dest;
   uint8_t dest_size; /* AGX size */
   struct agx3_sel_src src[3];
   uint32_t imm;      /* mov_imm immediate */
   uint32_t truth_table; /* bitop 4-bit truth table (UTIL_LUT2 convention) */
   uint8_t shift;     /* */
   bool saturate;
};

/* */
#define AGX3_SEL_TT_AND 0x8u
#define AGX3_SEL_TT_OR  0xEu
#define AGX3_SEL_TT_XOR 0x6u
#define AGX3_SEL_TT_NOT 0x5u /* ~a, independent of b */

/* */
#define AGX3_SEL_SIZE_32 1u

/* */
#define AGX3_SEL_MAX_LENGTH 16u

/*
 * Encode one selected instruction. On success writes `*len` bytes (<=
 * AGX3_SEL_MAX_LENGTH) into `out` and returns true. Returns false -- writing
 * nothing meaningful -- for an unknown encoding, an unmapped op, or any
 * instruction outside the grounded domain. A false return must fail the
 * shader; it must never be treated as an empty instruction.
 */
bool agx3_select_encode(enum agx_isa_encoding encoding,
                        const struct agx3_sel_instr *instr,
                        uint8_t out[AGX3_SEL_MAX_LENGTH], uint8_t *len);

/* */
bool agx3_select_op_supported(enum agx3_sel_op op);

#ifdef __cplusplus
}
#endif
