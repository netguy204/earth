#ifndef MATRIX_H
#define MATRIX_H

#include "point.h"
#include <string.h>

class Matrix;

static float DetIJ(const Matrix& m, const int i, const int j);
void m3dInvertMatrix44(Matrix& mInverse, const Matrix& m);

class Matrix {
public:
  float data[16];

  inline Matrix() {
    set_identity();
  }

  inline void set_identity() {
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

  inline static Matrix translation(float x, float y, float z) {
    Matrix m;
    m.set_translation(x, y, z);
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

  inline Matrix invertspecial() {
    Matrix result;

    // rotation component is the transpose
    result.elm(0,0) = elm(0,0);
    result.elm(0,1) = elm(1,0);
    result.elm(0,2) = elm(2,0);

    result.elm(1,0) = elm(0,1);
    result.elm(1,1) = elm(1,1);
    result.elm(1,2) = elm(2,1);

    result.elm(2,0) = elm(0,2);
    result.elm(2,1) = elm(1,2);
    result.elm(2,2) = elm(2,2);

    // translation component is displacement negated and rotated
    result.elm(0,3) = -(elm(0,3) * result.elm(0,0) +
                        elm(1,3) * result.elm(0,1) +
                        elm(2,3) * result.elm(0,2));
    result.elm(1,3) = -(elm(0,3) * result.elm(1,0) +
                        elm(1,3) * result.elm(1,1) +
                        elm(2,3) * result.elm(1,2));
    result.elm(2,3) = -(elm(0,3) * result.elm(2,0) +
                        elm(1,3) * result.elm(2,1) +
                        elm(2,3) * result.elm(2,2));

    // and the bottom row
    result.elm(3,0) = 0;
    result.elm(3,1) = 0;
    result.elm(3,2) = 0;
    result.elm(3,3) = 1;
    return result;
  }

  inline Matrix invert() {
    Matrix result;
    m3dInvertMatrix44(result, *this);
    return result;
  }

  inline void set_scale(float x, float y, float z) {
    elm(0,0) = x;
    elm(1,1) = y;
    elm(2,2) = z;
  }

  inline void set_translation(float x, float y, float z) {
    elm(0,3) = x;
    elm(1,3) = y;
    elm(2,3) = z;
  }

  inline void set_column(unsigned c, const Vector& v) {
    elm(0,c) = v.x;
    elm(1,c) = v.y;
    elm(2,c) = v.z;
  }

  inline Vector operator*(const Vector& o) {
    Vector result;
    result.x = o.x * elm(0,0) + o.y * elm(0,1) + o.z * elm(0,2) + elm(0,3);
    result.y = o.x * elm(1,0) + o.y * elm(1,1) + o.z * elm(1,2) + elm(1,3);
    result.z = o.x * elm(2,0) + o.y * elm(2,1) + o.z * elm(2,2) + elm(2,3);
    return result;
  }

  inline Vector4 operator*(const Vector4& o) {
    Vector4 result;
    result.x = o.x * elm(0,0) + o.y * elm(0,1) + o.z * elm(0,2) + o.w * elm(0,3);
    result.y = o.x * elm(1,0) + o.y * elm(1,1) + o.z * elm(1,2) + o.w * elm(1,3);
    result.z = o.x * elm(2,0) + o.y * elm(2,1) + o.z * elm(2,2) + o.w * elm(2,3);
    result.w = o.x * elm(3,0) + o.y * elm(3,1) + o.z * elm(3,2) + o.w * elm(3,3);
    return result;
  }

  inline void set_rotation(float angle, const Vector& v) {
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

  inline void print() {
    printf("%02.3f  %02.3f  %02.3f  %02.3f\n"
           "%02.3f  %02.3f  %02.3f  %02.3f\n"
           "%02.3f  %02.3f  %02.3f  %02.3f\n"
           "%02.3f  %02.3f  %02.3f  %02.3f\n",
           elm(0,0), elm(0,1), elm(0,2), elm(0,3),
           elm(1,0), elm(1,1), elm(1,2), elm(1,3),
           elm(2,0), elm(2,1), elm(2,2), elm(2,3),
           elm(3,0), elm(3,1), elm(3,2), elm(3,3));
  }

};

////////////////////////////////////////////////////////////////////////////
/// This function is not exported by library, just for this modules use only
// 3x3 determinant
static float DetIJ(const Matrix& m, const int i, const int j) {
  int x, y, ii, jj;
  float ret, mat[3][3];

  x = 0;
  for (ii = 0; ii < 4; ii++) {
    if (ii == i) continue;
    y = 0;
    for (jj = 0; jj < 4; jj++) {
      if (jj == j) continue;
      mat[x][y] = m.data[(ii*4)+jj];
      y++;
    }
    x++;
  }

  ret =  mat[0][0]*(mat[1][1]*mat[2][2]-mat[2][1]*mat[1][2]);
  ret -= mat[0][1]*(mat[1][0]*mat[2][2]-mat[2][0]*mat[1][2]);
  ret += mat[0][2]*(mat[1][0]*mat[2][1]-mat[2][0]*mat[1][1]);

  return ret;
}

////////////////////////////////////////////////////////////////////////////
///
// Invert matrix
void m3dInvertMatrix44(Matrix& mInverse, const Matrix& m) {
  int i, j;
  float det, detij;

  // calculate 4x4 determinant
  det = 0.0f;
  for (i = 0; i < 4; i++) {
    det += (i & 0x1) ? (-m.data[i] * DetIJ(m, 0, i)) : (m.data[i] * DetIJ(m, 0,i));
  }
  det = 1.0f / det;

  // calculate inverse
  for (i = 0; i < 4; i++) {
    for (j = 0; j < 4; j++) {
      detij = DetIJ(m, j, i);
      mInverse.data[(i*4)+j] = ((i+j) & 0x1) ? (-detij * det) : (detij *det);
    }
  }
}


#endif
