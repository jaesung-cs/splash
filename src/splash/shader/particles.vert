#version 430

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 center;
layout (location = 2) in vec3 color;

layout (location = 0) out vec3 vPosition;
layout (location = 1) out vec3 vNormal;
layout (location = 2) out vec3 vColor;

uniform mat4 model;
uniform mat4 modelInverseTranspose;
uniform mat4 view;
uniform mat4 projection;

uniform float radius;

void main() {
  const vec4 worldPosition = model * vec4(center + position * radius, 1.f);

  gl_Position = projection * view * worldPosition;
  vPosition = worldPosition.xyz;
  vNormal = mat3(modelInverseTranspose) * position;
  vColor = color;
}
