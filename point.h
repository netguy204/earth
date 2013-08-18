#ifndef POINT_H
#define POINT_H

#include <vector>
#include <math.h>

class Point {
public:
  float x, y, z;

 inline Point(float x, float y, float z)
    : x(x), y(y), z(z) {
  }

  // angles in radians
 inline static Point fromLatLon(double lat, double lon, double h = 0) {
   float Rn = 1;
   float E = 0;

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


#endif
