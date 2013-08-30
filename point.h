#ifndef POINT_H
#define POINT_H

#include <vector>
#include <math.h>

class Point {
public:
  float x, y, z;

  inline Point()
    : x(0), y(0), z(0) {
  }

  inline Point(float x, float y, float z)
    : x(x), y(y), z(z) {
  }

  // angles in radians
  inline static Point fromLatLon(double lat, double lon, double h = 0) {
    float Rn = 1;
    float E = 0.081819190842621;

    float x = (Rn + h) * cos(lat) * cos(lon);
    float y = (Rn + h) * cos(lat) * sin(lon);
    float z = ((1 - E*E) * Rn + h) * sin(lat);
    return Point(x, y, z);
  }

  inline float dot(const Point& o) const {
    return x*o.x + y*o.y + z*o.z;
  }

  inline float mag() const {
    return sqrt(dot(*this));
  }

  inline Point norm() const {
    float m = mag();
    return Point(x/m, y/m, z/m);
  }

  inline Point cross(const Point& o) const {
    Point r;
    r.x = y*o.z - z*o.y;
    r.y = z*o.x - x*o.z;
    r.z = x*o.y - y*o.x;
    return r;
  }

  inline Point operator-() const {
    return Point(-x, -y, -z);
  }

  inline Point operator/(float s) const {
    return Point(x/s, y/s, z/s);
  }

  inline void print() const {
    printf("%02.3f  %02.3f  %02.3f\n", x, y, z);
  }
};

typedef Point Vector;
typedef std::vector<Point> Points;

class TexCoord {
public:
  float u, v;

  inline TexCoord(float u, float v)
    : u(u), v(v) {
  }
};

typedef std::vector<TexCoord> TexCoords;

// for homogenous transforms
class Vector4 {
public:
  float x, y, z, w;

  inline Vector4()
    : x(0), y(0), z(0), w(1) {
  }

  inline Vector4(float x, float y, float z, float w)
    : x(x), y(y), z(z), w(w) {
  }

  inline void print() const {
    printf("%02.3f  %02.3f  %02.3f  %02.3f\n", x, y, z, w);
  }

  inline Vector point() const {
    return Vector(x, y, z) / w;
  }
};

#endif
