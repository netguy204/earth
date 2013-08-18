#ifndef IMAGE_H
#define IMAGE_H

#include "stb_image.h"
#include "utils.h"

class Texture {
public:
  GLuint texture;
  bool bound;
  unsigned tunit;

  inline Texture(unsigned w, unsigned h, GLuint type, unsigned char* data)
    : bound(false), tunit(0) {

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    gl_check(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    gl_check(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
    gl_check(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    gl_check(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    gl_check(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0,
                          type, GL_UNSIGNED_BYTE, data));
    glBindTexture(GL_TEXTURE_2D, 0);
  }

  static inline Texture* from_file(const char* fname) {
    int w, h, c;
    unsigned char* data = stbi_load(fname, &w, &h, &c, 0);
    if(!data) fail_exit("failed to load %s", fname);
    GLuint kind;
    if(c == 3) {
      kind = GL_RGB;
    } else if(c == 4) {
      kind = GL_RGBA;
    } else {
      fail_exit("don't know how to handle an image with %d channels", c);
    }

    Texture* tex = new Texture(w, h, kind, data);
    free(data);
    return tex;
  }

  inline void bind(unsigned unit) {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, texture);
    tunit = unit;
    bound = true;
  }

  inline void unbind() {
    glActiveTexture(GL_TEXTURE0 + tunit);
    glBindTexture(GL_TEXTURE_2D, 0);
    bound = false;
  }
};


#endif
