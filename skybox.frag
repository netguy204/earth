uniform samplerCube colors;

varying vec3 vvertex;

void main() {
  gl_FragColor = textureCube(colors, vvertex);
}
