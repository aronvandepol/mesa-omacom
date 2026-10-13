/* SPDX-License-Identifier: MIT
 * Copyright 2026 Chris Kearney
 * Pixel checks for integer vertex coordinates and neighboring packed lanes.
 */
#define GL_GLEXT_PROTOTYPES
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES3/gl3.h>
#define CHECK(x)                                                               \
   do {                                                                        \
      if (!(x)) {                                                              \
         fprintf(stderr, "line %d GL=%x: %s\n", __LINE__, glGetError(), #x);   \
         exit(1);                                                              \
      }                                                                        \
   } while (0)
static GLuint
shader(GLenum type, const char *s)
{
   GLuint o = glCreateShader(type);
   glShaderSource(o, 1, &s, NULL);
   glCompileShader(o);
   GLint ok;
   glGetShaderiv(o, GL_COMPILE_STATUS, &ok);
   if (!ok) {
      char log[4096];
      glGetShaderInfoLog(o, sizeof(log), NULL, log);
      puts(log);
   }
   CHECK(ok);
   return o;
}
struct vert {
   float x, y;
   unsigned char color[4];
   union {
      uint16_t u[2];
      uint32_t u32[2];
      float f[2];
   } uv;
};
int
main(void)
{
   setbuf(stdout, NULL);
   EGLDisplay d =
      eglGetPlatformDisplay(EGL_PLATFORM_SURFACELESS_MESA, NULL, NULL);
   CHECK(eglInitialize(d, NULL, NULL));
   CHECK(eglBindAPI(EGL_OPENGL_ES_API));
   EGLint a[] = {EGL_CONTEXT_MAJOR_VERSION, 3, EGL_CONTEXT_MINOR_VERSION, 0,
                 EGL_NONE};
   EGLContext ctx = eglCreateContext(d, EGL_NO_CONFIG_KHR, EGL_NO_CONTEXT, a);
   CHECK(ctx != EGL_NO_CONTEXT);
   CHECK(eglMakeCurrent(d, EGL_NO_SURFACE, EGL_NO_SURFACE, ctx));
   printf("renderer=%s\n", glGetString(GL_RENDERER));
   GLuint f, target, vao, vbo, ibo;
   glGenFramebuffers(1, &f);
   glBindFramebuffer(GL_FRAMEBUFFER, f);
   glGenTextures(1, &target);
   glBindTexture(GL_TEXTURE_2D, target);
   glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, 128, 128);
   glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                          target, 0);
   CHECK(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
   glGenVertexArrays(1, &vao);
   glBindVertexArray(vao);
   glGenBuffers(1, &vbo);
   glGenBuffers(1, &ibo);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
   uint16_t indices[] = {0, 1, 2, 2, 1, 3};
   glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices,
                GL_STATIC_DRAW);
   glViewport(0, 0, 128, 128);
   unsigned char mask[512 * 128];
   memset(mask, 0, sizeof(mask));
   for (int y = 0; y < 128; y++)
      for (int x = 0; x < 64; x++)
         mask[y * 512 + x] = 255;
   int failed = 0;
   for (int mode = 0; mode < 3; mode++) {
      const int integer = mode != 0;
      const int np = 0;
      const int bias = 0;
      const int size = 128;
      char vs[2048], fs[2048];
      const char *ext =
         np ? "#extension GL_NV_shader_noperspective_interpolation : require\n"
            : "";
      const char *qual = np ? "noperspective " : "";
      snprintf(
         vs, sizeof(vs),
         "#version 300 es\n%sprecision mediump float;layout(location=0) in highp vec2 pos;layout(location=1) in mediump vec4 color;layout(location=2) in %s uv;uniform highp float invSize;%sout highp vec2 texcoord;%sout mediump vec4 vcolor;void main(){gl_Position=vec4(pos,0.,1.);texcoord=vec2(uv)*invSize;vcolor=color;}",
         ext, integer ? "mediump uvec2" : "highp vec2", qual, qual);
      snprintf(
         fs, sizeof(fs),
         "#version 300 es\n%sprecision mediump float;precision mediump sampler2D;uniform sampler2D tex;%sin highp vec2 texcoord;%sin mediump vec4 vcolor;out mediump vec4 outColor;void main(){outColor=vcolor*texture(tex,texcoord%s).xxxx;}",
         ext, qual, qual, bias ? ",-0.475" : "");
      GLuint p = glCreateProgram();
      glAttachShader(p, shader(GL_VERTEX_SHADER, vs));
      glAttachShader(p, shader(GL_FRAGMENT_SHADER, fs));
      glLinkProgram(p);
      GLint ok;
      glGetProgramiv(p, GL_LINK_STATUS, &ok);
      CHECK(ok);
      glUseProgram(p);
      glUniform1i(glGetUniformLocation(p, "tex"), 0);
      glUniform1f(glGetUniformLocation(p, "invSize"), 1.f / size);
      struct vert v[4] = {0};
      for (int i = 0; i < 4; i++) {
         v[i].x = (i & 1) ? 1 : -1;
         v[i].y = (i & 2) ? 1 : -1;
         v[i].color[3] = 255;
         if (mode == 2) {
            v[i].uv.u32[0] = (i & 1) ? 128 : 0;
            v[i].uv.u32[1] = (i & 2) ? 128 : 0;
         } else if (integer) {
            v[i].uv.u[0] = (i & 1) ? 128 : 0;
            v[i].uv.u[1] = (i & 2) ? 128 : 0;
         } else {
            v[i].uv.f[0] = (i & 1) ? 128 : 0;
            v[i].uv.f[1] = (i & 2) ? 128 : 0;
         }
      }
      glBindBuffer(GL_ARRAY_BUFFER, vbo);
      glBufferData(GL_ARRAY_BUFFER, sizeof(v), v, GL_STREAM_DRAW);
      glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(struct vert),
                            (void *)offsetof(struct vert, x));
      glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE,
                            sizeof(struct vert),
                            (void *)offsetof(struct vert, color));
      if (integer)
         glVertexAttribIPointer(
            2, 2, mode == 2 ? GL_UNSIGNED_INT : GL_UNSIGNED_SHORT,
            sizeof(struct vert), (void *)offsetof(struct vert, uv));
      else
         glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(struct vert),
                               (void *)offsetof(struct vert, uv));
      for (int i = 0; i < 3; i++)
         glEnableVertexAttribArray(i);
      GLuint tex;
      glGenTextures(1, &tex);
      glBindTexture(GL_TEXTURE_2D, tex);
      glTexStorage2D(GL_TEXTURE_2D, 1, GL_R8, size, size);
      glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
      glPixelStorei(GL_UNPACK_ROW_LENGTH, 512);
      glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 128, 128, GL_RED,
                      GL_UNSIGNED_BYTE, mask);
      glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
      glClearColor(1, 1, 1, 1);
      glClear(GL_COLOR_BUFFER_BIT);
      glEnable(GL_BLEND);
      glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ONE,
                          GL_ONE_MINUS_SRC_ALPHA);
      glDrawRangeElements(GL_TRIANGLES, 0, 3, 6, GL_UNSIGNED_SHORT, NULL);
      glFinish();
      CHECK(glGetError() == GL_NO_ERROR);
      unsigned char px[128 * 128 * 4];
      glReadPixels(0, 0, 128, 128, GL_RGBA, GL_UNSIGNED_BYTE, px);
      CHECK(glGetError() == GL_NO_ERROR);
      unsigned bad = 0;
      for (int y = 0; y < 128; y++)
         for (int x = 0; x < 128; x++) {
            unsigned i = y * 128 + x;
            unsigned char c = x < 64 ? 0 : 255;
            bad += px[4 * i] != c || px[4 * i + 1] != c || px[4 * i + 2] != c ||
                   px[4 * i + 3] != 255;
         }
      printf(
         "mode=%d integer_uv=%d noperspective=%d bias=%d atlas=%d bad=%u left=%u,%u,%u,%u right=%u,%u,%u,%u\n",
         mode, integer, np, bias, size, bad, px[0], px[1], px[2], px[3],
         px[400], px[401], px[402], px[403]);
      failed += bad != 0;
      glDeleteProgram(p);
      glDeleteTextures(1, &tex);
   }
   eglMakeCurrent(d, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
   eglDestroyContext(d, ctx);
   eglTerminate(d);
   return failed;
}
