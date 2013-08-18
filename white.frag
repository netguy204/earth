uniform sampler2D textureUnit0;
uniform sampler2D textureUnit1;

varying vec2 tcoord;
varying vec3 vnormal;
varying vec3 vvertex;

void main() {
  vec3 light = vec3(1,1,1);
  float shade = dot(vnormal, (vvertex - light));
  gl_FragColor = texture2D(textureUnit0, tcoord) * max(0, shade);
}
