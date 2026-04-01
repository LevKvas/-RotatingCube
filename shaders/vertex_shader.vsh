#version 150

in vec4 colAttr;
in vec3 posAttr;
in vec3 normalAttr;

out vec4 col;
out vec3 worldPos;
out vec3 normal;

uniform mat4 matrix;
uniform float morphFactor;
uniform mat4 rot_matrix;


void main() {
  col = colAttr;

  vec3 spherePos = normalize(posAttr);
  vec3 morphedPos = mix(posAttr, spherePos, morphFactor);

  vec4 temp = (rot_matrix * vec4(morphedPos, 1.0)); // mult on rot matrix
  worldPos = temp.xyz;

  vec4 temp2 = (rot_matrix * vec4(mix(normalAttr, spherePos, morphFactor), 1.0)); // mult on rot matrix
  normal = temp2.xyz;
  //normal = normalize(mat3(rot_matrix) * mix(normalAttr, spherePos, morphFactor));

  gl_Position = matrix * vec4(worldPos, 1.0);
}

