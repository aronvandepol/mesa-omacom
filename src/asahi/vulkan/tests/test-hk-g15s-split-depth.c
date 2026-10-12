/* SPDX-License-Identifier: MIT */
#include <assert.h>
#include "hk_g15s_split_depth.h"

int main(void)
{
   assert(hk_g15s_split_depth_plane(AGX_CHIP_G15S, 0,
                                  VK_FORMAT_D32_SFLOAT_S8_UINT, 0, 1, true));
   for (unsigned chip = AGX_CHIP_G13G; chip <= AGX_CHIP_UNKNOWN; ++chip) {
      if (chip != AGX_CHIP_G15S)
         assert(!hk_g15s_split_depth_plane(chip, 0,
                     VK_FORMAT_D32_SFLOAT_S8_UINT, 0, 1, true));
   }
   assert(!hk_g15s_split_depth_plane(AGX_CHIP_G15S, 200,
                                  VK_FORMAT_D32_SFLOAT_S8_UINT, 0, 1, true));
   assert(!hk_g15s_split_depth_plane(AGX_CHIP_G15S, 0,
                                  VK_FORMAT_D32_SFLOAT_S8_UINT, 1, 1, true));
   assert(!hk_g15s_split_depth_plane(AGX_CHIP_G15S, 0,
                                  VK_FORMAT_D32_SFLOAT_S8_UINT, 0, 2, true));
   assert(!hk_g15s_split_depth_plane(AGX_CHIP_G15S, 0,
                                  VK_FORMAT_D32_SFLOAT_S8_UINT, 0, 1, false));
   for (unsigned fmt = VK_FORMAT_UNDEFINED; fmt <= VK_FORMAT_ASTC_12x12_SRGB_BLOCK; ++fmt) {
      if (fmt != VK_FORMAT_D32_SFLOAT_S8_UINT)
         assert(!hk_g15s_split_depth_plane(AGX_CHIP_G15S, 0, fmt, 0, 1, true));
   }
   VkImageCreateInfo info = {
      .imageType = VK_IMAGE_TYPE_2D, .tiling = VK_IMAGE_TILING_OPTIMAL,
      .extent = {129, 127, 1}, .mipLevels = 1, .arrayLayers = 1,
      .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT |
               VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
   };
   assert(hk_g15s_split_depth_shape(&info));
   VkImageCreateInfo bad = info;
   bad.arrayLayers = 2; assert(!hk_g15s_split_depth_shape(&bad));
   bad = info; bad.mipLevels = 2; assert(!hk_g15s_split_depth_shape(&bad));
   bad = info; bad.extent.depth = 2; assert(!hk_g15s_split_depth_shape(&bad));
   bad = info; bad.imageType = VK_IMAGE_TYPE_3D; assert(!hk_g15s_split_depth_shape(&bad));
   bad = info; bad.tiling = VK_IMAGE_TILING_LINEAR; assert(!hk_g15s_split_depth_shape(&bad));
   bad = info; bad.flags = VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT; assert(!hk_g15s_split_depth_shape(&bad));
   VkImageUsageFlags blocked[] = {VK_IMAGE_USAGE_STORAGE_BIT,
      VK_IMAGE_USAGE_HOST_TRANSFER_BIT_EXT, VK_IMAGE_USAGE_ATTACHMENT_FEEDBACK_LOOP_BIT_EXT};
   for (unsigned i = 0; i < sizeof(blocked) / sizeof(blocked[0]); ++i) {
      bad = info; bad.usage |= blocked[i]; assert(!hk_g15s_split_depth_shape(&bad));
   }
   return 0;
}
