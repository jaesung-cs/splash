#version 430

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec3 vColor;

layout (location = 0) out vec4 outColor;

struct Light {
  vec4 position; // .w = 0 for directional, 1 for positional
  vec4 ambient;
  vec4 diffuse;
  vec4 specular;
};

uniform vec3 eye;
uniform float shininess;

const uint MAX_NUM_LIGHTS = 8;
uniform int numLights;
uniform Light lights[MAX_NUM_LIGHTS];

vec3 calcLight(Light light, vec3 P, vec3 N, vec3 V) {
  vec3 L;
  if (light.position.w == 1) {
    L = normalize(light.position.xyz - P);
  } else {
    L = normalize(light.position.xyz);
  }

  const vec3 R = reflect(-L, N);

  const float ambientPower = 0.1f;
  const float diffusePower = max(dot(L, N), 0);
  const float specularPower = max(pow(dot(R, V), shininess), 0);

  const vec3 color = ambientPower * light.ambient.rgb
    + diffusePower * light.diffuse.rgb * vColor
    + specularPower * light.specular.rgb;

  return color;
}

void main() {
  vec3 V = normalize(eye - vPosition);
  vec3 N = normalize(vNormal);

  vec3 totalColor = vec3(0.f);
  for (int i = 0; i < numLights; i++) {
    totalColor += calcLight(lights[i], vPosition, N, V);
  }

  outColor = vec4(totalColor, 1.f);
}
