attribute vec3 Position;
attribute vec2 TexCoord;
attribute vec3 Normal;
attribute vec3 BiNormal;

varying vec4 pos;
varying vec3 n;
varying vec3 bn;
varying vec3 tn;

varying vec2 tc;

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
