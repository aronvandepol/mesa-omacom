/*
 * Copyright 2026 Aurora Silicon
 * SPDX-License-Identifier: MIT
 * 
 * AGX3 tile writeout barriers map wait_pix to pixwait and signal_pix to
 * pixrel. The writeout masks select depth/coverage or color classes.
 * Unsupported masks are rejected.
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "asahi/lib/agx_chip.h"

#ifdef __cplusplus
extern "C" {
#endif

/* pixwait and pixrel are both six bytes on AGX3b and AGX3c. */
#define AGX3_PIXEL_MAX_LENGTH 6u

/*
 * Open G17P's colour-tile protocol. This is the f14=512, f15=1 pixwait form
 * that precedes the ordinary class-12 colour wait. It is byte-exactly
 * witnessed on AGX3b only; AGX3c and AGX2 are refused.
 */
bool agx3_pixel_colour_entry(enum agx_isa_encoding enc,
                             uint8_t out[AGX3_PIXEL_MAX_LENGTH], uint8_t *len);

/*
 * Wait for the writeout classes named by `writeout` before continuing.
 *
 * `writeout` is the AGX2 wait_pix immediate. Accepted values are exactly those
 * witnessed in Apple's fragment corpus: 1 (depth/coverage) and 4, 8, 12, 16,
 * 32, 48, 60, 64, 128, 192 (colour classes; the multi-RT shaders widen the
 * mask). Mesa emits 1, 4, 8 and 12.
 *
 * The shader-entry gate (`f14=512, f15=1`) is exposed separately through
 * agx3_pixel_colour_entry(): it is a prologue Apple's compiler inserts, not a
 * writeout-class wait described by Mesa IR.
 *
 * `wait_mask` is the scoreboard wait set, one bit per slot starting at slot 1
 * (six slots on AGX3b, eight on AGX3c), matching the tile and memory cores.
 */
bool agx3_pixel_wait(enum agx_isa_encoding enc, uint32_t writeout,
                     uint16_t wait_mask, uint8_t out[AGX3_PIXEL_MAX_LENGTH],
                     uint8_t *len);

/*
 * Release the writeout classes named by `writeout`.
 *
 * The corpus descriptor factors exactly as `(last ? 0x200 : 0) | writeout`
 * across all nine witnessed values, so `last` -- set on the release that ends
 * the shader's writeout sequence -- is a separate argument. Accepted
 * `writeout` values are the witnessed set {0, 1, 12, 48, 60, 192}; the
 * colour-class singletons that only ever appear on pixwait (4, 8, 16, ...) are
 * refused here because no pixrel witness carries them.
 */
bool agx3_pixel_release(enum agx_isa_encoding enc, uint32_t writeout, bool last,
                        uint16_t wait_mask, uint8_t out[AGX3_PIXEL_MAX_LENGTH],
                        uint8_t *len);

#ifdef __cplusplus
}
#endif
