varying vec2 tc;

uniform sampler2D image;
uniform sampler2D bloom0;
uniform sampler2D bloom1;
uniform sampler2D bloom2;
uniform sampler2D bloom3;

void main(void) {
  vec4 color = texture2D( image, tc );

  color += texture2D( bloom0, tc )*0.1;
  color += texture2D( bloom1, tc )*0.2;
  color += texture2D( bloom2, tc )*0.3;
  color += texture2D( bloom3, tc )*0.6;

  gl_FragColor = color;
  }
