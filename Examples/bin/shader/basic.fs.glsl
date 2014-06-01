uniform sampler2D xtexture;
varying vec2 tc;

void main(void) {
  gl_FragColor = texture2D( xtexture, tc );
  }
