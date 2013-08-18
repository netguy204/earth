#define BUILD_SDL
#include "gl_headers.h"

#include "utils.h"
#include "stb_image.h"
#include "matrix.h"
#include "point.h"

#include <stdio.h>
#include <stdlib.h>
#include <SDL/SDL.h>
#include <math.h>

#include <map>
#include <vector>

const char* libbase = ".";
unsigned screen_width = 800;
unsigned screen_height = 800;

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
  GLPARAM_DONE
} ProgramParameters;

typedef enum {
  UNIFORM_MVP,
  UNIFORM_COLOR0,
  UNIFORM_TEX0,
  UNIFORM_TEX1,
  UNIFORM_LIGHT0_POSITION,
  UNIFORM_SCALE,
  UNIFORM_TEX_BL,
  UNIFORM_TEX_TR,
  UNIFORM_DONE
} ProgramUniforms;

class Program {
 public:
  Program();
  ~Program();

  inline GLuint requireUniform(ProgramUniforms uniform) {
    if(uniforms[uniform] == -1) {
      fail_exit("uniform %d is not defined", uniform);
    }
    return uniforms[uniform];
  }

  inline void use() {
    if(program != current_program) {
      glUseProgram(program);
      current_program = program;
    }
  }

  GLuint uniforms[UNIFORM_DONE];
  GLuint program;

  static GLuint current_program;
};

typedef Program* (*ProgramLoader)(void);
typedef std::map<ProgramLoader, Program*> LoaderToProgram;

#define GL_CHECK_ERRORS

void gl_check_(const char * msg);

#define STRINGIZE(x) STRINGIZE2(x)
#define STRINGIZE2(x) #x

#ifdef GL_CHECK_ERRORS
#define gl_check(command) command; gl_check_(__FILE__ ": " STRINGIZE(__LINE__) " " #command)
#else
#define gl_check(command) command
#endif

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

Program* renderer_link_shader(const char* vertexname, const char* fragmentname, ...) {
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

class TexCoord {
public:
  float u, v;

  TexCoord(float u, float v)
    : u(u), v(v) {
  }
};

typedef std::vector<TexCoord> TexCoords;

Program* white_program_loader() {
  Program *program = renderer_link_shader("white.vert",
                                          "white.frag",
                                          GLPARAM_VERTEX, "vertex",
                                          GLPARAM_NORMAL0, "normal",
                                          GLPARAM_TEXCOORD0, "tcoord0",
                                          GLPARAM_DONE);

  program_bind_uniforms(program,
                        UNIFORM_TEX0, "textureUnit0",
                        UNIFORM_TEX1, "textureUnit1",
                        UNIFORM_MVP, "mvp",
                        UNIFORM_DONE);

  return program;
}

int main(int argc, char** argv) {
  if(SDL_Init(SDL_INIT_VIDEO) < 0) {
    fail_exit("unable to init SDL: %s\n", SDL_GetError());
  }

  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_Surface* screen = SDL_SetVideoMode(screen_width, screen_height,
                                         16, SDL_OPENGL);
  if(!screen) {
    fail_exit("unable to create screen: %s\n", SDL_GetError());
  }

  if(glewInit() != GLEW_OK) {
    fail_exit("failed to initialize GLEW");
  }

  Program* prog = get_program(white_program_loader);

  SDL_WM_SetCaption("Chuckle", NULL);

  glEnable(GL_TEXTURE_2D);
  glEnable(GL_BLEND);
  glEnable(GL_DEPTH_TEST);

  glClearColor(0,0,0,0);
  glViewport(0, 0, screen_width, screen_height);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  GLuint vbuffer, tbuffer, nbuffer;
  gl_check(glGenBuffers(1, &vbuffer));
  gl_check(glGenBuffers(1, &tbuffer));
  gl_check(glGenBuffers(1, &nbuffer));

  unsigned lats = 360;
  unsigned lons = 360;
  double lat_step = M_PI / lats;
  double lon_step = 2 * M_PI / lons;

  Points points;
  Points normals;
  TexCoords tcoords;

  float alt = 0;

  for(unsigned ilat = 0; ilat < lats; ++ilat) {
    float hlat = -M_PI/2 + lat_step * ilat;
    float nlat = -M_PI/2 + lat_step * (ilat + 1);

    float rhlat = (float)ilat / lats;
    float rnlat = (float)(ilat+1) / lats;

    for(unsigned ilon = 0; ilon < lons; ++ilon) {
      // generate a quad "here" to "next"
      float hlon = lon_step * ilon;
      float nlon = lon_step * (ilon + 1);

      float rhlon = (float)ilon / lons;
      float rnlon  = (float)(ilon+1) / lons;

      points.push_back(Point::fromLatLon(hlat, hlon, alt));
      tcoords.push_back(TexCoord(rhlon, rhlat));
      normals.push_back(Point::fromLatLon(hlat, hlon, alt).norm());

      points.push_back(Point::fromLatLon(nlat, hlon, alt));
      tcoords.push_back(TexCoord(rhlon, rnlat));
      normals.push_back(Point::fromLatLon(nlat, hlon, alt).norm());

      points.push_back(Point::fromLatLon(nlat, nlon, alt));
      tcoords.push_back(TexCoord(rnlon, rnlat));
      normals.push_back(Point::fromLatLon(nlat, nlon, alt).norm());


      points.push_back(Point::fromLatLon(nlat, nlon, alt));
      tcoords.push_back(TexCoord(rnlon, rnlat));
      normals.push_back(Point::fromLatLon(nlat, nlon, alt).norm());

      points.push_back(Point::fromLatLon(hlat, nlon, alt));
      tcoords.push_back(TexCoord(rnlon, rhlat));
      normals.push_back(Point::fromLatLon(hlat, nlon, alt).norm());

      points.push_back(Point::fromLatLon(hlat, hlon, alt));
      tcoords.push_back(TexCoord(rhlon, rhlat));
      normals.push_back(Point::fromLatLon(hlat, hlon, alt).norm());
    }
  }

  int ww, wh, wc;
  unsigned char* data = stbi_load("world.png", &ww, &wh, &wc, 0);
  if(!data) fail_exit("failed to load world.png");
  GLuint colors;
  glGenTextures(1, &colors);
  glBindTexture(GL_TEXTURE_2D, colors);
  gl_check(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
  gl_check(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
  gl_check(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
  gl_check(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
  gl_check(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, ww, wh, 0,
                        GL_RGB, GL_UNSIGNED_BYTE, data));

  free(data);
  data = stbi_load("topo.png", &ww, &wh, &wc, 0);
  if(!data) fail_exit("failed to load topo.png");
  GLuint topo;
  glGenTextures(1, &topo);
  glBindTexture(GL_TEXTURE_2D, topo);
  gl_check(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
  gl_check(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
  gl_check(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
  gl_check(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
  gl_check(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, ww, wh, 0,
                        GL_RGB, GL_UNSIGNED_BYTE, data));



  float angle = 0;

  Matrix pole_up;
  pole_up.set_rotation(M_PI/2, Vector(1,0,0));

  Matrix scale;
  scale.set_scale(0.8, 0.8, 0.8);
  pole_up = pole_up * scale;

  // render loop
  while(true) {
    Matrix m;
    m.set_rotation(angle, Vector(0,1,0));
    m = m * pole_up;

    angle += 0.001;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    SDL_Event event;
    /* pump the events */
    while(SDL_PollEvent(&event)) {
      switch(event.type) {
      case SDL_QUIT:
        return 0;
        break;
      default:
        break;
      }
    }

    prog->use();
    // verts
    gl_check(glEnableVertexAttribArray(GLPARAM_VERTEX));
    gl_check(glBindBuffer(GL_ARRAY_BUFFER, vbuffer));
    gl_check(glBufferData(GL_ARRAY_BUFFER, sizeof(Point) * points.size(),
                          (float*)&points[0], GL_DYNAMIC_DRAW));
    gl_check(glVertexAttribPointer(GLPARAM_VERTEX, 3, GL_FLOAT, GL_FALSE, 0, 0));

    // normals
    gl_check(glEnableVertexAttribArray(GLPARAM_NORMAL0));
    gl_check(glBindBuffer(GL_ARRAY_BUFFER, nbuffer));
    gl_check(glBufferData(GL_ARRAY_BUFFER, sizeof(Point) * normals.size(),
                          (float*)&points[0], GL_DYNAMIC_DRAW));
    gl_check(glVertexAttribPointer(GLPARAM_NORMAL0, 3, GL_FLOAT, GL_FALSE, 0, 0));

    // texs
    gl_check(glEnableVertexAttribArray(GLPARAM_TEXCOORD0));
    gl_check(glBindBuffer(GL_ARRAY_BUFFER, tbuffer));
    gl_check(glBufferData(GL_ARRAY_BUFFER, sizeof(TexCoord) * tcoords.size(),
                          (float*)&tcoords[0], GL_DYNAMIC_DRAW));
    gl_check(glVertexAttribPointer(GLPARAM_TEXCOORD0, 2, GL_FLOAT, GL_FALSE, 0, 0));

    gl_check(glActiveTexture(GL_TEXTURE0));
    gl_check(glBindTexture(GL_TEXTURE_2D, colors));
    gl_check(glUniform1i(prog->requireUniform(UNIFORM_TEX0), 0));

    gl_check(glActiveTexture(GL_TEXTURE1));
    gl_check(glBindTexture(GL_TEXTURE_2D, topo));
    gl_check(glUniform1i(prog->requireUniform(UNIFORM_TEX1), 1));

    gl_check(glUniformMatrix4fv(prog->requireUniform(UNIFORM_MVP), 1, GL_FALSE, m.data));

    gl_check(glDrawArrays(GL_TRIANGLES, 0, points.size()));

    SDL_GL_SwapBuffers();

  }

  return 0;
}
