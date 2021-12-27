#version 430

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texcoord;

layout (location = 0) out vec3 vPosition;
layout (location = 1) out vec3 vNormal;
layout (location = 2) out vec2 vTexcoord;

uniform mat4 model;
uniform mat4 modelInverseTranspose;
uniform mat4 view;
uniform mat4 projection;

void main() {
  vec4 worldPosition = model * vec4(position, 1.f);
  gl_Position = projection * view * worldPosition;
  vPosition = worldPosition.xyz;
  vNormal = mat3(modelInverseTranspose) * normal;
  vTexcoord = texcoord;
}
