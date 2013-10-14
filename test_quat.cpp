#include <SDL/SDL.h>
#include "matrix.h"

int main(int argc, char** argv) {
  Matrix r1 = Matrix::rotation(.42, Vector(1,2,3).norm());

  Quaternion q1 = Quaternion::rotation(.42, Vector(1,2,3).norm());
  Matrix r2 = q1.matrix();

  printf("r1:\n%s\n\nr2\n%s\n", r1.str().c_str(), r2.str().c_str());

  Vector v1(1,2,3);
  printf(" v1: %s\n v2: %s\n v2': %s\n", (r1*v1).str().c_str(), (q1*v1).str().c_str(),
         (r2 * v1).str().c_str());
  printf(" v2'': %s\n", (Quaternion(0, v1.x, v1.y, v1.z) * q1).str().c_str());
  return 0;
}
