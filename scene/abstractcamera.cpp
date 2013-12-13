#include "abstractcamera.h"

#include <Tempest/Matrix4x4>
#include <cmath>
#include <algorithm>

using namespace Tempest;

Frustum::Frustum() {
  Tempest::Matrix4x4 m;
  m.identity();
  fromMatrix( m );
  }

Frustum::Frustum(const Tempest::AbstractCamera &c) {
  Tempest::Matrix4x4 cl = c.projective();
  cl.mul( c.view() );
  fromMatrix(cl);
  }

Frustum::Frustum(const Tempest::Matrix4x4 &c){
  fromMatrix(c);
  }

void Frustum::fromMatrix( const Tempest::Matrix4x4 &cl ) {
  float clip[16], t;
  std::copy( cl.data(), cl.data()+16, clip );

  f[0][0] = clip[ 3] - clip[ 0];
  f[0][1] = clip[ 7] - clip[ 4];
  f[0][2] = clip[11] - clip[ 8];
  f[0][3] = clip[15] - clip[12];

  t = sqrt( f[0][0] * f[0][0] + f[0][1] * f[0][1] + f[0][2] * f[0][2] );

  f[0][0] /= t;
  f[0][1] /= t;
  f[0][2] /= t;
  f[0][3] /= t;

  f[1][0] = clip[ 3] + clip[ 0];
  f[1][1] = clip[ 7] + clip[ 4];
  f[1][2] = clip[11] + clip[ 8];
  f[1][3] = clip[15] + clip[12];

  t = sqrt( f[1][0] * f[1][0] + f[1][1] * f[1][1] + f[1][2] * f[1][2] );

  f[1][0] /= t;
  f[1][1] /= t;
  f[1][2] /= t;
  f[1][3] /= t;

  f[2][0] = clip[ 3] + clip[ 1];
  f[2][1] = clip[ 7] + clip[ 5];
  f[2][2] = clip[11] + clip[ 9];
  f[2][3] = clip[15] + clip[13];

  t = sqrt( f[2][0] * f[2][0] + f[2][1] * f[2][1] + f[2][2] * f[2][2] );

  f[2][0] /= t;
  f[2][1] /= t;
  f[2][2] /= t;
  f[2][3] /= t;

  f[3][0] = clip[ 3] - clip[ 1];
  f[3][1] = clip[ 7] - clip[ 5];
  f[3][2] = clip[11] - clip[ 9];
  f[3][3] = clip[15] - clip[13];

  t = sqrt( f[3][0] * f[3][0] + f[3][1] * f[3][1] + f[3][2] * f[3][2] );

  f[3][0] /= t;
  f[3][1] /= t;
  f[3][2] /= t;
  f[3][3] /= t;

  f[4][0] = clip[ 3] - clip[ 2];
  f[4][1] = clip[ 7] - clip[ 6];
  f[4][2] = clip[11] - clip[10];
  f[4][3] = clip[15] - clip[14];

  t = sqrt( f[4][0] * f[4][0] + f[4][1] * f[4][1] + f[4][2] * f[4][2] );

  f[4][0] /= t;
  f[4][1] /= t;
  f[4][2] /= t;
  f[4][3] /= t;

  f[5][0] = clip[ 3] + clip[ 2];
  f[5][1] = clip[ 7] + clip[ 6];
  f[5][2] = clip[11] + clip[10];
  f[5][3] = clip[15] + clip[14];

  t = sqrt( f[5][0] * f[5][0] + f[5][1] * f[5][1] + f[5][2] * f[5][2] );

  f[5][0] /= t;
  f[5][1] /= t;
  f[5][2] /= t;
  f[5][3] /= t;
  }

const float *Frustum::plane(int i) const {
  return f[i];
  }

Tempest::Matrix4x4 Tempest::AbstractCamera::viewProjective() const {
  Matrix4x4 vp = projective();
  vp.mul( view() );

  return vp;
  }

Frustum AbstractCamera::frustum() const {
  return Frustum( viewProjective() );
  }
