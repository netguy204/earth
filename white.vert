attribute vec3 vertex;
attribute vec3 normal;
attribute vec2 tcoord0;

uniform mat4 mvp;
varying vec2 tcoord;
varying vec3 vnormal;
varying vec3 vvertex;

void main() {
  tcoord = tcoord0;
  vnormal = mat3(mvp) * normal;
  vvertex = mat3(mvp) * vertex;

  gl_Position = vec4(vvertex, 1);
}
