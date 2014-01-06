attribute vec3 Position;
attribute vec2 TexCoord;

uniform mat4 mvpMatrix, mvpMatrixInv;
varying vec4 eyeray_o, eyeray_d;

void main(void) {
  eyeray_d     = vec4(Position, 1.0);
  eyeray_o     = mvpMatrixInv * vec4(0.0, 0.0, 0.0, 1.0);

  gl_Position  = mvpMatrix * vec4(Position, 1.0);
  }
