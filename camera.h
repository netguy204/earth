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

  inline const Vector& axisZ() const {
    return look;
  }

  inline const Vector& axisY() const {
    return up;
  }

  inline Vector axisX() const {
    return look.cross(up); //up.cross(look);
  }

  inline Matrix getMatrix(bool rotationOnly = false) const {
    Matrix result;
    result.set_column(0, axisX());
    result.set_column(1, axisY());
    result.set_column(2, axisZ());
    if(!rotationOnly) result.set_column(3, pos);

    return result;
  }

  inline Matrix getWorldToCamera(bool rotationOnly = false) const {
    Matrix result;

    Vector z = -axisZ();
    Vector y = axisY();
    Vector x = y.cross(z);

    result.set_row(0, x);
    result.set_row(1, y);
    result.set_row(2, z);

    if(rotationOnly) {
      return result;
    } else {
      return result * Matrix::translation(-pos.x, -pos.y, -pos.z);
    }
  }

  void moveForward(float delta) {
    pos = pos + look * delta;
  }

  void moveUp(float delta) {
    pos = pos + up * delta;
  }

  void moveRight(float delta) {
    pos = pos + axisX() * delta;
  }

  inline void rotateZ(float angle) {
    Matrix m;
    m.set_rotation(angle, axisZ());
    up = m * up;
  }

  inline void rotateY(float angle) {
    Matrix m;
    m.set_rotation(angle, axisY());
    look = m * look;
  }

  inline void rotateX(float angle) {
    Matrix m;
    m.set_rotation(angle, axisX());
    look = m * look;
    up = m * up;
  }

  inline void normalize() {
    look = look.norm();
    up = up.norm();
  }

  inline void forceUp(const Vector& suggested_up) {
    // renormalize our vectors in a way that removes roll and puts the
    // up vector in the world up/look plane.
    look = look.norm();

    if(fabs(look.dot(suggested_up)) > 0.8) {
      Vector right = look.cross(up.norm());
      up = right.cross(look);
    } else {
      Vector right = look.cross(suggested_up);
      up = right.cross(look);
    }
  }

  inline Matrix getPerspectiveTransform() {
    /*
    Matrix result;
    float f = 1.0f / tan(fov/2);

    result.elm(0, 0) = f / aspect;
    result.elm(1, 1) = f;
    result.elm(2, 2) = (zmax + zmin) / (zmin - zmax);
    result.elm(2, 3) = 2 * zmax * zmin / (zmin - zmax);
    result.elm(3, 2) = -1;

    return result;
    */

    Matrix result;
    result.set_identity();

    float ymax = zmin * tanf(fov * 0.5f);
    float ymin = -ymax;
    float xmin = ymin * aspect;
    float xmax = -xmin;

    result.data[0] = (2.0f * zmin) / (xmax - xmin);
    result.data[5] = (2.0f * zmin) / (ymax - ymin);
    result.data[8] = (xmax + xmin) / (xmax - xmin);
    result.data[9] = (ymax + ymin) / (ymax - ymin);
    result.data[10] = -((zmax + zmin) / (zmax - zmin));
    result.data[11] = -1.0f;
    result.data[14] = -((2.0f * (zmax*zmin))/(zmax - zmin));
    result.data[15] = 0.0f;
    return result;
  }
};

#endif
