attribute vec3 vertex;
uniform mat4 mv;

varying vec3 vvertex;

void main() {
  vvertex = mat3(mv) * vertex;
  gl_Position = vec4(vertex, 1);
}
