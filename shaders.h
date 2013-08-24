#ifndef SHADERS_H
#define SHADERS_H

#include "gl_headers.h"

#include <map>


// opengl error checking
#define GL_CHECK_ERRORS

void gl_check_(const char * msg);

#define STRINGIZE(x) STRINGIZE2(x)
#define STRINGIZE2(x) #x

#ifdef GL_CHECK_ERRORS
#define gl_check(command) command; gl_check_(__FILE__ ": " STRINGIZE(__LINE__) " " #command)
#else
#define gl_check(command) command
#endif


typedef enum {
  GLPARAM_VERTEX,
  GLPARAM_OTHER0,
  GLPARAM_NORMAL0,
  GLPARAM_COLOR0,
  GLPARAM_COLOR1,
  GLPARAM_FOGCOORD0,
  GLPARAM_OTHER1,
  GLPARAM_TEXCOORD0,
  GLPARAM_TEXCOORD1,
  GLPARAM_TEXCOORD2,
  GLPARAM_TANGENT0,
  GLPARAM_DONE
} ProgramParameters;

typedef enum {
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
  UNIFORM_DONE
} ProgramUniforms;

class Program {
  Program();

 public:
  static Program* create(const char* vname, const char* fname, ...);

  ~Program();

  GLuint requireUniform(ProgramUniforms uniform);

  void use();

  GLuint uniforms[UNIFORM_DONE];
  GLuint program;

  static GLuint current_program;
};

typedef Program* (*ProgramLoader)(void);
typedef std::map<ProgramLoader, Program*> LoaderToProgram;

Program* renderer_link_shader(const char* vertexname, const char* fragmentname, ...);

void program_bind_uniforms(Program* p, ...);

Program* get_program(ProgramLoader loader);

#endif
