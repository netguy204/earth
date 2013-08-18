#include "shaders.h"
#include "utils.h"

void gl_check_(const char * msg) {
  GLenum error = glGetError();
  if(error == GL_NO_ERROR) return;

  const char* e_msg;
  switch(error) {
  case GL_INVALID_ENUM:
    e_msg = "GL_INVALID_ENUM";
    break;
  case GL_INVALID_VALUE:
    e_msg = "GL_INVALID_VALUE";
    break;
  case GL_INVALID_OPERATION:
    e_msg = "GL_INVALID_OPERATION";
    break;
  case GL_OUT_OF_MEMORY:
    e_msg = "GL_OUT_OF_MEMORY";
    break;
  default:
    e_msg = "unknown";
  }

  LOGW("GL_ERROR: %s => %s\n", msg, e_msg);
}

char* shader_buffer = NULL;

int renderer_load_shader(const char* src, GLenum kind) {
  int shader = glCreateShader(kind);
  gl_check_("glCreateShader");

  const int max_shader = 8 * 1024;
  if(!shader_buffer) {
    shader_buffer = (char*)malloc(max_shader);
  }

#ifndef ANDROID
  snprintf(shader_buffer, max_shader, "#version 120\n%s", src);
#else
  snprintf(shader_buffer, max_shader, "%s", src);
#endif

  glShaderSource(shader, 1, (const char**)&shader_buffer, NULL);
  gl_check_("glShaderSource");

  glCompileShader(shader);
  gl_check_("glCompileShader");

  int status;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

  if(status == GL_FALSE) {
    char buffer[1024];
    int length;
    glGetShaderInfoLog(shader, sizeof(buffer), &length, buffer);
    fail_exit("glCompileShader: %s, %s\n", buffer, src);
  }

  return shader;
}

Program::Program() {
  program = -1;
  for(int ii = 0; ii < UNIFORM_DONE; ++ii) {
    uniforms[ii] = -1;
  }
}

Program::~Program() {
  if(program != -1) {
    glDeleteProgram(program);
  }
}

GLuint Program::requireUniform(ProgramUniforms uniform) {
  if(uniforms[uniform] == -1) {
    fail_exit("uniform %d is not defined", uniform);
  }
  return uniforms[uniform];
}

void Program::use() {
  if(program != current_program) {
    glUseProgram(program);
    current_program = program;
  }
}

Program* Program::create(const char* vertexname, const char* fragmentname, ...) {
  char* vertex_source = filename_slurp(vertexname);
  char* fragment_source = filename_slurp(fragmentname);

  LOGI("renderer_load_shader: %s", vertexname);
  int vertex = renderer_load_shader(vertex_source, GL_VERTEX_SHADER);
  LOGI("renderer_load_shader: %s", fragmentname);
  int fragment = renderer_load_shader(fragment_source, GL_FRAGMENT_SHADER);
  free(vertex_source);
  free(fragment_source);

  int program = glCreateProgram();

  gl_check(glAttachShader(program, vertex));
  gl_check(glAttachShader(program, fragment));

  va_list ap;
  va_start(ap, fragmentname);
  while(1) {
    ProgramParameters param = (ProgramParameters)va_arg(ap, int);
    if(param == GLPARAM_DONE) break;

    const char* name = va_arg(ap, char*);
    gl_check(glBindAttribLocation(program, param, name));
  }

  gl_check(glLinkProgram(program));

  gl_check(glDeleteShader(vertex));
  gl_check(glDeleteShader(fragment));
  int link_status;
  glGetProgramiv(program, GL_LINK_STATUS, &link_status);
  if(link_status == GL_FALSE) {
    char buffer[1024];
    int length;
    glGetProgramInfoLog(program, sizeof(buffer), &length, buffer);
    fail_exit("glLinkProgram: %s\n", buffer);
  }

  Program* p = new Program();
  p->program = program;
  return p;
}

void program_bind_uniforms(Program* p, ...) {
  va_list ap;
  va_start(ap, p);
  while(1) {
    ProgramUniforms uniform = (ProgramUniforms)va_arg(ap, int);
    if(uniform == UNIFORM_DONE) break;

    const char* name = va_arg(ap, char*);
    GLint loc = glGetUniformLocation(p->program, name);
    if(loc < 0) {
      fail_exit("glGetUniformLocation: %s error %d", name, loc);
    }

    p->uniforms[uniform] = loc;
  }
}


GLuint Program::current_program = 0;

LoaderToProgram programs;
Program* get_program(ProgramLoader loader) {
  LoaderToProgram::iterator iter = programs.find(loader);
  if(iter == programs.end()) {
    Program* program = loader();
    programs.insert(std::make_pair(loader, program));
    return program;
  } else {
    return iter->second;
  }
}
