/* Copyright 2026 Chris Kearney
 * SPDX-License-Identifier: MIT */
#define GL_GLEXT_PROTOTYPES
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES3/gl3.h>

#define W 64
#define H 64
#define CHECK(x)                                                               \
   do {                                                                        \
      if (!(x)) {                                                              \
         fprintf(stderr, "line=%d GL=%x\n", __LINE__, glGetError());           \
         return 2;                                                             \
      }                                                                        \
   } while (0)

static unsigned
check_target(GLuint source, GLuint resolved, unsigned rt, bool partial)
{
   glBindFramebuffer(GL_READ_FRAMEBUFFER, source);
   glReadBuffer(GL_COLOR_ATTACHMENT0 + rt);
   glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolved);
   glBlitFramebuffer(0, 0, W, H, 0, 0, W, H, GL_COLOR_BUFFER_BIT, GL_NEAREST);
   glBindFramebuffer(GL_READ_FRAMEBUFFER, resolved);
   glReadBuffer(GL_COLOR_ATTACHMENT0);
   unsigned char pixels[W * H * 4];
   glReadPixels(0, 0, W, H, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
   unsigned bad = 0;
   for (unsigned y = 0; y < H; ++y) {
      for (unsigned x = 0; x < W; ++x) {
         const bool blue = partial && rt == 0 && x < W / 2;
         const unsigned char expected[4] = {
            rt == 0 && !blue ? 255 : 0,
            rt == 1 ? 255 : 0,
            blue ? 255 : 0,
            255,
         };
         bool different = false;
         for (unsigned c = 0; c < 4; ++c)
            different |= pixels[(y * W + x) * 4 + c] != expected[c];
         bad += different;
      }
   }
   const GLenum error = glGetError();
   printf("case=%s rt=%u bad=%u GL=%x\n",
          partial ? "reload-partial-clear" : "clear-store", rt, bad, error);
   return bad != 0 || error != GL_NO_ERROR;
}

int
main(int argc, char **argv)
{
   CHECK(argc == 2);
   const unsigned samples = strtoul(argv[1], NULL, 10);
   CHECK(samples == 1 || samples == 2 || samples == 4);
   setvbuf(stdout, NULL, _IONBF, 0);
   EGLDisplay d =
      eglGetPlatformDisplay(EGL_PLATFORM_SURFACELESS_MESA, NULL, NULL);
   CHECK(eglInitialize(d, NULL, NULL));
   CHECK(eglBindAPI(EGL_OPENGL_ES_API));
   EGLint attrs[] = {EGL_CONTEXT_MAJOR_VERSION, 3, EGL_NONE};
   EGLContext c = eglCreateContext(d, EGL_NO_CONFIG_KHR, EGL_NO_CONTEXT, attrs);
   CHECK(c != EGL_NO_CONTEXT);
   CHECK(eglMakeCurrent(d, EGL_NO_SURFACE, EGL_NO_SURFACE, c));
   GLint max_rt, max_samples;
   glGetIntegerv(GL_MAX_DRAW_BUFFERS, &max_rt);
   glGetIntegerv(GL_MAX_SAMPLES, &max_samples);
   CHECK(max_rt >= 2 && max_samples >= (int)samples);
   printf("renderer=%s maxrt=%d maxsamples=%d samples=%u\n",
          glGetString(GL_RENDERER), max_rt, max_samples, samples);

   GLuint source, rb[2], resolved, texture;
   glGenFramebuffers(1, &source);
   glBindFramebuffer(GL_FRAMEBUFFER, source);
   glGenRenderbuffers(2, rb);
   for (unsigned i = 0; i < 2; ++i) {
      glBindRenderbuffer(GL_RENDERBUFFER, rb[i]);
      glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, GL_RGBA8, W,
                                       H);
      glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i,
                                GL_RENDERBUFFER, rb[i]);
   }
   GLenum bufs[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
   glDrawBuffers(2, bufs);
   CHECK(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
   const float red[] = {1, 0, 0, 1}, green[] = {0, 1, 0, 1};
   glClearBufferfv(GL_COLOR, 0, red);
   glClearBufferfv(GL_COLOR, 1, green);
   CHECK(glGetError() == GL_NO_ERROR);
   puts("clear=accepted flush=next");
   glFinish();
   CHECK(glGetError() == GL_NO_ERROR);
   puts("flush=completed");

   glGenTextures(1, &texture);
   glBindTexture(GL_TEXTURE_2D, texture);
   glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, W, H);
   glGenFramebuffers(1, &resolved);
   glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolved);
   glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                          GL_TEXTURE_2D, texture, 0);
   CHECK(glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER) ==
         GL_FRAMEBUFFER_COMPLETE);
   unsigned failures = check_target(source, resolved, 0, false) +
                       check_target(source, resolved, 1, false);

   /* Resolving an attachment must retain its samples. Reload both targets and
    * clear only half of RT0, preserving RT1 and the other half of RT0. */
   glBindFramebuffer(GL_FRAMEBUFFER, source);
   glEnable(GL_SCISSOR_TEST);
   glScissor(0, 0, W / 2, H);
   const float blue[] = {0, 0, 1, 1};
   glClearBufferfv(GL_COLOR, 0, blue);
   glDisable(GL_SCISSOR_TEST);
   CHECK(glGetError() == GL_NO_ERROR);
   glFinish();
   CHECK(glGetError() == GL_NO_ERROR);
   failures += check_target(source, resolved, 0, true) +
               check_target(source, resolved, 1, true);
   printf("samples=%u failures=%u\n", samples, failures);
   return failures ? 1 : 0;
}
