precision mediump float;

uniform sampler2D texture;
varying vec2 tc;

void main() {
  gl_FragColor = texture2D( texture, tc );
  }
