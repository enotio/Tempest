#include "tessobject.h"

#include <cmath>

Tempest::RawModel<> Tempest::TessObject::sphere( int passCount, double R ){
  Tempest::RawModel<>::VertexList r;

  static const double pi = 3.141592654;

  Tempest::DefaultVertex v1 = {
    1, 0, -0.5,
    0,0, {0,0,0}
    };
  Tempest::DefaultVertex v2 = {
    (float)cos(2*pi/3), (float)sin(2*pi/3), -0.50,
    0,0, {0,0,0}
    };
  Tempest::DefaultVertex v3 = {
    (float)cos(2*pi/3), -(float)sin(2*pi/3), -0.5,
    0,0, {0,0,0}
    };

  Tempest::DefaultVertex v4 = {
    0, 0, 0.5,
    0,0, {0,0,0}
    };

  r.push_back(v1);
  r.push_back(v3);
  r.push_back(v2);

  r.push_back(v1);
  r.push_back(v2);
  r.push_back(v4);

  r.push_back(v2);
  r.push_back(v3);
  r.push_back(v4);

  r.push_back(v1);
  r.push_back(v4);
  r.push_back(v3);

  for( size_t i=0; i<r.size(); ++i ){
    Tempest::DefaultVertex & v = r[i];
    double l = sqrt(v.x*v.x + v.y*v.y + v.z*v.z);

    v.x /= l;
    v.y /= l;
    v.z /= l;
    }

  for(int c=0; c<passCount; ++c){
    size_t maxI = r.size();
    for( size_t i=0; i<maxI; i+=3 ){
      Tempest::DefaultVertex x = {
        0.5f*(r[i].x+r[i+1].x),
        0.5f*(r[i].y+r[i+1].y),
        0.5f*(r[i].z+r[i+1].z),
        0,0, {0,0,0}
        };
      Tempest::DefaultVertex y = {
        0.5f*(r[i+2].x+r[i+1].x),
        0.5f*(r[i+2].y+r[i+1].y),
        0.5f*(r[i+2].z+r[i+1].z),
        0,0, {0,0,0}
        };
      Tempest::DefaultVertex z = {
        0.5f*(r[i].x+r[i+2].x),
        0.5f*(r[i].y+r[i+2].y),
        0.5f*(r[i].z+r[i+2].z),
        0,0, {0,0,0}
        };

      r.push_back( r[i] );
      r.push_back( x );
      r.push_back( z );

      r.push_back( x );
      r.push_back( r[i+1] );
      r.push_back( y );

      r.push_back( y );
      r.push_back( r[i+2] );
      r.push_back( z );

      r[i]   = x;
      r[i+1] = y;
      r[i+2] = z;
      }

    for( size_t i=0; i<r.size(); ++i ){
      Tempest::DefaultVertex & v = r[i];
      double l = sqrt(v.x*v.x + v.y*v.y + v.z*v.z);

      v.x /= l;
      v.y /= l;
      v.z /= l;
      }
    }


  for( size_t i=0; i<r.size(); ++i ){
    Tempest::DefaultVertex & v = r[i];
    v.normal[0] = v.x;
    v.normal[1] = v.y;
    v.normal[2] = v.z;

    v.x *= R;
    v.y *= R;
    v.z *= R;
    }

  Tempest::RawModel<> raw;
  raw.vertex   = r;
  raw.hasIndex = false;
  return raw;
  }
