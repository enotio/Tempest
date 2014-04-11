uniform sampler2D image;
varying vec2 tc;

uniform vec2 dir;

vec4 color = vec4(0.0);

void tx( float offset, float weight ){
  color += texture2D( image, tc + offset*dir )*weight;
  }

void main(void) {
  tx( -3.2307692308, 0.0702702703 );
  tx( -1.3846153846, 0.3162162162 );
  tx(  0.0,          0.2270270270 );
  tx(  1.3846153846, 0.3162162162 );
  tx(  3.2307692308, 0.0702702703 );

  gl_FragColor = color;
  }
