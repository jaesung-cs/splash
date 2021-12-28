#version 430

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec3 vColor;

layout (location = 0) out vec4 outColor;

void main() {
  // Test
  outColor = vec4(vNormal, 1.f);
}
