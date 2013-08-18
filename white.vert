uniform sampler2D textureUnit1;

attribute vec3 vertex;
attribute vec3 normal;
attribute vec2 tcoord0;

uniform mat4 mvp;
varying vec2 tcoord;
varying vec3 vnormal;
varying vec3 vvertex;

void main() {
  tcoord = tcoord0;
  vnormal = normal;
  vvertex = vertex + normal * length(texture2D(textureUnit1, tcoord0)) * 0.025;
  gl_Position = mvp * vec4(vvertex, 1);
}
