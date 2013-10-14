#ifndef MATRIX_H
#define MATRIX_H

#include "point.h"
#include <string.h>

class Matrix;

class Matrix {
public:
  float data[16];

  Matrix();

  void set_identity();

  static Matrix rotation(float angle, const Vector& v);

  static Matrix scale(float x, float y, float z);

  static Matrix translation(float x, float y, float z);

  inline float& elm(unsigned r, unsigned c) {
    return data[c*4+r];
  }

  inline float elm(unsigned r, unsigned c) const {
    return data[c*4+r];
  }

  Matrix operator*(const Matrix& o);

  Matrix invertspecial();

  Matrix invert();

  void set_scale(float x, float y, float z);

  void set_translation(float x, float y, float z);

  void set_column(unsigned c, const Vector& v);

  void set_row(unsigned r, const Vector& v);

  Vector operator*(const Vector& o);

  Vector4 operator*(const Vector4& o);

  void set_rotation(float angle, const Vector& v);

  std::string str() const;
};

class Quaternion {
public:
  float w, x, y, z;

  Quaternion(float w, float x, float y, float z);
  Quaternion();

  static Quaternion rotation(float angle, const Vector& axis);

  Quaternion operator*(const Quaternion& other) const;
  Vector operator*(const Vector& other) const;

  Quaternion conj() const;

  float magnitude() const;

  void normalize();

  Matrix matrix() const;

  std::string str() const;
};

#endif
