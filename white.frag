uniform sampler2D colors;
uniform sampler2D topo;
uniform sampler2D specular;
uniform sampler2D night_lights;
uniform sampler2D normal_map;

varying vec2 tcoord;
varying vec3 vnormal;
varying vec3 vvertex;

// http://www.thetenthplanet.de/archives/1180
mat3 cotangent_frame(vec3 N, vec3 p, vec2 uv) {
  // get edge vectors of the pixel triangle
  vec3 dp1 = dFdx( p );
  vec3 dp2 = dFdy( p );
  vec2 duv1 = dFdx( uv );
  vec2 duv2 = dFdy( uv );

  // solve the linear system
  vec3 dp2perp = cross( dp2, N );
  vec3 dp1perp = cross( N, dp1 );
  vec3 T = dp2perp * duv1.x + dp1perp * duv2.x;
  vec3 B = dp2perp * duv1.y + dp1perp * duv2.y;

  // construct a scale-invariant frame
  float invmax = inversesqrt( max( dot(T,T), dot(B,B) ) );
  return mat3( T * invmax, B * invmax, N );
}

vec3 perturb_normal( vec3 N, vec3 V, vec2 texcoord ) {
  // assume N, the interpolated vertex normal and
  // V, the view vector (vertex to eye)
  vec3 map = texture2D(normal_map, texcoord ).xyz;
  map = map * 255./127. - 128./127.;
  mat3 TBN = cotangent_frame(N, -V, texcoord);
  return normalize(TBN * map);
}

void main() {
  vec3 eye = vec3(0,0,-10);
  vec3 light = vec3(100,100,-100); //mat3(mvp) * vec3(1,1,1);


  vec3 eyeDir = normalize(eye - vvertex);
  vec3 lightDir = normalize(light - vvertex);
  vec3 normal = perturb_normal(vnormal, eyeDir, tcoord);

  // sort of like the amount of light that is directly reflecting into
  // the eye
  vec3 reflection = reflect(-lightDir, normal);
  float eyeReflectionAngle = max(0.0, dot(reflection, eyeDir));
  float specCoeff = 1.8 * length(texture2D(specular, tcoord));
  float spec = 0.5 * min(1.0, pow(eyeReflectionAngle, specCoeff));
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
