#version 430

layout (location = 0) in vec3 position; // [0, 1]^3
layout (location = 1) in vec3 boxMin;
layout (location = 2) in vec3 boxMax;
layout (location = 3) in vec3 boxColor;

layout (location = 0) out vec3 vColor;

uniform mat4 model;
uniform mat4 modelInverseTranspose;
uniform mat4 view;
uniform mat4 projection;

void main() {
  vec4 worldPosition = model * vec4(boxMin + (boxMax - boxMin) * position, 1.f);
  gl_Position = projection * view * worldPosition;
  vColor = boxColor;
}
