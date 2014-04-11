attribute vec3 Position;
attribute vec2 TexCoord;
attribute vec3 Normal;

varying vec4 pos, shPos;
varying vec3 norm;
varying vec2 tc;

uniform mat4 mvpMatrix, modelView, shadowMatrix;

void main() {
  vec4 v = modelView*vec4( normalize(Normal), 0.0);
  norm   = v.xyz;
  tc     = TexCoord;

#ifdef shadow_pass
  v     = shadowMatrix*vec4( Position, 1.0 );
  shPos = v;
#else
  shPos = shadowMatrix*vec4( Position, 1.0 );
  v     =    mvpMatrix*vec4( Position, 1.0 );
#endif

  gl_Position = v;
  pos         = v;
  }
