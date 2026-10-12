/* SPDX-License-Identifier: MIT */
#pragma once

#include <stdbool.h>
#include <vulkan/vulkan_core.h>
#include "asahi/lib/agx_chip.h"

static inline bool
hk_g15s_split_depth_plane(enum agx_chip chip, unsigned hal, VkFormat format,
                         unsigned plane, unsigned samples, bool enabled)
{
   return enabled && chip == AGX_CHIP_G15S && hal == 0 && plane == 0 &&
          samples == 1 && format == VK_FORMAT_D32_SFLOAT_S8_UINT;
}

static inline bool
hk_g15s_split_depth_shape(const VkImageCreateInfo *info)
{
   return info->imageType == VK_IMAGE_TYPE_2D &&
          info->tiling == VK_IMAGE_TILING_OPTIMAL && info->mipLevels == 1 &&
          info->arrayLayers == 1 && info->extent.depth == 1 && info->flags == 0 &&
          !(info->usage & (VK_IMAGE_USAGE_STORAGE_BIT |
                           VK_IMAGE_USAGE_HOST_TRANSFER_BIT_EXT |
                           VK_IMAGE_USAGE_ATTACHMENT_FEEDBACK_LOOP_BIT_EXT));
}
