#version 430

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec2 vTexcoord;

layout (location = 0) out vec4 outColor;

uniform sampler2D tex;

void main() {
  // TODO: lighting
  outColor = vec4(texture(tex, vTexcoord).rgb, 1.f);
}
