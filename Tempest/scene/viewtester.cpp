#include "viewtester.h"

#include <Tempest/AbstractGraphicObject>
#include <Tempest/AbstractCamera>
#include <Tempest/Matrix4x4>

#include <cmath>

using namespace Tempest;

ViewTester::ViewTester() {
  }

ViewTester::~ViewTester() {
  }

bool ViewTester::isVisible( const AbstractSceneObject &obj,
                            const AbstractCamera &c ) const {
  if( !obj.isVisible() )
    return false;

  Tempest::Matrix4x4 m = c.projective();
  m.mul( c.view() );
  m.mul( obj.transform() );

  return isVisible( obj, m );
  }

bool ViewTester::isVisible( const AbstractSceneObject &obj,
                            const Matrix4x4 &mvp ) const {
  if( !obj.isVisible() )
    return false;

  return isVisible( obj.bounds(), mvp );
  }

bool ViewTester::isVisible( const ModelBounds &obj,
                            const Matrix4x4 &m ) const {
  //Matrix4x4 m = mvp;
  //m.transpose();

  float left[4] = { m.at(0,0), m.at(1,0), m.at(2,0), 0 };
  float  top[4] = { m.at(0,1), m.at(1,1), m.at(2,1), 0 };
  float data1[4], data2[4];

  for( int r=0; r<3; ++r ){
    left[3] += left[r]*left[r];
    top[3]  +=  top[r]* top[r];
    }

  left[3] = sqrt(left[3]);
  top [3] = sqrt(top[3]);

  float rad = obj.radius();

  for( int r=0; r<3; ++r ){
    left[r] /= left[3];
    top [r] /=  top[3];

    left[r] = rad*(left[r]+top[r]);
    }

  m.project( left[0] + obj.mid[0],
             left[1] + obj.mid[1],
             left[2] + obj.mid[2], 1,
             data1[0], data1[1], data1[2], data1[3] );

  m.project( -left[0] + obj.mid[0],
             -left[1] + obj.mid[1],
             -left[2] + obj.mid[2], 1,
             data2[0], data2[1], data2[2], data2[3] );

  data1[3] = fabs(data1[3]);
  data2[3] = fabs(data2[3]);

  for( int i=0; i<2; ++i ){
    data1[i] /= data1[3];
    data2[i] /= data2[3];
    }

  for( int i=0; i<2; ++i ){
    if( data1[i] < -1 )
      data1[i] = -1;

    if( data2[i] < -1 )
      data2[i] = -1;

    if( data1[i] > 1 )
      data1[i] = 1;

    if( data2[i] > 1 )
      data2[i] = 1;
    }

  return data2[0]<data1[0] && data2[1]<data1[1];
  }

bool ViewTester::isVisible( float x,
                            float y,
                            float z,
                            float r,
                            const Frustum &frustum ) const {
  return checkVisible(x,y,z,r, frustum)!=NotVisible;
  }

bool ViewTester::checkVisible( float x,
                               float y,
                               float z,
                               float r,
                               const Frustum &frustum) const {
  bool fv = true;
  for( int p=0; p < 6; p++ ){
    float l = frustum.plane(p)[0] * x +
              frustum.plane(p)[1] * y +
              frustum.plane(p)[2] * z +
              frustum.plane(p)[3];
    if( l <= -r )
      return NotVisible;

    if( !(l >= r) )
      fv = false;
    }

  if( fv )
    return FullVisible; else
    return PartialVisible;
  }

