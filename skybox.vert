attribute vec3 vertex;
uniform mat4 mvp;

varying vec3 vvertex;

void main() {
  vvertex = transpose(mat3(mvp)) * vertex;
  gl_Position = vec4(vertex, 1);
}
