#ifndef SHADERS_H
#define SHADERS_H

#include "gl_headers.h"
#include "image.h"
#include "matrix.h"

#include <map>


#define BINDING_ATTRIBUTES (int)-1
#define BINDING_UNIFORMS (int)-2
#define BINDING_DONE (int)-3
#define BINDING_INVALID (int)-4

typedef enum {
  ATTRIBUTE_VERTEX,
  ATTRIBUTE_OTHER0,
  ATTRIBUTE_NORMAL0,
  ATTRIBUTE_COLOR0,
  ATTRIBUTE_COLOR1,
  ATTRIBUTE_FOGCOORD0,
  ATTRIBUTE_OTHER1,
  ATTRIBUTE_TEXCOORD0,
  ATTRIBUTE_TEXCOORD1,
  ATTRIBUTE_TEXCOORD2,
  ATTRIBUTE_TANGENT0,
  ATTRIBUTE_MAX
} ProgramParameters;

typedef enum {
  UNIFORM_V,
  UNIFORM_MV,
  UNIFORM_MVP,
  UNIFORM_PERSPECTIVE,
  UNIFORM_COLOR0,
  UNIFORM_EYE,
  UNIFORM_TEX0,
  UNIFORM_TEX1,
  UNIFORM_TEX2,
  UNIFORM_TEX3,
  UNIFORM_TEX4,
  UNIFORM_TEX5,
  UNIFORM_TEX6,
  UNIFORM_LIGHT0_POSITION,
  UNIFORM_SCALE,
  UNIFORM_TEX_BL,
  UNIFORM_TEX_TR,
  UNIFORM_MAX
} ProgramUniforms;

class Program {
  Program();

 public:
  static Program* create(const char* vname, const char* fname, ...);

  ~Program();

  GLuint requireUniform(ProgramUniforms uniform);

  void use();
  void bind_attribute_buffer(ProgramParameters attr, unsigned element_length, GLuint buffer);

  void bind_uniform(Texture* tex, unsigned slot, ProgramUniforms uni);
  void bind_uniform(Texture* tex, ProgramUniforms uni);
  void bind_uniform(CubeMap* tex, unsigned slot, ProgramUniforms uni);
  void bind_uniform(CubeMap* tex, ProgramUniforms uni);
  void bind_uniform(const Vector& v, ProgramUniforms uni);
  void bind_uniform(const Matrix& m, ProgramUniforms uni);

  GLuint uniforms[UNIFORM_MAX];
  GLuint program;

  static GLuint current_program;
};

typedef Program* (*ProgramLoader)(void);
typedef std::map<ProgramLoader, Program*> LoaderToProgram;

Program* get_program(ProgramLoader loader);

#endif
