attribute vec3 Position;
attribute vec2 TexCoord;

uniform mat4 mvpMatrix;
varying vec2 tc;

void main() {
  tc           = TexCoord;
  gl_Position  = mvpMatrix * vec4(Position, 1.0);
  }
