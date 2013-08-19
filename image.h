#ifndef IMAGE_H
#define IMAGE_H

#include "stb_image.h"
#include "utils.h"

#include <stdlib.h>

class Image {
public:
  int w;
  int h;
  int ch;
  unsigned char* data;

  inline Image(unsigned w, unsigned h, unsigned ch) {
    size_t sz = w * h * ch;
    if(sz > 0) {
      data = (unsigned char*)malloc(sz);
    } else {
      data = NULL;
    }
  }

  inline ~Image() {
    if(data) free(data);
  }

  inline static Image* from_file(const char* fname) {
    Image* im = new Image(0, 0, 0);
    im->data = stbi_load(fname, &im->w, &im->h, &im->ch, 0);
    if(!im->data) fail_exit("failed to load %s", fname);
    return im;
  }

  inline unsigned char elm(unsigned x, unsigned y, unsigned c) const {
    return data[y * (w*ch) + (x*ch) + c];
  }

  inline unsigned char& elm(unsigned x, unsigned y, unsigned c) {
    return data[y * (w*ch) + (x*ch) + c];
  }
};

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
    gl_check(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0,
                          type, GL_UNSIGNED_BYTE, data));
    glBindTexture(GL_TEXTURE_2D, 0);
  }

  inline ~Texture() {
    glDeleteTextures(1, &texture);
  }

  static inline Texture* from_file(const char* fname) {
    Image* im = Image::from_file(fname);
    Texture* tex = Texture::from_image(im);
    delete im;
    return tex;
  }

  static inline Texture* from_image(Image* im) {
    GLuint kind;
    if(im->ch == 3) {
      kind = GL_RGB;
    } else if(im->ch == 4) {
      kind = GL_RGBA;
    } else {
      fail_exit("don't know how to handle an image with %d channels", im->ch);
    }

    return new Texture(im->w, im->h, kind, im->data);
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

class CubeMap {
public:
  GLuint texture;
  bool bound;
  unsigned tunit;

  inline CubeMap(unsigned w, unsigned h, GLuint type,
                 unsigned char* posx, unsigned char* negx,
                 unsigned char* posy, unsigned char* negy,
                 unsigned char* posz, unsigned char* negz) {

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
    gl_check(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    gl_check(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
    gl_check(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE));
    gl_check(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    gl_check(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGB, w, h, 0, type, GL_UNSIGNED_BYTE, posx);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGB, w, h, 0, type, GL_UNSIGNED_BYTE, negx);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGB, w, h, 0, type, GL_UNSIGNED_BYTE, posy);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGB, w, h, 0, type, GL_UNSIGNED_BYTE, negy);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGB, w, h, 0, type, GL_UNSIGNED_BYTE, posz);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGB, w, h, 0, type, GL_UNSIGNED_BYTE, negz);

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
  }

  inline ~CubeMap() {
    glDeleteTextures(1, &texture);
  }


  static inline CubeMap* from_files(const char* posx, const char* negx,
                                    const char* posy, const char* negy,
                                    const char* posz, const char* negz) {

    Image* iposx = Image::from_file(posx);
    Image* inegx = Image::from_file(negx);
    Image* iposy = Image::from_file(posy);
    Image* inegy = Image::from_file(negy);
    Image* iposz = Image::from_file(posz);
    Image* inegz = Image::from_file(negz);

    GLuint kind;
    if(iposx->ch == 3) {
      kind = GL_RGB;
    } else if(iposx->ch == 4) {
      kind = GL_RGBA;
    } else {
      fail_exit("don't know how to handle an image with %d channels", iposx->ch);
    }

    CubeMap* map = new CubeMap(iposx->w, iposx->h, kind, iposx->data, inegx->data,
                               iposy->data, inegy->data, iposz->data, inegz->data);

    delete iposx; delete inegx;
    delete iposy; delete inegy;
    delete iposz; delete inegz;
    return map;
  }

  inline void bind(unsigned unit) {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
    tunit = unit;
    bound = true;
  }

  inline void unbind() {
    glActiveTexture(GL_TEXTURE0 + tunit);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    bound = false;
  }
};

#endif
