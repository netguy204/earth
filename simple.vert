attribute vec3 vertex;
attribute vec3 normal;
attribute vec2 tcoord0;

uniform mat4 mv;
uniform mat4 perspective;

varying vec2 tcoord;
varying vec3 vnormal;
varying vec3 vvertex;

void main() {
  tcoord = tcoord0;
  vnormal = mat3(mv) * normal;

  vec4 vertex = mv * vec4(vertex, 1);
  vvertex = (mv * vertex).xyz;

  vec4 pvert = perspective * vertex;
  gl_Position = pvert / pvert.w;
}
