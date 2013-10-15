uniform sampler2D colors;
uniform sampler2D norm_spec;
uniform sampler2D night_lights;

varying vec2 tcoord;
varying vec3 eyeDir;
varying vec3 lightDir;

void main() {
  // lightDir and eyeDir are already in tangent space so we can just
  // read our normal
  vec3 normal = normalize(texture2D(norm_spec, tcoord).rgb * 2.0 - 1.0);

  // proportional to the energy received by the surface
  float diffuseCoeff = dot(normal, lightDir);
  vec4 nightColor = vec4(0,0,0,0);
  const float ambient = 0.1;

  if(diffuseCoeff <= 0) {
    diffuseCoeff = 0;
    nightColor = texture2D(night_lights, tcoord);

    // only let the bright parts through
    if(length(vec3(nightColor)) < 0.6) {
      nightColor = vec4(0,0,0,0);
    }
  }

  vec3 lightDir = normalize(lightDir);
  vec3 eyeDir = normalize(eyeDir);

  vec4 color = (diffuseCoeff + ambient) * texture2D(colors, tcoord) + nightColor;

  // constants
  const float shininess = 100;
  const vec4 spec_color = vec4(1,1,0.8,1);
  const float specular_intensity = 0.4;

  // sort of like the amount of light that is directly reflecting into
  // the eye
  vec3 reflection = reflect(-lightDir, normal);
  float eyeReflectionAngle = dot(reflection, eyeDir);

  float specCoeff = texture2D(norm_spec, tcoord).a;
  float spec = max(0, specular_intensity * pow(eyeReflectionAngle, shininess * specCoeff)) * length(color);

  // kill specular if normal is facing away from light
  if(dot(lightDir, normal) < 0) spec = 0;

  gl_FragColor = color + spec_color * spec;
}
