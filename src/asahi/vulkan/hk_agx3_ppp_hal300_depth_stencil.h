/*
 * Copyright 2026 Aurora Silicon
 * SPDX-License-Identifier: MIT
 *
 * Honeykrisp bridge for the bounded HAL300 PPP depth/stencil block.
 */

#pragma once

#include <stdbool.h>
#include <stddef.h>

#include "asahi/lib/agx_ppp_hal300_depth_stencil_core.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * HAL300 depth/stencil field helpers. These do not allocate PPP storage,
 * emit VDM pointers, submit work or advertise device features.
 */

bool hk_agx3_encode_ppp_hal300_depth_stencil(
   enum agx_chip chip,
   const struct agx_ppp_hal300_depth_stencil_fields *fields,
   struct agx_ppp_hal300_depth_stencil_core *out);

bool hk_agx3_decode_ppp_hal300_depth_stencil(
   enum agx_chip chip, const void *payload, size_t size,
   struct agx_ppp_hal300_depth_stencil_fields *fields);

#ifdef __cplusplus
}
#endif
