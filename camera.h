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

    float yMax = zmin * tanf(fov * 0.5f);
    float yMin = -yMax;
    float xMin = yMin * aspect;
    float xMax = -xMin;

    result.data[0] = (2.0f * zmin) / (xMax - xMin);
    result.data[5] = (2.0f * zmin) / (yMax - yMin);
    result.data[8] = (xMax + xMin) / (xMax - xMin);
    result.data[9] = (yMax + yMin) / (yMax - yMin);
    result.data[10] = -((zmax + zmin) / (zmax - zmin));
    result.data[11] = -1.0f;
    result.data[14] = -((2.0f * (zmax*zmin))/(zmax - zmin));
    result.data[15] = 0.0f;

    return result;
  }
};

#endif
