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

  inline Vector right() const {
    return look.cross(up);
  }

  inline Matrix getCameraToWorld() const {
    Matrix result;
    result.set_column(0, right());
    result.set_column(1, up);
    result.set_column(2, -look);

    return Matrix::translation(pos.x, pos.y, pos.z) * result;
  }

  inline void rotateZ(float angle) {
    Matrix m;
    m.set_rotation(angle, look);
    up = m * up;
  }

  inline void rotateY(float angle) {
    Matrix m;
    m.set_rotation(angle, up);
    look = m * look;
  }

  inline void rotateX(float angle) {
    Matrix m;
    m.set_rotation(angle, right());
    look = m * look;
    up = m * up;
  }

  inline void forceUp(const Vector& suggested_up) {
    // renormalize our vectors in a way that removes roll and puts the
    // up vector in the world up/look plane.
    look = look.norm();
    Vector right = look.cross(suggested_up);
    up = right.cross(look);
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
