#version 410 core

uniform sampler2D diffuse;
uniform sampler2D heightmap;

in VertexFS {
  vec2 texcoord;
  vec3 normal;
  } vert;

out vec4 color;

void main(void) {
  vec2 tc = vert.texcoord;
  color = texture2D(diffuse, tc);
  }
