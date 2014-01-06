varying vec3 eyeray_o, eyeray_d;

uniform sampler3D volume;

struct Ray {
  vec3 o;   // origin
  vec3 d;   // direction
  };

// calculate intersection between ray and box
// http://www.siggraph.org/education/materials/HyperGraph/raytrace/rtinter3.htm
bool intersectBox( Ray r,
                   vec3 boxmin,
                   vec3 boxmax,
                   out float tnear,
                   out float tfar ){
  // compute intersection of ray with all six bbox planes
  vec3 invR = 1.0 / r.d;
  vec3 tbot = invR * (boxmin.xyz - r.o);
  vec3 ttop = invR * (boxmax.xyz - r.o);

  // re-order intersections to find smallest and largest on each axis
  vec3 tmin = min (ttop, tbot);
  vec3 tmax = max (ttop, tbot);

  // find the largest tmin and the smallest tmax
  vec2 t0             = max (tmin.xx, tmin.yz);
  float largest_tmin  = max (t0.x, t0.y);
  t0                  = min (tmax.xx, tmax.yz);
  float smallest_tmax = min (t0.x, t0.y);

  // check for hit
  bool hit = (largest_tmin <= smallest_tmax);
  tnear = largest_tmin;
  tfar  = smallest_tmax;

  return hit;
  }

vec4 color( vec3 pos ){
  vec4 c = texture3D(volume, pos );
  return vec4(c.a);
  }

void main(void) {
  const int steps = 128;
  float stepsize  = 1.0 / float(steps);

  Ray eyeray;
  eyeray.o = eyeray_o.xyz;
  eyeray.d = eyeray_d.xyz - eyeray.o;

  eyeray.d = normalize(eyeray.d);

  float tnear, tfar;
  bool hit = intersectBox(eyeray, -vec3(1), vec3(1), tnear, tfar);
  if (!hit)
    discard;

  if (tnear < 0.0)
    tnear = 0.0;

  vec3 Pnear = eyeray.o + eyeray.d*tnear;
  vec3 Pfar  = eyeray.o + eyeray.d*tfar;

  Pnear = (Pnear/2.0 + 0.5);
  Pfar  = (Pfar /2.0 + 0.5);

  vec4 c = vec4(0.0);
  vec3 P = Pfar;
  vec3 Pstep = (Pnear-Pfar)*stepsize;

  for(int i=0; i<steps; i++) {
    vec4 s = color(P);
    c += s;

    P += Pstep;
    }

  //if( c.a==0.0 )
    //discard;

  gl_FragColor = c;
  }
