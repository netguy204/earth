#include "gl_headers.h"

#include "utils.h"
#include "stb_image.h"
#include "matrix.h"
#include "point.h"
#include "shaders.h"
#include "image.h"
#include "time.h"

#include <stdio.h>
#include <stdlib.h>
#include <SDL/SDL.h>
#include <math.h>

#include <vector>

const char* libbase = ".";
unsigned screen_width = 800;
unsigned screen_height = 800;

Program* white_program_loader() {
  Program *program = Program::create("white.vert",
                                     "white.frag",
                                     GLPARAM_VERTEX, "vertex",
                                     GLPARAM_NORMAL0, "normal",
                                     GLPARAM_TEXCOORD0, "tcoord0",
                                     GLPARAM_TANGENT0, "tangent",
                                     GLPARAM_DONE);

  program_bind_uniforms(program,
                        UNIFORM_TEX0, "colors",
                        UNIFORM_TEX1, "specular",
                        UNIFORM_TEX2, "night_lights",
                        UNIFORM_TEX3, "normal_map",
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

  GLuint vbuffer, tbuffer, nbuffer, tanbuffer;
  gl_check(glGenBuffers(1, &vbuffer));
  gl_check(glGenBuffers(1, &tbuffer));
  gl_check(glGenBuffers(1, &nbuffer));
  gl_check(glGenBuffers(1, &tanbuffer));

  unsigned lats = 360;
  unsigned lons = 360;
  double lat_step = M_PI / lats;
  double lon_step = 2 * M_PI / lons;

  Points points;
  Points normals;
  Points tangents;
  TexCoords tcoords;

  float alt = 0;
  Vector axis(0,0,1);

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
      Vector n1 = Point::fromLatLon(hlat, hlon, alt).norm();
      normals.push_back(n1);
      tangents.push_back(axis.cross(n1));

      points.push_back(Point::fromLatLon(nlat, hlon, alt));
      tcoords.push_back(TexCoord(rhlon, rnlat));
      Vector n2 = Point::fromLatLon(nlat, hlon, alt).norm();
      normals.push_back(n2);
      tangents.push_back(axis.cross(n2));

      points.push_back(Point::fromLatLon(nlat, nlon, alt));
      tcoords.push_back(TexCoord(rnlon, rnlat));
      Vector n3 = Point::fromLatLon(nlat, nlon, alt).norm();
      normals.push_back(n3);
      tangents.push_back(axis.cross(n3));


      points.push_back(Point::fromLatLon(nlat, nlon, alt));
      tcoords.push_back(TexCoord(rnlon, rnlat));
      Vector n4 = Point::fromLatLon(nlat, nlon, alt).norm();
      normals.push_back(n4);
      tangents.push_back(axis.cross(n4));

      points.push_back(Point::fromLatLon(hlat, nlon, alt));
      tcoords.push_back(TexCoord(rnlon, rhlat));
      Vector n5 = Point::fromLatLon(hlat, nlon, alt).norm();
      normals.push_back(n5);
      tangents.push_back(axis.cross(n5));

      points.push_back(Point::fromLatLon(hlat, hlon, alt));
      tcoords.push_back(TexCoord(rhlon, rhlat));
      Vector n6 = Point::fromLatLon(hlat, hlon, alt).norm();
      normals.push_back(n6);
      tangents.push_back(axis.cross(n6));
    }
  }

  Texture* colors = Texture::from_file("world.png");
  Texture* specular = Texture::from_file("EarthSpec.png");
  Texture* night_lights = Texture::from_file("earth_lights.png");
  Texture* normal_map = Texture::from_file("EarthNormal.png");

  // bind all of our constant data

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

  // tangents
  gl_check(glEnableVertexAttribArray(GLPARAM_TANGENT0));
  gl_check(glBindBuffer(GL_ARRAY_BUFFER, tanbuffer));
  gl_check(glBufferData(GL_ARRAY_BUFFER, sizeof(Point) * tangents.size(),
                        (float*)&tangents[0], GL_DYNAMIC_DRAW));
  gl_check(glVertexAttribPointer(GLPARAM_TANGENT0, 3, GL_FLOAT, GL_FALSE, 0, 0));

  float angle = 0;

  Matrix pole_up = Matrix::rotation(M_PI/2, Vector(1,0,0)) * Matrix::scale(0.8, 0.8, 0.8);

  // render loop
  unsigned fcount = 0;
  Time flast;
  Time last_frame;

  while(true) {
    fcount++;
    Time now;

    TimeLength dt = now - last_frame;
    last_frame = now;

    // print fps every second
    TimeLength dt_fps = now - flast;
    if(dt_fps > TimeLength::inSeconds(1)) {
      printf("%f\n", fcount / dt_fps.seconds());
      flast = now;
      fcount = 0;
    }

    Matrix m = Matrix::rotation(angle, Vector(0,1,0)) * pole_up;

    angle += dt.seconds() * (2 * M_PI * 0.05); // 1/20 rev per second

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

    // textures
    colors->bind(0);
    gl_check(glUniform1i(prog->requireUniform(UNIFORM_TEX0), 0));

    specular->bind(1);
    gl_check(glUniform1i(prog->requireUniform(UNIFORM_TEX1), 1));

    night_lights->bind(2);
    gl_check(glUniform1i(prog->requireUniform(UNIFORM_TEX2), 2));

    normal_map->bind(3);
    gl_check(glUniform1i(prog->requireUniform(UNIFORM_TEX3), 3));

    gl_check(glUniformMatrix4fv(prog->requireUniform(UNIFORM_MVP), 1, GL_FALSE, m.data));

    gl_check(glDrawArrays(GL_TRIANGLES, 0, points.size()));

    SDL_GL_SwapBuffers();

  }

  return 0;
}
