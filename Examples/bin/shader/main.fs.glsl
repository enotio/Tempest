//precision mediump float;

varying vec4 pos, shPos;
varying vec3 norm;
varying vec2 tc;

uniform vec3 lightDir;
uniform sampler2D shadow;

uniform mat4 projMatrix;
uniform sampler2D diffuse;
uniform sampler2D screen;

uniform vec4 matColor;

vec3 viewVec;
vec3 normal;
vec3 shPosition;

float light(){
  return max( dot( normal, lightDir ), 0.0  );
  }

float blinn(){
  vec3 ref = reflect(lightDir, normal);
  float v  = max( dot(ref, viewVec), 0.0 );
  return pow(v, 64.0);
  }

float shadowVal( vec2 ddx ){
  vec2 tc = (shPosition.xy)*0.5+vec2(0.5);
  vec4 sh = texture2D(shadow, tc+ddx);

  bool border = ( 0.0<tc.x && tc.x<1.0 &&
                  0.0<tc.y && tc.y<1.0 );

  return border ? step(shPosition.z, sh.r/sh.g+0.001):1.0;
  }

float shadowDiff(){
  vec2 tc = (shPosition.xy)*0.5+vec2(0.5);
  vec4 sh = texture2D(shadow, tc);

  return shPosition.z - sh.r/sh.g;
  }

float shadowValPcf( float ksize, float nom ){
  vec2 dx = vec2(nom, 0.0)/ksize;
  vec2 dy = vec2(0.0, dx.x);

  float v = shadowVal( +dx+dy ) +
            shadowVal( -dx+dy ) +
            shadowVal( +dx-dy ) +
            shadowVal( -dx-dy );

  return v/4.0;
  }

float shadowValPcf( float r ){
  float v = shadowValPcf(1024.0,r) +
            shadowValPcf( 512.0,r) +
            shadowValPcf( 256.0,r);
  return v/3.0;
  }

vec3 phong( float sh ){
#ifndef emissionv
  return vec3( light()*sh )*0.7 + 0.3;
#else
  return vec3( mix( light()*sh, 1.0, emissionv) )*0.7 + 0.3;
#endif
  }

vec3 color(){
  float r  = 1.0;//shadowDiff();
  float sh = shadowValPcf(r);
  //float sh = shadowVal(vec2(0.0) );
  return phong(sh) + sh*blinn()*vec3(1.0);
  }

vec3 refractv(){
#ifdef shadow_pass
  return normal;
#else
  vec3 ref = refract(viewVec, normal, 0.6);
  vec4 rx = projMatrix*vec4(ref, 0.0);
  return normalize(rx.xyz);
#endif
  }

vec4 shPass(){
  vec3 x =  pos.xyz/pos.w;
  x.xy = x.xy*0.5 + vec2(0.5);

  return vec4( pos.z, pos.w, 0.0, 0.0 );
  }

vec4 mainPass(){
  shPosition = shPos.xyz/shPos.w;
  normal  = normalize(norm);
  vec3 npos = pos.xyz/pos.w;
  viewVec = normalize( npos );

#ifdef shadow_pass
  vec3 cl = shPass().rgb;
#else
#ifdef no_texture
  vec3 cl = matColor.rgb;
#else
  vec3 cl = texture2D(diffuse, tc).rgb*matColor.rgb;
#endif
#endif

#ifdef refractions
  {
  vec3 sn  = refractv();
  vec4 ncl = texture2D( screen, 0.5*pos.xy/pos.w+vec2(0.5) );
  float d  = ncl.w - npos.z;
  cl = vec3( sn.xyz*0.5 + vec3(0.5) );
  vec4 clx = texture2D( screen, 0.5*pos.xy/pos.w+vec2(0.5) + 4.0*sn.xy*d );

  cl = clx.w>npos.z ? clx.rgb:ncl.rgb;
  }
#endif
  //return vec4( shadowDiff() );
  return vec4( color()*cl.rgb, npos.z );
  }

void main() {
#ifdef shadow_pass
  gl_FragColor = shPass();
#else
  gl_FragColor = mainPass();
#endif
  }
