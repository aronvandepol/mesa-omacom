/*
 * Copyright 2026 Aurora Silicon
 * SPDX-License-Identifier: MIT
 * 
 * AGX3 interpolation: iter maps to f32interp, ldcf to ldcfr, and st_vary
 * to uvsw. Perspective interpolation uses RHW interpolation, reciprocal and
 * multiplication. Center and centroid locations are explicit.
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "asahi/lib/agx_chip.h"

#ifdef __cplusplus
extern "C" {
#endif

#define AGX3_VARY_MAX_LENGTH 10u

/* Interpolate coefficient `coef_index` into `dest`. `perspective` selects the
 * perspective-correct linear iterator (the caller pairs it with the software
 * rcp/mul sequence). Center location only for now. */
bool agx3_vary_interp(enum agx_isa_encoding enc, uint16_t dest,
                      uint16_t coef_index, bool perspective,
                      uint8_t out[AGX3_VARY_MAX_LENGTH], uint8_t *len);

/* Load the raw coefficient triple for `coef_index` into `dest`, on scoreboard
 * slot `slot` (1..6/1..8). */
bool agx3_vary_ldcf(enum agx_isa_encoding enc, uint16_t dest,
                    uint16_t coef_index, uint8_t slot,
                    uint8_t out[AGX3_VARY_MAX_LENGTH], uint8_t *len);

/* Write `value` to unified-vertex-store index `uvs_index`, on slot `slot`.
 * `value_kill` marks the last write of that register. `wait_mask` bit i waits
 * scoreboard slot i. */
bool agx3_vary_uvsw(enum agx_isa_encoding enc, uint16_t value,
                    uint16_t uvs_index, uint8_t slot, bool value_kill,
                    uint16_t wait_mask, uint8_t out[AGX3_VARY_MAX_LENGTH],
                    uint8_t *len);

#ifdef __cplusplus
}
#endif
