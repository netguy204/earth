uniform sampler2D colors;
uniform sampler2D topo;
uniform sampler2D specular;
uniform sampler2D night_lights;
uniform sampler2D normal_map;

varying vec2 tcoord;
varying vec3 eyeDir;
varying vec3 lightDir;

void main() {
  // lightDir and eyeDir are already in tangent space so we can just
  // read our normal
  vec3 normal = normalize(texture2D(normal_map, tcoord).rgb * 2.0 - 1.0);

  // sort of like the amount of light that is directly reflecting into
  // the eye
  vec3 reflection = reflect(-lightDir, normal);
  float eyeReflectionAngle = max(0.0, dot(reflection, eyeDir));
  float specCoeff = 2.2 * texture2D(specular, tcoord).a;
  float spec = min(1.0, pow(eyeReflectionAngle, specCoeff));
  vec4 spec_color = vec4(1,1,0.8,1);

  // proportional to the energy received by the surface
  float diffuseCoeff = clamp(dot(normal, lightDir), 0, 1);

  // random scattering off the surface
  float ambientCoeff = 0.1;

  float dayCoeff = clamp(diffuseCoeff + ambientCoeff, 0, 1);
  float nightCoeff = pow(max(0, 1 - dayCoeff), 10);

  gl_FragColor = dayCoeff * texture2D(colors, tcoord)
    + nightCoeff * texture2D(night_lights, tcoord)
    + spec_color * spec;
}
