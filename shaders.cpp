#include "shaders.h"
#include "utils.h"

#include <stdlib.h>
#include <stdarg.h>

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
  for(int ii = 0; ii < UNIFORM_MAX; ++ii) {
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

  // uniform bindings are deferred until the program is linked
  const char* uniform_bindings[UNIFORM_MAX];
  memset(uniform_bindings, 0, sizeof(uniform_bindings));

  int program = glCreateProgram();
  Program* p = new Program();
  p->program = program;

  gl_check(glAttachShader(program, vertex));
  gl_check(glAttachShader(program, fragment));

  int mode = BINDING_INVALID;

  va_list ap;
  va_start(ap, fragmentname);
  while(1) {
    unsigned arg = va_arg(ap, int);
    // are we done?
    if(arg == BINDING_DONE) break;

    // if mode switch, do it now
    if(arg == BINDING_ATTRIBUTES || arg == BINDING_UNIFORMS) {
      mode = arg;
      continue;
    }

    // if there is no mode then error
    if(mode == BINDING_INVALID) {
      fail_exit("mode not defined before first binding");
    }

    if(mode == BINDING_ATTRIBUTES) {
      ProgramParameters param = (ProgramParameters)arg;
      const char* name = va_arg(ap, char*);
      gl_check(glBindAttribLocation(program, param, name));
    } else if(mode == BINDING_UNIFORMS) {
      ProgramUniforms uniform = (ProgramUniforms)arg;
      const char* name = va_arg(ap, char*);
      uniform_bindings[uniform] = name;
    }
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

  // now bind the uniforms
  for(unsigned uniform = 0; uniform < UNIFORM_MAX; ++uniform) {
    if(!uniform_bindings[uniform]) continue;
    const char* name = uniform_bindings[uniform];
    GLint loc = glGetUniformLocation(program, name);
    if(loc < 0) {
      fail_exit("glGetUniformLocation: %s error %d", name, loc);
    }
    p->uniforms[uniform] = loc;
  }

  return p;
}

void Program::bind_attribute_buffer(ProgramParameters attr, unsigned element_length, GLuint buffer) {
  gl_check(glBindBuffer(GL_ARRAY_BUFFER, buffer));
  gl_check(glEnableVertexAttribArray(attr));
  gl_check(glVertexAttribPointer(attr, element_length, GL_FLOAT, GL_FALSE, 0, 0));
}

void Program::bind_uniform(Texture* tex, unsigned slot, ProgramUniforms uni) {
  tex->bind(slot);
  gl_check(glUniform1i(requireUniform(uni), slot));
}

void Program::bind_uniform(Texture* tex, ProgramUniforms uni) {
  unsigned slot = uni - UNIFORM_TEX0;
  bind_uniform(tex, slot, uni);
}

void Program::bind_uniform(CubeMap* tex, unsigned slot, ProgramUniforms uni) {
  tex->bind(slot);
  gl_check(glUniform1i(requireUniform(uni), slot));
}

void Program::bind_uniform(CubeMap* tex, ProgramUniforms uni) {
  unsigned slot = uni - UNIFORM_TEX0;
  bind_uniform(tex, slot, uni);
}

void Program::bind_uniform(const Vector& v, ProgramUniforms uni) {
  gl_check(glUniform3fv(requireUniform(uni), 1, (const float*)&v));
}

void Program::bind_uniform(const Matrix& m, ProgramUniforms uni) {
  gl_check(glUniformMatrix4fv(requireUniform(uni), 1, GL_FALSE, m.data));
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
