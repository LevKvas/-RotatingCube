#version 150
in vec4 col;
out vec4 fragColor;

in vec3 worldPos;
in vec3 normal;
in vec2 texCoord;

uniform mat4 matrix;
uniform int is_use_lamp;
uniform int is_use_SpotLight;
uniform int is_use_DirectionalLight;
uniform vec3 source_color;

uniform sampler2D normalMap; // sampler of normals map
uniform sampler2D earthTexture; // for Earth

// positions
uniform vec3 posLamp;
uniform vec3 PosSpotLight;

// directions
uniform vec3 SpotLightDir;
uniform vec3 DirLightDir;
uniform vec3 CameraPos;
uniform mat4 rot_matrix;
uniform float morphFactor;

// constants
uniform float a = 0.1;
uniform float b = 0.1;
uniform float c = 0.05;

uniform int n = 32; // degree of shiness

void main() {
  vec3 earthColor = texture2D(earthTexture, texCoord).rgb; // color of Earth
  // earthColor *= 0.1;
  // earthColor += vec3(0.9, 0.9, 0.9);

  vec3 N_geom = normalize(normal);
  // Касательная для сферы
  vec3 T = normalize(cross(vec3(0.0, 1.0, 0.0), N_geom));
  if (length(T) < 0.001) T = vec3(1.0, 0.0, 0.0); // защита на полюсах
  vec3 B = cross(N_geom, T);

  vec3 mapNormal = texture2D(normalMap, texCoord).rgb * 2.0 - vec3(1, 1, 1);;

  // Перевод из касательного пространства в мировое
  vec3 N = normalize(T * mapNormal.x + B * mapNormal.y + N_geom * mapNormal.z);
  vec3 V = normalize(CameraPos - worldPos);

  float specularIntensity = 0.3;

  vec3 I_lamp = vec3(0.0);
  vec3 I_spotLight = vec3(0.0);
  vec3 I_direct = vec3(0.0);
  vec3 I_background = earthColor * 0.005; // some noise

  // for lamp
  highp float d_lamp = length(posLamp - worldPos);
  float f_att_lamp = 1.0 / (a + b * d_lamp + c * d_lamp * d_lamp);
  vec3 L_lamp = normalize(posLamp - worldPos); // vector on Lamp

  vec3 R_lamp = reflect(-L_lamp, N);
  highp float cosAlpha_lamp = max(dot(R_lamp, V), 0.0);
  highp float cosTheta_lamp = max(dot(N, L_lamp), 0.0);

  float spec_lamp = 0.0;
  if (cosTheta_lamp > 0.0) {
      spec_lamp = pow(cosAlpha_lamp, n);
  }

  I_lamp = f_att_lamp * earthColor * source_color *  (cosTheta_lamp + specularIntensity * spec_lamp);

  // for SpotLight
  float d_SpotLight = length(PosSpotLight - worldPos);
  float f_att_spotLight = 1.0 / (a + b * d_SpotLight + c * d_SpotLight * d_SpotLight);
  vec3 L_SpotLight = normalize(PosSpotLight - worldPos); // vector on spotLight

  // Проверка угла конуса
  vec3 spotDirection = normalize(-SpotLightDir);  // направление ОТ источника
  float theta = dot(L_SpotLight, spotDirection);
  float cutoff = 0.7;  // cos(18°) — угол конуса

  if (theta < cutoff) {
      I_spotLight = vec3(0.0);
  }
  else {
      vec3 R_SpotLight = reflect(-L_SpotLight, N);
      float cosAlpha_spot = max(dot(R_SpotLight, V), 0.0);
      float cosTheta_spot = max(dot(N, L_SpotLight), 0.0);

      float spec_spot = 0.0;
      if (cosTheta_spot > 0.0) {
          spec_spot = pow(cosAlpha_spot, n);
      }
      I_spotLight = f_att_spotLight * earthColor * source_color * (cosTheta_spot + specularIntensity * spec_spot);
  }

  // for direct
  float f_att_direct = 1.0 / 256;
  vec3 L_direct = normalize(-DirLightDir);

  float cosTheta_direct = max(dot(N, L_direct), 0.0);

  vec3 R_direct = reflect(-L_direct, N);

  float spec_direct = 0.0;
  if (cosTheta_direct > 0.0) {
      vec3 R_direct = reflect(-L_direct, N);
      float cosAlpha_direct = max(dot(R_direct, V), 0.0);
      spec_direct = pow(cosAlpha_direct, n);
  }

  I_direct = f_att_direct * earthColor * source_color * (cosTheta_direct + specularIntensity * spec_direct);

  fragColor = vec4(is_use_lamp * I_lamp + is_use_SpotLight * I_spotLight + is_use_DirectionalLight * I_direct + I_background, 1.0);

  //fragColor= vec4(source_color * (cosTheta_direct + specularIntensity * spec_direct) / 256.0, 1.0);
}
