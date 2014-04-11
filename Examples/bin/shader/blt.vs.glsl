attribute vec4 Position;
attribute vec2 TexCoord;

varying vec2 tc;

void main(void) {
  gl_Position = Position;
  tc          = vec2( TexCoord.x, 1.0-TexCoord.y );
  }
