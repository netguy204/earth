#ifndef CAMERA_H
#define CAMERA_H

#include "point.h"

inline float d2r(float deg) {
  return deg * M_PI / 180.0f;
}

inline float r2d(float rad) {
  return rad * 180.0f / M_PI;
}

class Camera {
public:
  Vector look;
  Vector up;
  Vector pos;

  float fov;
  float aspect;
  float zmin;
  float zmax;

  inline Camera(float fov, float aspect, float zmin, float zmax)
    : look(Vector(0,0,-1)), up(Vector(0,1,0)), pos(Vector(0,0,0)),
      fov(fov), aspect(aspect), zmin(zmin), zmax(zmax) {
  }

  inline Matrix getCameraToWorld() const {
    Vector right = look.cross(up);
    Matrix result;
    result.set_column(0, right);
    result.set_column(1, up);
    result.set_column(2, -look);
    result.set_column(3, pos);
    return result;
  }

  inline Matrix getPerspectiveTransform() {
    Matrix result;
    float f = 1.0f / tan(fov/2);

    result.elm(0, 0) = f / aspect;
    result.elm(1, 1) = f;
    result.elm(2, 2) = (zmax + zmin) / (zmin - zmax);
    result.elm(2, 3) = 2 * zmax * zmin / (zmin - zmax);
    result.elm(3, 2) = -1;

    return result;
  }
};

#endif
