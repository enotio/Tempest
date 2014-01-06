attribute vec3 Position;
attribute vec2 TexCoord;

uniform mat4 mvpMatrix, mvpMatrixInv;
varying vec3 eyeray_o, eyeray_d;

void main(void) {
  eyeray_d     = Position;
  vec4 tmp     = (mvpMatrixInv * vec4(0.0, 0.0, 0.0, 1.0));
  eyeray_o     = tmp.xyz;

  gl_Position  = mvpMatrix * vec4(Position, 1.0);
  }
