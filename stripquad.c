// Draw a full-viewport quad several ways on a surfaceless EGL/GLES2 context,
// read back the four corner regions. A dropped triangle shows as a black corner.
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES3/gl3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static GLuint prog(void) {
  const char *vs = "attribute vec2 p; void main(){ gl_Position = vec4(p,0.0,1.0); }";
  const char *fs = "precision mediump float; void main(){ gl_FragColor = vec4(1.0,1.0,1.0,1.0); }";
  GLuint v = glCreateShader(GL_VERTEX_SHADER), f = glCreateShader(GL_FRAGMENT_SHADER), p = glCreateProgram();
  glShaderSource(v, 1, &vs, 0); glCompileShader(v);
  glShaderSource(f, 1, &fs, 0); glCompileShader(f);
  glAttachShader(p, v); glAttachShader(p, f); glBindAttribLocation(p, 0, "p"); glLinkProgram(p);
  return p;
}

static void corners(const char *label, int W, int H) {
  unsigned char px[4];
  int pts[4][2] = {{W/8, H/8}, {W*7/8, H/8}, {W/8, H*7/8}, {W*7/8, H*7/8}};
  const char *names[4] = {"BL", "BR", "TL", "TR"};
  printf("%-28s", label);
  for (int i = 0; i < 4; i++) {
    glReadPixels(pts[i][0], pts[i][1], 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, px);
    printf(" %s=%s", names[i], px[0] > 128 ? "ok" : "MISSING");
  }
  printf("\n");
}

int main(void) {
  EGLDisplay d = eglGetPlatformDisplay(EGL_PLATFORM_SURFACELESS_MESA, EGL_DEFAULT_DISPLAY, NULL);
  if (!eglInitialize(d, 0, 0)) { fprintf(stderr, "eglInitialize failed\n"); return 1; }
  eglBindAPI(EGL_OPENGL_ES_API);
  EGLint ca[] = {EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT, EGL_NONE}, n; EGLConfig c;
  eglChooseConfig(d, ca, &c, 1, &n);
  EGLint cx[] = {EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE};
  EGLContext ctx = eglCreateContext(d, n ? c : 0, EGL_NO_CONTEXT, cx);
  if (!eglMakeCurrent(d, EGL_NO_SURFACE, EGL_NO_SURFACE, ctx)) { fprintf(stderr, "makecurrent failed\n"); return 1; }
  printf("GL_RENDERER: %s\n", glGetString(GL_RENDERER));
  int W = 256, H = 256;
  GLuint fbo, tex;
  glGenTextures(1, &tex); glBindTexture(GL_TEXTURE_2D, tex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, W, H, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
  glGenFramebuffers(1, &fbo); glBindFramebuffer(GL_FRAMEBUFFER, fbo);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);
  glViewport(0, 0, W, H);
  GLuint p = prog(); glUseProgram(p); glEnableVertexAttribArray(0);

  float strip[] = {-1,-1, 1,-1, -1,1, 1,1};
  float tris[]  = {-1,-1, 1,-1, -1,1,  -1,1, 1,-1, 1,1};
  unsigned short idx_strip[] = {0,1,2,3};
  unsigned short idx_tris[] = {0,1,2, 2,1,3};
  GLuint vbo, ibo; glGenBuffers(1, &vbo); glGenBuffers(1, &ibo);

#define CLEAR() do { glClearColor(0,0,0,1); glClear(GL_COLOR_BUFFER_BIT); } while (0)
  // client-side arrays
  CLEAR(); glBindBuffer(GL_ARRAY_BUFFER, 0); glVertexAttribPointer(0,2,GL_FLOAT,0,0,strip);
  glDrawArrays(GL_TRIANGLE_STRIP,0,4); corners("strip arrays (client)", W, H);
  CLEAR(); glVertexAttribPointer(0,2,GL_FLOAT,0,0,tris);
  glDrawArrays(GL_TRIANGLES,0,6); corners("triangles arrays (client)", W, H);
  // VBO
  glBindBuffer(GL_ARRAY_BUFFER, vbo); glBufferData(GL_ARRAY_BUFFER, sizeof strip, strip, GL_STATIC_DRAW);
  CLEAR(); glVertexAttribPointer(0,2,GL_FLOAT,0,0,0);
  glDrawArrays(GL_TRIANGLE_STRIP,0,4); corners("strip arrays (vbo)", W, H);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof idx_strip, idx_strip, GL_STATIC_DRAW);
  CLEAR(); glDrawElements(GL_TRIANGLE_STRIP,4,GL_UNSIGNED_SHORT,0); corners("strip elements u16 (vbo)", W, H);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof idx_tris, idx_tris, GL_STATIC_DRAW);
  CLEAR(); glDrawElements(GL_TRIANGLES,6,GL_UNSIGNED_SHORT,0); corners("triangles elements u16 (vbo)", W, H);
  // Qt Quick style: interleaved x,y,u,v floats, stride 16
  float qt[] = {-1,-1,0,0, -1,1,0,1, 1,-1,1,0, 1,1,1,1};
  glBindBuffer(GL_ARRAY_BUFFER, vbo); glBufferData(GL_ARRAY_BUFFER, sizeof qt, qt, GL_STATIC_DRAW);
  glVertexAttribPointer(0,2,GL_FLOAT,0,16,0);
  CLEAR(); glDrawArrays(GL_TRIANGLE_STRIP,0,4); corners("strip interleaved stride16", W, H);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof idx_strip, idx_strip, GL_STATIC_DRAW);
  CLEAR(); glDrawElements(GL_TRIANGLE_STRIP,4,GL_UNSIGNED_SHORT,0); corners("strip interleaved elements", W, H);
  // Qt Quick's RHI GL path: u16 strip indices at a 2-byte offset in the index buffer,
  // GL_PRIMITIVE_RESTART_FIXED_INDEX enabled.
  glBindBuffer(GL_ARRAY_BUFFER, vbo); glBufferData(GL_ARRAY_BUFFER, sizeof strip, strip, GL_STATIC_DRAW);
  glVertexAttribPointer(0,2,GL_FLOAT,0,0,0);
  unsigned short idx_off[] = {0xbeef, 0,1,2,3, 0xbeef};
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof idx_off, idx_off, GL_STATIC_DRAW);
  for (int restart = 0; restart < 2; restart++) {
    if (restart) glEnable(GL_PRIMITIVE_RESTART_FIXED_INDEX); else glDisable(GL_PRIMITIVE_RESTART_FIXED_INDEX);
    char l[64];
    snprintf(l, sizeof l, "strip u16 off0 restart=%d", restart);
    unsigned short idx0[] = {0,1,2,3};
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof idx0, idx0, GL_STATIC_DRAW);
    CLEAR(); glDrawElements(GL_TRIANGLE_STRIP,4,GL_UNSIGNED_SHORT,(void*)0); corners(l, W, H);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof idx_off, idx_off, GL_STATIC_DRAW);
    snprintf(l, sizeof l, "strip u16 off2 restart=%d", restart);
    CLEAR(); glDrawElements(GL_TRIANGLE_STRIP,4,GL_UNSIGNED_SHORT,(void*)2); corners(l, W, H);
    snprintf(l, sizeof l, "triangles u16 off2 restart=%d", restart);
    unsigned short idx_t[] = {0xbeef, 0,1,2, 2,1,3};
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof idx_t, idx_t, GL_STATIC_DRAW);
    CLEAR(); glDrawElements(GL_TRIANGLES,6,GL_UNSIGNED_SHORT,(void*)2); corners(l, W, H);
    unsigned short idx_4[] = {0xbeef, 0xbeef, 0,1,2,3};
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof idx_4, idx_4, GL_STATIC_DRAW);
    snprintf(l, sizeof l, "strip u16 off4 restart=%d", restart);
    CLEAR(); glDrawElements(GL_TRIANGLE_STRIP,4,GL_UNSIGNED_SHORT,(void*)4); corners(l, W, H);
  }
  return 0;
}
