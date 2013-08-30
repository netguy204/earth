#include "gl_headers.h"

#include "utils.h"
#include "stb_image.h"
#include "matrix.h"
#include "point.h"
#include "shaders.h"
#include "image.h"
#include "time.h"
#include "camera.h"

#include <stdio.h>
#include <stdlib.h>
#include <SDL/SDL.h>
#include <math.h>

#include <vector>

const char* libbase = ".";
unsigned screen_width = 1200;
unsigned screen_height = 720;

Program* ads_program_loader() {
  Program *program = Program::create("ads.vert",
                                     "ads.frag",
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
                        UNIFORM_MV, "mv",
                        UNIFORM_PERSPECTIVE, "perspective",
                        UNIFORM_DONE);

  return program;
}

Program* skybox_program_loader() {
  Program* program = Program::create("skybox.vert",
                                     "skybox.frag",
                                     GLPARAM_VERTEX, "vertex",
                                     GLPARAM_DONE);

  program_bind_uniforms(program,
                        UNIFORM_TEX0, "colors",
                        UNIFORM_MV, "mv",
                        UNIFORM_DONE);
  return program;
}

Program* simple_program_loader() {
  Program* program = Program::create("simple.vert",
                                     "simple.frag",
                                     GLPARAM_VERTEX, "vertex",
                                     GLPARAM_TEXCOORD0, "tcoord0",
                                     GLPARAM_NORMAL0, "normal",
                                     GLPARAM_DONE);

  program_bind_uniforms(program,
                        UNIFORM_MV, "mv",
                        UNIFORM_PERSPECTIVE, "perspective",
                        UNIFORM_TEX0, "colors",
                        UNIFORM_DONE);
  return program;
}


GLuint vbuffer, tbuffer, nbuffer, tanbuffer, qverts;
Program *ads;
Program *skybox;
Program *simple;

Texture* colors;
Texture* specular;
Texture* night_lights;
Texture* normal_map;
CubeMap* stars;

float angle;
unsigned points_size;
Camera camera(d2r(20), float(screen_width) / float(screen_height),
              0.1, 1000.0);
Matrix perspective(camera.getPerspectiveTransform());

void render_frame(const TimeLength& dt) {
  Matrix pole_up = Matrix::rotation(M_PI/2, Vector(1,0,0));
  Matrix o2w = Matrix::rotation(angle, Vector(0,1,0)) * pole_up;
  Matrix m = camera.getCameraToWorld().invert() * o2w;

  angle += dt.seconds() * (2 * M_PI * 0.05); // 1/20 rev per second

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  ads->use();

  // attributes
  gl_check(glBindBuffer(GL_ARRAY_BUFFER, vbuffer));
  gl_check(glEnableVertexAttribArray(GLPARAM_VERTEX));
  gl_check(glVertexAttribPointer(GLPARAM_VERTEX, 3, GL_FLOAT, GL_FALSE, 0, 0));

  gl_check(glBindBuffer(GL_ARRAY_BUFFER, nbuffer));
  gl_check(glEnableVertexAttribArray(GLPARAM_NORMAL0));
  gl_check(glVertexAttribPointer(GLPARAM_NORMAL0, 3, GL_FLOAT, GL_FALSE, 0, 0));

  gl_check(glBindBuffer(GL_ARRAY_BUFFER, tbuffer));
  gl_check(glEnableVertexAttribArray(GLPARAM_TEXCOORD0));
  gl_check(glVertexAttribPointer(GLPARAM_TEXCOORD0, 2, GL_FLOAT, GL_FALSE, 0, 0));

  gl_check(glBindBuffer(GL_ARRAY_BUFFER, tanbuffer));
  gl_check(glEnableVertexAttribArray(GLPARAM_TANGENT0));
  gl_check(glVertexAttribPointer(GLPARAM_TANGENT0, 3, GL_FLOAT, GL_FALSE, 0, 0));

  // textures
  colors->bind(0);
  gl_check(glUniform1i(ads->requireUniform(UNIFORM_TEX0), 0));

  specular->bind(1);
  gl_check(glUniform1i(ads->requireUniform(UNIFORM_TEX1), 1));

  night_lights->bind(2);
  gl_check(glUniform1i(ads->requireUniform(UNIFORM_TEX2), 2));

  normal_map->bind(3);
  gl_check(glUniform1i(ads->requireUniform(UNIFORM_TEX3), 3));

  gl_check(glUniformMatrix4fv(ads->requireUniform(UNIFORM_MV), 1, GL_FALSE, m.data));
  gl_check(glUniformMatrix4fv(ads->requireUniform(UNIFORM_PERSPECTIVE), 1, GL_FALSE, perspective.data));
  gl_check(glDrawArrays(GL_TRIANGLES, 0, points_size));


  gl_check(glDisableVertexAttribArray(GLPARAM_VERTEX));
  gl_check(glDisableVertexAttribArray(GLPARAM_NORMAL0));
  gl_check(glDisableVertexAttribArray(GLPARAM_TEXCOORD0));
  gl_check(glDisableVertexAttribArray(GLPARAM_TANGENT0));


  // render the skybox
  skybox->use();

  gl_check(glBindBuffer(GL_ARRAY_BUFFER, qverts));
  gl_check(glEnableVertexAttribArray(GLPARAM_VERTEX));
  gl_check(glVertexAttribPointer(GLPARAM_VERTEX, 3, GL_FLOAT, GL_FALSE, 0, 0));

  // hack... slow down the stars
  m = Matrix::rotation(angle*0.2, Vector(0,1,0)) * pole_up;

  stars->bind(0);
  gl_check(glUniform1i(skybox->requireUniform(UNIFORM_TEX0), 0));
  gl_check(glUniformMatrix4fv(skybox->requireUniform(UNIFORM_MV), 1, GL_FALSE, m.data));
  gl_check(glDrawArrays(GL_TRIANGLES, 0, 6));

  SDL_GL_SwapBuffers();
}

int main(int argc, char** argv) {
  char* output_prefix = NULL;
  int output_frames = 0;

  if(argc == 3) {
    output_prefix = argv[1];
    output_frames = atoi(argv[2]);
  }

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

  ads = get_program(ads_program_loader);
  skybox = get_program(skybox_program_loader);
  simple = get_program(simple_program_loader);

  SDL_WM_SetCaption("Chuckle", NULL);

  glEnable(GL_TEXTURE_2D);
  glEnable(GL_BLEND);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);

  glClearColor(0,0,0,0);
  glViewport(0, 0, screen_width, screen_height);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  gl_check(glGenBuffers(1, &vbuffer));
  gl_check(glGenBuffers(1, &tbuffer));
  gl_check(glGenBuffers(1, &nbuffer));
  gl_check(glGenBuffers(1, &tanbuffer));

  unsigned lats = 90;
  unsigned lons = 90;
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

  // bind all of our constant data

  // verts
  gl_check(glBindBuffer(GL_ARRAY_BUFFER, vbuffer));
  gl_check(glBufferData(GL_ARRAY_BUFFER, sizeof(Point) * points.size(),
                        (float*)&points[0], GL_DYNAMIC_DRAW));
  // normals
  gl_check(glBindBuffer(GL_ARRAY_BUFFER, nbuffer));
  gl_check(glBufferData(GL_ARRAY_BUFFER, sizeof(Point) * normals.size(),
                        (float*)&points[0], GL_DYNAMIC_DRAW));
  // texs
  gl_check(glBindBuffer(GL_ARRAY_BUFFER, tbuffer));
  gl_check(glBufferData(GL_ARRAY_BUFFER, sizeof(TexCoord) * tcoords.size(),
                        (float*)&tcoords[0], GL_DYNAMIC_DRAW));
  // tangents
  gl_check(glBindBuffer(GL_ARRAY_BUFFER, tanbuffer));
  gl_check(glBufferData(GL_ARRAY_BUFFER, sizeof(Point) * tangents.size(),
                        (float*)&tangents[0], GL_DYNAMIC_DRAW));

  points_size = points.size();

  colors = Texture::from_file("world.png");
  specular = Texture::from_file("EarthSpec.png");
  night_lights = Texture::from_file("earth_lights.png");
  normal_map = Texture::from_file("EarthNormal.png");
  stars = CubeMap::from_files("purplenebula_left.jpg",
                              "purplenebula_right.jpg",
                              "purplenebula_top.jpg",
                              "purplenebula_top.jpg",
                              "purplenebula_front.jpg",
                              "purplenebula_back.jpg");

  // build a full screen quad
  float qpoints[] = {
    -1, -1, 0.99,
    -1, 1, 0.99,
    1, 1, 0.99,

    1, 1, 0.99,
    1, -1, 0.99,
    -1, -1, 0.99
  };

  glGenBuffers(1, &qverts);
  gl_check(glBindBuffer(GL_ARRAY_BUFFER, qverts));
  gl_check(glBufferData(GL_ARRAY_BUFFER, sizeof(qpoints), qpoints, GL_DYNAMIC_DRAW));

  angle = 0;

  // render loop
  unsigned fcount = 0;
  Time flast;
  Time last_frame;

  FBO* fbo = NULL;
  if(output_prefix) {
    fbo = new FBO(screen_width, screen_height, GL_RGBA);
  }

  //perspective.set_identity();

  unsigned frame = 0;
  camera.pos = Vector(0, 0, 10);
  camera.look = Vector(0, 0, -1);
  camera.up = Vector(0, 1, 0);
  perspective.print();
  printf("\n");

  Matrix c(camera.getCameraToWorld().invert());
  c.print();
  printf("\n");

  for(int z = -15; z < 15; ++z) {
    printf("%d: ", z);
    (c * Vector4(0, 0, z, 1)).point().print();
    printf("%d: ", z);
    (perspective * c * Vector4(0, 0, z, 1)).point().print();
  }


  bool left = false;
  bool right = false;
  bool up = false;
  bool down = false;

  while(true) {
    SDL_Event event;
    /* pump the events */
    while(SDL_PollEvent(&event)) {
      switch(event.type) {
      case SDL_QUIT:
        exit(0);
        break;
      case SDL_KEYDOWN:
        switch(event.key.keysym.sym) {
        case SDLK_LEFT: left = true; break;
        case SDLK_RIGHT: right = true; break;
        case SDLK_UP: up = true; break;
        case SDLK_DOWN: down = true; break;
        default: break;
        }
        break;
      case SDL_KEYUP:
        switch(event.key.keysym.sym) {
        case SDLK_LEFT: left = false; break;
        case SDLK_RIGHT: right = false; break;
        case SDLK_UP: up = false; break;
        case SDLK_DOWN: down = false; break;
        default: break;
        }
        break;
      default:
        break;
      }
    }

    fcount++;
    Time now;

    TimeLength dt;
    if(fbo) {
      dt = TimeLength::inSeconds(1.0/60.0);
    } else {
      dt = now - last_frame;
    }
    last_frame = now;

    float speed = 5.0;
    float speedx = 0;
    float speedz = 0;
    if(left) speedx = -speed;
    if(right) speedx = speed;
    if(up) speedz = speed;
    if(down) speedz = -speed;

    float x = camera.pos.x;
    float y = camera.pos.y;
    float z = camera.pos.z;
    camera.pos = Vector(x + speedx * dt.seconds(), y,
                        z + speedz * dt.seconds());

    // keep the camera looking at world center
    //camera.look = (camera.pos).norm();

    if(!fbo) {
      // print fps every second
      TimeLength dt_fps = now - flast;
      if(dt_fps > TimeLength::inSeconds(1)) {
        fprintf(stderr, "%f\n", fcount / dt_fps.seconds());
        flast = now;
        fcount = 0;
      }
    }

    if(fbo) fbo->bind();

    render_frame(dt);

    if(fbo) {
      fbo->unbind();

      if(strcmp(output_prefix, "-") == 0) {
        fbo->to_ppm_file(stdout);
      } else {
        char fname[256];
        snprintf(fname, sizeof(fname), "%s_%06d.ppm", output_prefix, frame);
        fbo->to_ppm(fname);
      }

      frame++;
      if(frame == output_frames) break;
    }
  }

  if(fbo) delete fbo;

  return 0;
}
