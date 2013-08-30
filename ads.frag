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
  vec3 lightDir = normalize(lightDir);
  vec3 eyeDir = normalize(eyeDir);

  // proportional to the energy received by the surface
  float diffuseCoeff = clamp(dot(normal, lightDir), 0, 1);
  float ambientCoeff = 0.1;

  float dayCoeff = clamp(diffuseCoeff + ambientCoeff, 0, 1);
  float nightCoeff = pow(max(0, 1 - dayCoeff), 20);
  vec4 color = dayCoeff * texture2D(colors, tcoord)
    + nightCoeff * texture2D(night_lights, tcoord);

  // constants
  const float shininess = 100;
  const vec4 spec_color = vec4(1,1,0.8,1);
  const float specular_intensity = 0.4;

  // sort of like the amount of light that is directly reflecting into
  // the eye
  vec3 reflection = reflect(lightDir, normal);
  float eyeReflectionAngle = dot(reflection, eyeDir);

  float specCoeff = texture2D(specular, tcoord).a;
  float spec = max(0, specular_intensity * pow(eyeReflectionAngle, shininess * specCoeff)) * length(color);

  gl_FragColor = color + spec_color * spec;
  //gl_FragColor = vec4(eyeReflectionAngle, eyeReflectionAngle, eyeReflectionAngle, 1);
}
