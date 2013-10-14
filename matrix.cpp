#include "matrix.h"

static float DetIJ(const Matrix& m, const int i, const int j);
void m3dInvertMatrix44(Matrix& mInverse, const Matrix& m);

Matrix::Matrix() {
  set_identity();
}

void Matrix::set_identity() {
  const float ident[16] = {1, 0, 0, 0,
                           0, 1, 0, 0,
                           0, 0, 1, 0,
                           0, 0, 0, 1};
  memcpy(data, ident, sizeof(ident));
}

Matrix Matrix::rotation(float angle, const Vector& v) {
  Matrix m;
  m.set_rotation(angle, v);
  return m;
}

Matrix Matrix::scale(float x, float y, float z) {
  Matrix m;
  m.set_scale(x, y, z);
  return m;
}

Matrix Matrix::translation(float x, float y, float z) {
  Matrix m;
  m.set_translation(x, y, z);
  return m;
}

float& Matrix::elm(unsigned r, unsigned c) {
  return data[c*4+r];
}

float Matrix::elm(unsigned r, unsigned c) const {
  return data[c*4+r];
}

Matrix Matrix::operator*(const Matrix& o) {
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

Matrix Matrix::invertspecial() {
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

Matrix Matrix::invert() {
  Matrix result;
  m3dInvertMatrix44(result, *this);
  return result;
}

void Matrix::set_scale(float x, float y, float z) {
  elm(0,0) = x;
  elm(1,1) = y;
  elm(2,2) = z;
}

void Matrix::set_translation(float x, float y, float z) {
  elm(0,3) = x;
  elm(1,3) = y;
  elm(2,3) = z;
}

void Matrix::set_column(unsigned c, const Vector& v) {
  elm(0,c) = v.x;
  elm(1,c) = v.y;
  elm(2,c) = v.z;
}

Vector Matrix::operator*(const Vector& o) {
  Vector result;
  result.x = o.x * elm(0,0) + o.y * elm(0,1) + o.z * elm(0,2) + elm(0,3);
  result.y = o.x * elm(1,0) + o.y * elm(1,1) + o.z * elm(1,2) + elm(1,3);
  result.z = o.x * elm(2,0) + o.y * elm(2,1) + o.z * elm(2,2) + elm(2,3);
  return result;
}

Vector4 Matrix::operator*(const Vector4& o) {
  Vector4 result;
  result.x = o.x * elm(0,0) + o.y * elm(0,1) + o.z * elm(0,2) + o.w * elm(0,3);
  result.y = o.x * elm(1,0) + o.y * elm(1,1) + o.z * elm(1,2) + o.w * elm(1,3);
  result.z = o.x * elm(2,0) + o.y * elm(2,1) + o.z * elm(2,2) + o.w * elm(2,3);
  result.w = o.x * elm(3,0) + o.y * elm(3,1) + o.z * elm(3,2) + o.w * elm(3,3);
  return result;
}

void Matrix::set_rotation(float angle, const Vector& v) {
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

std::string Matrix::str() const {
  return stdstring("%02.3f  %02.3f  %02.3f  %02.3f\n"
                   "%02.3f  %02.3f  %02.3f  %02.3f\n"
                   "%02.3f  %02.3f  %02.3f  %02.3f\n"
                   "%02.3f  %02.3f  %02.3f  %02.3f",
                   elm(0,0), elm(0,1), elm(0,2), elm(0,3),
                   elm(1,0), elm(1,1), elm(1,2), elm(1,3),
                   elm(2,0), elm(2,1), elm(2,2), elm(2,3),
                   elm(3,0), elm(3,1), elm(3,2), elm(3,3));
}

// lifted from opengl superbible
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

inline void m3dInvertMatrix44(Matrix& mInverse, const Matrix& m) {
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



Quaternion::Quaternion(float w, float x, float y, float z)
  : w(w), x(x), y(y), z(z) {
}

Quaternion::Quaternion()
  : w(1), x(0), y(0), z(0) {
}

Quaternion Quaternion::rotation(float angle, const Vector& axis) {
  float cangle = cosf(angle/2);
  float sangle = sinf(angle/2);

  return Quaternion(cangle, axis.x * sangle, axis.y * sangle, axis.z * sangle);
}

Quaternion Quaternion::operator*(const Quaternion& o) const {
  float nw = w * o.w - x * o.x - y * o.y - z * o.z;
  float nx = w * o.x + x * o.w + y * o.z - z * o.y;
  float ny = w * o.y - x * o.z + y * o.w + z * o.x;
  float nz = w * o.z + x * o.y - y * o.x + z * o.w;

  return Quaternion(nw, nx, ny, nz);
}

Vector Quaternion::operator*(const Vector& v) const {
  Quaternion r = *this * Quaternion(0, v.x, v.y, v.z);
  return Vector(r.x, r.y, r.z);
}

float Quaternion::magnitude() const {
  return sqrt(w * w + x * x + y * y + z * z);
}

void Quaternion::normalize() {
  float m = magnitude();
  w /= m;
  x /= m;
  y /= m;
  z /= m;
}

Matrix Quaternion::matrix() const {
  Matrix result;
  float xSq = x * x;
  float ySq = y * y;
  float zSq = z * z;
  float wSq = w * w;
  float twoX = 2.0f * x;
  float twoY = 2.0f * y;
  float twoW = 2.0f * w;
  float xy = twoX * y;
  float xz = twoX * z;
  float yz = twoY * z;
  float wx = twoW * x;
  float wy = twoW * y;
  float wz = twoW * z;

  //fill in the first row
  result.elm(0,0) = wSq + xSq - ySq - zSq;
  result.elm(0,1) = xy - wz;
  result.elm(0,2) = xz + wy;
  result.elm(0,3) = 0;

  //fill in the second row
  result.elm(1,0) = xy + wz;
  result.elm(1,1) = wSq - xSq + ySq - zSq;
  result.elm(1,2) = yz - wx;
  result.elm(1,3) = 0;

  //fill in the third row
  result.elm(2,0) = xz - wy;
  result.elm(2,1) = yz + wx;
  result.elm(2,2) = wSq - xSq - ySq + zSq;
  result.elm(2,3) = 0;

  result.elm(3,0) = 0;
  result.elm(3,1) = 0;
  result.elm(3,2) = 0;
  result.elm(3,3) = 1;
  return result;
}

std::string Quaternion::str() const {
  return stdstring("(%f, %f, %f, %f)", w, x, y, z);
}
