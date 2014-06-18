//precision mediump float;

varying vec4 pos;
varying vec3 n;
varying vec3 bn;
varying vec3 tn;

varying vec2 tc;

uniform sampler2D texture;
uniform sampler2D bump;
uniform sampler2D heightMap;

uniform vec3 lightDir;

vec2 texCoord;
vec3 norm, bnorm, tnorm;
vec4 position;

vec3 normal(){
  vec3 nm = texture2D(bump, texCoord).xyz*2.0 - vec3(1.0);
  return normalize( nm.x*bnorm + nm.y*tnorm + nm.z*norm );
  }

float lambert(){
  return max( dot(normal(), lightDir), 0.0 );
  }

vec3 light(){
  return vec3( lambert()*0.8+0.2 );
  }

void parallax(){
  const int numsteps = 7;
  float scale    = 0.1;

  vec3 viewV = normalize( position.xyz );
  vec2 V = vec2( dot(viewV, bnorm), dot(viewV, tnorm) );

  float height = 0.0;
  float Bias   = -0.03 / float(numsteps);
  float Scale  = scale / float(numsteps);

  vec2 TexCoords = texCoord;

  for(int i = 0; i < numsteps; i++){
    height    = 1.0 - texture2D(heightMap, TexCoords).r;
    height    = height * Scale + Bias;
    TexCoords = TexCoords + height*V;
    }

  texCoord = TexCoords;
  }

void main() {
  position = pos;
  position.xyz /= position.w;

  norm  = normalize( n);
  bnorm = normalize(bn);
  tnorm = normalize(tn);
  texCoord = tc;

  parallax();

  vec4 diff    = texture2D(texture, texCoord);
  gl_FragColor = vec4( diff.xyz*light(), diff.w );
  }
