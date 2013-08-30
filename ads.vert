attribute vec3 vertex;
attribute vec3 normal;
attribute vec2 tcoord0;
attribute vec3 tangent;

uniform mat4 mv;
uniform mat4 perspective;

varying vec2 tcoord;
varying vec3 eyeDir;
varying vec3 lightDir;

void main() {
  tcoord = tcoord0;
  vec4 tfvert = mv * vec4(vertex, 1);

  vec3 normal = mat3(mv) * normal;
  vec3 vertex = vec3(tfvert);

  // build the transform from view space to tangent space
  vec3 tangent = mat3(mv) * tangent;
  vec3 bitangent = cross(normal, tangent);
  mat3 e2t = transpose(mat3(tangent, bitangent, normal));

  // light location in view space
  vec3 light = vec3(100,0,100);

  // send to fragment shader in tangent space
  eyeDir = e2t * normalize(-vertex);
  lightDir = e2t * normalize(light - vertex);

  vec4 pvert = perspective * tfvert;
  gl_Position = pvert / pvert.w;
}
