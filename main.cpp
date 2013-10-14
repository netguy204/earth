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

unsigned screen_width = 1280;
unsigned screen_height = 800;

Program* ads_program_loader() {
  Program *program = Program::create("ads.vert",
                                     "ads.frag",
                                     BINDING_ATTRIBUTES,
                                     ATTRIBUTE_VERTEX, "vertex",
                                     ATTRIBUTE_NORMAL0, "normal",
                                     ATTRIBUTE_TEXCOORD0, "tcoord0",
                                     ATTRIBUTE_TANGENT0, "tangent",

                                     BINDING_UNIFORMS,
                                     UNIFORM_TEX0, "colors",
                                     UNIFORM_TEX1, "norm_spec",
                                     UNIFORM_TEX2, "night_lights",
                                     UNIFORM_MV, "mv",
                                     UNIFORM_LIGHT0_POSITION, "light",
                                     UNIFORM_PERSPECTIVE, "perspective",

                                     BINDING_DONE);

  return program;
}

Program* skybox_program_loader() {
  Program* program = Program::create("skybox.vert",
                                     "skybox.frag",
                                     BINDING_ATTRIBUTES,
                                     ATTRIBUTE_VERTEX, "vertex",

                                     BINDING_UNIFORMS,
                                     UNIFORM_TEX0, "colors",
                                     UNIFORM_MV, "mv",

                                     BINDING_DONE);

  return program;
}

Program* simple_program_loader() {
  Program* program = Program::create("simple.vert",
                                     "simple.frag",
                                     BINDING_ATTRIBUTES,
                                     ATTRIBUTE_VERTEX, "vertex",
                                     ATTRIBUTE_TEXCOORD0, "tcoord0",
                                     ATTRIBUTE_NORMAL0, "normal",

                                     BINDING_UNIFORMS,
                                     UNIFORM_MV, "mv",
                                     UNIFORM_PERSPECTIVE, "perspective",
                                     UNIFORM_TEX0, "colors",

                                     BINDING_DONE);

  return program;
}


GLuint vbuffer, tbuffer, nbuffer, tanbuffer, qverts;
Program *ads;
Program *skybox;
Program *simple;

Texture* colors;
Texture* norm_spec;
Texture* night_lights;
CubeMap* stars;

float angle;
unsigned points_size;
Camera camera(d2r(60), float(screen_width) / float(screen_height),
              0.1, 1000.0);
Matrix perspective(camera.getPerspectiveTransform());

void render_frame(const TimeLength& dt) {
  Matrix pole_up = Matrix::rotation(-M_PI/2, Vector(1,0,0));
  Matrix o2w = Matrix::rotation(angle, Vector(0,1,0)) * pole_up;
  Matrix w2c = camera.getWorldToCamera();
  Matrix m = w2c * o2w;

  angle += dt.seconds() * (2 * M_PI * 0.05); // 1/20 rev per second

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  ads->use();

  // attributes
  ads->bind_attribute_buffer(ATTRIBUTE_VERTEX, 3, vbuffer);
  ads->bind_attribute_buffer(ATTRIBUTE_NORMAL0, 3, nbuffer);
  ads->bind_attribute_buffer(ATTRIBUTE_TEXCOORD0, 2, tbuffer);
  ads->bind_attribute_buffer(ATTRIBUTE_TANGENT0, 3, tanbuffer);

  // textures
  ads->bind_uniform(colors, UNIFORM_TEX0);
  ads->bind_uniform(norm_spec, UNIFORM_TEX1);
  ads->bind_uniform(night_lights, UNIFORM_TEX2);

  // light
  Point light = w2c * Point(100, 0, 100);
  ads->bind_uniform(light, UNIFORM_LIGHT0_POSITION);
  ads->bind_uniform(m, UNIFORM_MV);
  ads->bind_uniform(perspective, UNIFORM_PERSPECTIVE);

  gl_check(glDrawArrays(GL_TRIANGLES, 0, points_size));

  // render the skybox
  skybox->use();

  skybox->bind_attribute_buffer(ATTRIBUTE_VERTEX, 3, qverts);
  skybox->bind_uniform(stars, UNIFORM_TEX0);
  skybox->bind_uniform(camera.getMatrix(true), UNIFORM_MV);

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
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  glFrontFace(GL_CCW);

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
    float nlat = hlat + lat_step;

    float rhlat = 1.0f - float(ilat) / float(lats);
    float rnlat = 1.0f - float(ilat+1) / float(lats);

    for(unsigned ilon = 0; ilon < lons; ++ilon) {
      // generate a quad "here" to "next"
      float hlon = -M_PI + lon_step * ilon;
      float nlon = hlon + lon_step;

      float rhlon = float(ilon) / float(lons);
      float rnlon  = float(ilon+1) / float(lons);

      points.push_back(Point::fromLatLon(hlat, hlon, alt));
      tcoords.push_back(TexCoord(rhlon, rhlat));
      Vector n1 = Point::fromLatLon(hlat, hlon, alt).norm();
      normals.push_back(n1);
      tangents.push_back(axis.cross(n1));

      points.push_back(Point::fromLatLon(nlat, nlon, alt));
      tcoords.push_back(TexCoord(rnlon, rnlat));
      Vector n3 = Point::fromLatLon(nlat, nlon, alt).norm();
      normals.push_back(n3);
      tangents.push_back(axis.cross(n3));

      points.push_back(Point::fromLatLon(nlat, hlon, alt));
      tcoords.push_back(TexCoord(rhlon, rnlat));
      Vector n2 = Point::fromLatLon(nlat, hlon, alt).norm();
      normals.push_back(n2);
      tangents.push_back(axis.cross(n2));

      points.push_back(Point::fromLatLon(nlat, nlon, alt));
      tcoords.push_back(TexCoord(rnlon, rnlat));
      Vector n4 = Point::fromLatLon(nlat, nlon, alt).norm();
      normals.push_back(n4);
      tangents.push_back(axis.cross(n4));

      points.push_back(Point::fromLatLon(hlat, hlon, alt));
      tcoords.push_back(TexCoord(rhlon, rhlat));
      Vector n6 = Point::fromLatLon(hlat, hlon, alt).norm();
      normals.push_back(n6);
      tangents.push_back(axis.cross(n6));

      points.push_back(Point::fromLatLon(hlat, nlon, alt));
      tcoords.push_back(TexCoord(rnlon, rhlat));
      Vector n5 = Point::fromLatLon(hlat, nlon, alt).norm();
      normals.push_back(n5);
      tangents.push_back(axis.cross(n5));
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
  norm_spec = Texture::from_file("EarthNormSpec.png");
  night_lights = Texture::from_file("earth_lights.png");
  stars = CubeMap::from_files("purplenebula_left.jpg",
                              "purplenebula_right.jpg",
                              "purplenebula_top.jpg",
                              "purplenebula_top.jpg",
                              "purplenebula_front.jpg",
                              "purplenebula_back.jpg");

  // build a full screen quad
  float qpoints[] = {
    -1, -1, 0.99,
    1, 1, 0.99,
    -1, 1, 0.99,

    1, 1, 0.99,
    -1, -1, 0.99,
    1, -1, 0.99
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
  /*
  perspective.print();
  printf("\n");

  Matrix c(camera.getCameraToWorld().invertspecial());
  c.print();
  printf("\n");

  for(int z = -15; z < 25; ++z) {
    printf("%d: ", z);
    (c * Vector4(1, 1, z, 1)).point().print();
    printf("%d: ", z);
    (perspective * c * Vector4(1, 1, z, 1)).point().print();
  }
  */

  bool left = false;
  bool right = false;
  bool up = false;
  bool down = false;

  SDL_WM_GrabInput(SDL_GRAB_ON);
  SDL_ShowCursor(SDL_DISABLE);

  while(true) {
    SDL_Event event;
    float xrel = 0.0f;
    float yrel = 0.0f;

    /* pump the events */
    while(SDL_PollEvent(&event)) {
      switch(event.type) {
      case SDL_QUIT:
        exit(0);
        break;
      case SDL_MOUSEMOTION:
        xrel = event.motion.xrel;
        yrel = event.motion.yrel;
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

    float speed = 0.05;
    float speedx = 0;
    float speedz = 0;
    if(left) speedx = -speed;
    if(right) speedx = speed;
    if(up) speedz = speed;
    if(down) speedz = -speed;

    if(abs(yrel) > 0) camera.rotateX(yrel * dt.seconds());
    if(abs(xrel) > 0) camera.rotateY(-xrel * dt.seconds());
    //camera.forceUp(Vector(0, 1, 0));
    camera.normalize();

    camera.moveForward(speedz);
    camera.moveRight(speedx);

    printf("look: %s  up: %s\n", camera.look.str().c_str(), camera.up.str().c_str());
    printf("pos: %s\n", camera.pos.str().c_str());
    printf("l.u=%f, u.r=%f, r.l=%f\n", camera.look.dot(camera.up), camera.up.dot(camera.axisX()),
           camera.axisX().dot(camera.look));

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
