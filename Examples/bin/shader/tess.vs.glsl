#version 410 core

in vec3 Position;
in vec2 TexCoord;
in vec3 Normal;
in vec3 BiNormal;

out vec4 pos;
out vec3 n;
out vec3 bn;
out vec3 tn;

out vec2 tc;

uniform mat4 mvpMatrix, modelView;

vec3 normal( in vec3 n ){
  vec4 tmp = modelView*vec4(n,0.0);
  return normalize(tmp.xyz);
  }

void main() {
  n  =  normal(Normal);
  bn =  normal(BiNormal);
  tn = -normalize(cross( bn, n ));

  tc = TexCoord;

  vec4 tmp = mvpMatrix*vec4( Position, 1.0 );
  gl_Position = tmp;
  pos         = tmp;
  }
