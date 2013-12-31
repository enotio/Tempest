uniform sampler2D texture;
varying vec2 tc;

void main(void) {
  gl_FragColor = texture2D( texture, tc );
  }
