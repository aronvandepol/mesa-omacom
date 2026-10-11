/* Copyright 2026 Chris Kearney
 * SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "compiler/agx_builder.h"
#include "compiler/agx_compiler.h"
#define CHECK(condition, message) do { if (!(condition)) { fprintf(stderr, "%s\n", message); return 1; } } while (0)

static int
check_mediump_pack(void)
{
   for (unsigned signed_norm = 0; signed_norm < 2; ++signed_norm) {
      void *mem = ralloc_context(NULL);
      agx_context *ctx = rzalloc(mem, agx_context);
      struct agx_shader_key key = {0};
      key.dev.encoding = AGX_ISA_ENCODING_AGX3B;
      ctx->key = &key;
      list_inithead(&ctx->blocks);
      agx_block *blk = rzalloc(ctx, agx_block);
      blk->predecessors = UTIL_DYNARRAY_INIT;
      ctx->num_blocks = 1;
      list_addtail(&blk->link, &ctx->blocks);
      list_inithead(&blk->instructions);
      agx_builder b = {.shader = ctx, .cursor = agx_after_block(blk)};
      agx_index wide[2];
      for (unsigned i = 0; i < 2; ++i) {
         agx_index half = agx_mov(&b, agx_register(i + 1, AGX_SIZE_16));
         wide[i] = agx_temp(ctx, AGX_SIZE_32);
         agx_fmov_to(&b, wide[i], half);
      }
      agx_index out = agx_temp(ctx, AGX_SIZE_16);
      agx_instr *pack = agx_fltnorm_agx3_to(&b, out, wide[0], wide[1],
         signed_norm ? AGX_FORMAT_S8NORM : AGX_FORMAT_U8NORM);
      agx_unit_test(&b, out);
      agx_optimizer_backward(ctx);
      agx_optimizer_forward(ctx);
      agx_dce(ctx, true);
      CHECK(pack->src[0].size == AGX_SIZE_32 &&
            pack->src[1].size == AGX_SIZE_32,
            "optimizer discarded normalized-color widening");
      ralloc_free(mem);
   }
   puts("mediump-normalized-optimizer=pass unorm8+snorm8");
   return 0;
}

int main(void) { return check_mediump_pack(); }
