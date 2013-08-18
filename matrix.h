#ifndef MATRIX_H
#define MATRIX_H

#include "point.h"
#include <string.h>

class Matrix {
public:
  float data[16];

  inline Matrix() {
    const float ident[16] = {1, 0, 0, 0,
                       0, 1, 0, 0,
                       0, 0, 1, 0,
                       0, 0, 0, 1};
    memcpy(data, ident, sizeof(ident));
  }

  inline static Matrix rotation(float angle, const Vector& v) {
    Matrix m;
    m.set_rotation(angle, v);
    return m;
  }

  inline static Matrix scale(float x, float y, float z) {
    Matrix m;
    m.set_scale(x, y, z);
    return m;
  }

  inline float& elm(unsigned r, unsigned c) {
    return data[c*4+r];
  }

  inline float elm(unsigned r, unsigned c) const {
    return data[c*4+r];
  }

  inline Matrix operator*(const Matrix& o) {
    Matrix result;
    for(unsigned ii = 0; ii < 4; ++ii) {
      for(unsigned jj = 0; jj < 4; ++jj) {
        result.elm(ii,jj) = 0;
        for(unsigned kk = 0; kk < 4; ++kk) {
          result.elm(ii,jj) += elm(ii,kk) * o.elm(kk,jj);
        }
      }
    }
    return result;
  }

  void set_scale(float x, float y, float z) {
    elm(0,0) = x;
    elm(1,1) = y;
    elm(2,2) = z;
  }

  void set_rotation(float angle, const Vector& v) {
    Vector n = v.norm();
    float s = float(sin(angle));
    float c = float(cos(angle));
    float x = n.x;
    float y = n.y;
    float z = n.z;
    float xx = x * x;
    float yy = y * y;
    float zz = z * z;
    float xy = x * y;
    float yz = y * z;
    float zx = z * x;
    float xs = x * s;
    float ys = y * s;
    float zs = z * s;
    float one_c = 1.0f - c;

    elm(0,0) = (one_c * xx) + c;
    elm(0,1) = (one_c * xy) - zs;
    elm(0,2) = (one_c * zx) + ys;

    elm(1,0) = (one_c * xy) + zs;
    elm(1,1) = (one_c * yy) + c;
    elm(1,2) = (one_c * yz) - xs;

    elm(2,0) = (one_c * zx) - ys;
    elm(2,1) = (one_c * yz) + xs;
    elm(2,2) = (one_c * zz) + c;
  }
};

#endif
