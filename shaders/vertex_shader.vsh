#version 150

in vec4 colAttr;
in vec3 posAttr;
in vec3 normalAttr;

out vec4 col;
out vec3 worldPos;
out vec3 normal;
out vec2 texCoord; // to fragment shader


uniform mat4 matrix;
uniform float morphFactor;
uniform mat4 rot_matrix;


void main() {
  col = colAttr;

  vec3 spherePos = normalize(posAttr);
  vec3 morphedPos = mix(posAttr, spherePos, morphFactor);

  // === uv to sphere coordinates ===
  float u = 0.5 + atan(spherePos.x, spherePos.z) / (2.0 * 3.14159265); // longitude
  float v = 0.5 - asin(spherePos.y) / 3.14159265; // latitude
  texCoord = vec2(clamp(u, 0.0, 1.0), clamp(v, 0.0, 1.0));

  vec4 temp = (rot_matrix * vec4(morphedPos, 1.0)); // mult on rot matrix
  worldPos = temp.xyz;

  normal = normalize(mat3(rot_matrix) * mix(normalAttr, spherePos, morphFactor));

  gl_Position = matrix * vec4(worldPos, 1.0);
}
