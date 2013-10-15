attribute vec3 vertex;
uniform mat4 mv;
uniform mat4 perspective_inv;

varying vec3 vvertex;

void main() {
  vvertex = vec3(mv * perspective_inv * vec4(vertex,1));
  gl_Position = vec4(vertex, 1);
}
