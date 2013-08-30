uniform sampler2D colors;

uniform vec3 eye;

varying vec2 tcoord;
varying vec3 vnormal;
varying vec3 vvertex;

void main() {
  vec3 to_eye = normalize(eye - vvertex);
  gl_FragColor = texture2D(colors, tcoord);
}
