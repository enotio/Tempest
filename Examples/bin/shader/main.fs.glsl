//precision mediump float;

varying vec4 pos,  shPos;
varying vec3 norm, shNormal;
varying vec2 tc;

uniform vec3 lightDir;
uniform sampler2D shadow;

uniform mat4 projMatrix, invMvp;
uniform sampler2D diffuse;
uniform sampler2D screen;

#ifdef use_rsm
uniform sampler2D rsmColor;
uniform sampler2D rsmNormal;
#endif

uniform vec4 matColor;

vec3 viewVec;
vec3 normal;//screeen-space
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

  //return sh.r/sh.g - shPosition.z;
  return border ? (1.0-step(-shPosition.z, sh.r/sh.g-0.001)) : 1.0;
  }

float shadowDiff(){
#ifdef use_shadow_map
  vec2 tc = (shPosition.xy)*0.5+vec2(0.5);
  vec4 sh = texture2D(shadow, tc);

  return shPosition.z - sh.r/sh.g;
#else
  return 0.0;
#endif
  }

float shadowValPcf( float ksize, float nom ){
#ifdef use_shadow_map
  vec2 dx = vec2(nom, 0.0)/ksize;
  vec2 dy = vec2(0.0, dx.x);

  float v = shadowVal( +dx+dy ) +
            shadowVal( -dx+dy ) +
            shadowVal( +dx-dy ) +
            shadowVal( -dx-dy );

  return v/4.0;
#else
  return 1.0;
#endif
  }

float shadowValPcf( float r ){
#ifdef use_shadow_map
  float v = shadowValPcf(1024.0,r) +
            shadowValPcf( 512.0,r) +
            shadowValPcf( 256.0,r);
  return v/3.0;
#else
  return 1.0;
#endif
  }

vec4 shPass(){
  vec3 x =  pos.xyz/pos.w;
  x.xy = x.xy*0.5 + vec2(0.5);

  return vec4( -pos.z, pos.w, 0.0, 0.0 );
  }

vec3 colorDiff(){
#ifdef shadow_out
  return shPass().rgb;
#else
#ifdef no_texture
  return matColor.rgb;
#else
  return texture2D(diffuse, tc).rgb*matColor.rgb;
#endif
#endif
  return vec3(0.0);
  }

vec3 reconstructPos( vec2 tc, float depth ){
  //vec2 tc = shPosition.xy;
  vec4 v = invMvp*vec4( tc.x, tc.y, depth, 1.0);
  //v.xyz /= v.w;
  //v = objectMat*vec4(v.xyz, 1.0);
  return v.xyz/v.w;
  }

#ifdef use_rsm
vec4 rsm(){
  vec3 cl = vec3(0.0);
  vec2 tc = (shPosition.xy)*0.5+vec2(0.5);

  vec4 norm   = texture2D( rsmNormal, tc );
  norm.xyz    = norm.xyz*2.0-vec3(1.0);
  vec3 curPos = 0.01*reconstructPos(shPosition.xy, shPosition.z);
  //return vec4(curPos,norm.w);

  const int n = 5;
  vec4 nx;
  vec3 d, px;
  vec3 n0 = normalize(shNormal);

  for( int i=-n; i<=n; ++i )
    for( int r=-n; r<=n; ++r ){
      vec2 crd = tc+vec2(i,r)*0.02;
      nx  = texture2D( rsmNormal, crd );
      px  = reconstructPos( crd*2.0-vec2(1.0), nx.w);
      d   = ( curPos-px );

      nx.xyz   = nx.xyz*2.0-vec3(1.0);

      //float k = dot(norm.xyz,d.xyz);
      float k = 1.0-max(dot(-n0.xyz,nx.xyz), 0.0);//*length(d);
      cl += k*texture2D( rsmColor, crd ).rgb;
      }

  cl *= colorDiff();
  return vec4( cl/float((n*2+1)*(n*2+1)), 0.0 );
  //return vec4(0.0);
  }
#endif

vec3 phong( float sh ){
  float ablimient = 0.3;
#ifndef emissionv
  return vec3( light()*sh )*0.7;// + ablimient;
#else
  return vec3( mix( light()*sh, 1.0, emissionv) );// + ablimient;
#endif
  }

vec3 color(){
  float r  = 1.0;//shadowDiff();
  float sh = shadowValPcf(r);
  //float sh = shadowVal(vec2(0.0) );
  return phong(sh) + sh*blinn()*vec3(1.0);
  }

vec3 refractv(){
#ifdef shadow_out
  return normal;
#else
  vec3 ref = refract(viewVec, normal, 0.6);
  vec4 rx = projMatrix*vec4(ref, 0.0);
  return normalize(rx.xyz);
#endif
  }

void setupGlobals(){
  shPosition = shPos.xyz/shPos.w;
  normal  = normalize(norm);
  vec3 npos = pos.xyz/pos.w;
  viewVec = normalize( npos );
  }

vec4 mainPass(){
  vec3 npos = pos.xyz/pos.w;
  vec3 cl = colorDiff();

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

vec3 pack( vec3 v ){
  return v*0.5+vec3(0.5);
  }

void main() {
  setupGlobals();

  vec4 cl;
#ifdef shadow_out
  cl = shPass();
#else
#ifdef use_rsm
  //cl = rsm();
  cl = mainPass() + rsm();
#else
  cl = mainPass();
#endif
#endif

#ifdef gbuffer
  gl_FragData[0] = cl;
  gl_FragData[1] = vec4( colorDiff(),  1.0 );
  gl_FragData[2] = vec4( pack(normal), shPosition.z);
#else
  gl_FragColor = cl;
#endif
  }
