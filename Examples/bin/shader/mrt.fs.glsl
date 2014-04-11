uniform sampler2D texture;
varying vec2 tc;

void main() {
  gl_FragData[0] = texture2D( texture, tc );
  gl_FragData[1] = vec4(tc, 1.0, 1.0);
  //gl_FragColor = texture2D( texture, tc );
  }
