uniform sampler2D image;
varying vec2 tc;

uniform vec2 dir;

void main(void) {
  vec4 color = texture2D( image, tc );

  float l = dot( vec3(0.3, 0.59, 0.11), color.rgb );

  l = clamp( (l-0.6)/0.1, 0.0, 1.0 );
  gl_FragColor = color*l;
  }
