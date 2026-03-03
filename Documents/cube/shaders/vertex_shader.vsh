#version 150
in vec3 posAttr;
in vec4 colAttr;
in vec3 normalAttr;

out vec4 col;
out vec3 worldPos;
out vec3 normal;

uniform mat4 matrix;
uniform mat4 rot_matrix;
uniform float morphFactor;

uniform int is_use_lamp;
uniform int is_use_SpotLight;
uniform int is_use_DirectionalLight;

// positions
uniform vec3 posLamp;
uniform vec3 PosSpotLight;

// directions
uniform vec3 SpotLightDir;
uniform vec3 DirLightDir;
uniform vec3 CameraPos;

// constants
uniform float a = 0.1;
uniform float b = 0.1;
uniform float c = 1.0;

uniform int n = 32; // degree of shiness

void main() {
  col = colAttr;

  vec3 spherePos = normalize(posAttr);
  vec3 morphedPos = mix(posAttr, spherePos, morphFactor);

  vec4 temp = (rot_matrix * vec4(morphedPos, 1.0)); // mult on rot matrix
  worldPos = temp.xyz;
  normal = normalAttr;

  gl_Position = matrix * rot_matrix * vec4(morphedPos, 1.0);

  vec3 N = normalize(normal); // normal of surface
  vec3 V = normalize(CameraPos - worldPos); // vector on camera

  vec3 I_lamp = vec3(0.0);
  vec3 I_spotLight = vec3(0.0);
  vec3 I_direct = vec3(0.0);
  vec3 I_background = vec3(0.2); // some noise

  // for lamp
  float d_lamp = length(posLamp - worldPos);
  float f_att_lamp = 1.0 / (a + b * d_lamp + c * d_lamp * d_lamp);
  vec3 L_lamp = normalize(posLamp - worldPos); // vector on Lamp

  vec3 R_lamp = reflect(-L_lamp, N);
  float cosAlpha_lamp = max(dot(R_lamp, V), 0.0);
  float cosTheta_lamp = max(dot(N, L_lamp), 0.0);

  I_lamp = f_att_lamp * colAttr.rgb * (cosTheta_lamp + pow(cosAlpha_lamp, n));

  // for SpotLight
  float d_SpotLight = length(PosSpotLight - worldPos);
  float f_att_spotLight = 1.0 / (a + b * d_SpotLight + c * d_SpotLight * d_SpotLight);
  vec3 L_SpotLight = normalize(PosSpotLight - worldPos); // vector on spotLight

  vec3 R_SpotLight = reflect(-L_SpotLight, N);
  float cosAlpha_spot = max(dot(R_SpotLight, V), 0.0);
  float cosTheta_spot = max(dot(N, L_SpotLight), 0.0);

  I_spotLight = f_att_spotLight * colAttr.rgb * (cosTheta_spot + pow(cosAlpha_spot, n));

  // for direct
  float f_att_direct = 1 / a;
  vec3 L_direct = normalize(-DirLightDir);

  float cosTheta_direct = max(dot(N, L_direct), 0.0);

  vec3 R_direct = reflect(-L_direct, N);
  float cosAlpha_direct = max(dot(R_direct, V), 0.0);

  I_direct = f_att_direct * colAttr.rgb * (cosTheta_direct + pow(cosAlpha_direct, n));

  col = vec4(is_use_lamp * I_lamp + is_use_SpotLight * I_spotLight + is_use_DirectionalLight * I_direct + I_background, 1.0);
}

