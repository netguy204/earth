uniform sampler2D colors;
uniform sampler2D topo;
uniform sampler2D specular;
uniform sampler2D night_lights;

varying vec2 tcoord;
varying vec3 vnormal;
varying vec3 vvertex;

void main() {
  vec3 eye = vec3(0,0,-10);
  vec3 light = vec3(100,100,-100); //mat3(mvp) * vec3(1,1,1);


  vec3 eyeDir = normalize(eye - vvertex);
  vec3 lightDir = normalize(light - vvertex);

  // sort of like the amount of light that is directly reflecting into
  // the eye
  vec3 reflection = reflect(-lightDir, vnormal);
  float eyeReflectionAngle = max(0.0, dot(reflection, eyeDir));
  float specCoeff = 1.1 * length(texture2D(specular, tcoord));
  float spec = min(1.0, pow(eyeReflectionAngle, specCoeff));
  vec4 spec_color = vec4(1,1,0.8,1);

  // proportional to the energy received by the surface
  float diffuseCoeff = clamp(dot(vnormal, lightDir), 0, 1);

  // random scattering off the surface
  float ambientCoeff = 0.05;

  float dayCoeff = clamp(diffuseCoeff + ambientCoeff, 0, 1);
  float nightCoeff = pow(max(0, 1 - diffuseCoeff), 10);

  gl_FragColor = dayCoeff * texture2D(colors, tcoord)
    + nightCoeff * texture2D(night_lights, tcoord)
    + spec_color * spec;
}
