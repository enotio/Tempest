#version 410 core

in vec3 Position;
in vec2 TexCoord;
in vec3 Normal;
in vec3 BiNormal;

out VertexVS
{
  vec2 texcoord;
  vec3 normal;
} vertvs;

uniform mat4 modelView;

vec3 normal( in vec3 n ){
  vec4 tmp = modelView*vec4(n,0.0);
  return normalize(tmp.xyz);
  }

void main() {
  vertvs.normal   = Normal;
  vertvs.texcoord = TexCoord;

  gl_Position = vec4(Position, 1.0);
  }
