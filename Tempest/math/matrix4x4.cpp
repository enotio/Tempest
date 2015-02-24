#include "matrix4x4.h"
/*
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_projection.hpp>
#include <glm/gtc/type_ptr.hpp>
*/
#include <nv_math/nv_matrix.h>

#include <cmath>
#include <cstring>
#include <new>

#include <iostream>

using namespace Tempest;


Matrix4x4::Matrix4x4(){
    identity();
    }

Matrix4x4::Matrix4x4( const Matrix4x4& other ){
    setData( other.data() );
    }

Matrix4x4::Matrix4x4( const float data[/*16*/] ){
    setData( data );
    }

Matrix4x4::Matrix4x4( float a11, float a12, float a13, float a14,
                      float a21, float a22, float a23, float a24,
                      float a31, float a32, float a33, float a34,
                      float a41, float a42, float a43, float a44 ){
  setData(a11, a12, a13, a14,
          a21, a22, a23, a24,
          a31, a32, a33, a34,
          a41, a42, a43, a44 );
  }

Matrix4x4::~Matrix4x4(){
  }

Matrix4x4& Matrix4x4::operator = ( const Matrix4x4& other ){
  NvCopyMatf( m, other.m );
  return *this;
  }

bool Matrix4x4::operator == ( const Matrix4x4& other ) const{
  for( int i=0; i<4; ++i )
    for( int r=0; r<4; ++r )
      if( m[i][r]!=other.m[i][r] )
        return false;

  return true;
  }

void Matrix4x4::identity(){
  setData( 1, 0, 0, 0,
           0, 1, 0, 0,
           0, 0, 1, 0,
           0, 0, 0, 1 );
  }
/*
void Matrix4x4::translate( const float v[]){
  translate( v[0], v[1], v[2] );
  }

void Matrix4x4::translate(float x, float y, float z){
  //NvMultTranslateMatf( m, m, x, y, z );
  m[3][0] = m[0][0] * x + m[1][0] * y + m[2][0] * z + m[3][0];
  m[3][1] = m[0][1] * x + m[1][1] * y + m[2][1] * z + m[3][1];
  m[3][2] = m[0][2] * x + m[1][2] * y + m[2][2] * z + m[3][2];
  m[3][3] = m[0][3] * x + m[1][3] * y + m[2][3] * z + m[3][3];
  }*/
/*
void Matrix4x4::scale( float x ){
  scale(x, x, x);
  }

void Matrix4x4::scale(float x, float y, float z){
  //NvMultScaleMatf( m, m, x, y, z );
  m[0][0] *= x;
  m[0][1] *= x;
  m[0][2] *= x;
  m[0][3] *= x;

  m[1][0] *= y;
  m[1][1] *= y;
  m[1][2] *= y;
  m[1][3] *= y;

  m[2][0] *= z;
  m[2][1] *= z;
  m[2][2] *= z;
  m[2][3] *= z;
  }*/

void Matrix4x4::rotate(float angle, float x, float y, float z){
  float ax[3] = {x,y,z};
  NvMultRotDegMatf( m, m, ax, angle );
  }

void Matrix4x4::rotateOX( float angle ){
  //NvMultRotXDegMatf(m,m,angle);
  angle *= KD_DEG_TO_RAD_F;
  const GLfloat s = sinf(angle);
  const GLfloat c = cosf(angle);

  float r[2][4];
  r[0][0] = m[1][0] *  c + m[2][0] * s;
  r[0][1] = m[1][1] *  c + m[2][1] * s;
  r[0][2] = m[1][2] *  c + m[2][2] * s;
  r[0][3] = m[1][3] *  c + m[2][3] * s;

  r[1][0] = m[1][0] * -s + m[2][0] * c;
  r[1][1] = m[1][1] * -s + m[2][1] * c;
  r[1][2] = m[1][2] * -s + m[2][2] * c;
  r[1][3] = m[1][3] * -s + m[2][3] * c;

  memcpy(m[1],r[0],4*sizeof(float));
  memcpy(m[2],r[1],4*sizeof(float));
  }

void Matrix4x4::rotateOY( float angle ){
  //NvMultRotYDegMatf(m,m,angle);
  angle *= KD_DEG_TO_RAD_F;
  const GLfloat s = sinf(angle);
  const GLfloat c = cosf(angle);

  float r[2][4];
  r[0][0] = m[0][0] * c + m[2][0] * -s;
  r[0][1] = m[0][1] * c + m[2][1] * -s;
  r[0][2] = m[0][2] * c + m[2][2] * -s;
  r[0][3] = m[0][3] * c + m[2][3] * -s;

  r[1][0] = m[0][0] * s + m[2][0] * c;
  r[1][1] = m[0][1] * s + m[2][1] * c;
  r[1][2] = m[0][2] * s + m[2][2] * c;
  r[1][3] = m[0][3] * s + m[2][3] * c;

  memcpy(m[0],r[0],4*sizeof(float));
  memcpy(m[2],r[1],4*sizeof(float));
  }

void Matrix4x4::rotateOZ( float angle ){
  //NvMultRotZDegMatf(m,m,angle);

  angle *= KD_DEG_TO_RAD_F;
  const GLfloat s = sinf(angle);
  const GLfloat c = cosf(angle);
  //NvMultRotZDegMatf(m,m,angle);
  //rotate(angle, 0, 0, 1);
  float r[2][4];
  r[0][0] = m[0][0] * c + m[1][0] * s;
  r[0][1] = m[0][1] * c + m[1][1] * s;
  r[0][2] = m[0][2] * c + m[1][2] * s;
  r[0][3] = m[0][3] * c + m[1][3] * s;

  r[1][0] = m[0][0] * -s + m[1][0] * c;
  r[1][1] = m[0][1] * -s + m[1][1] * c;
  r[1][2] = m[0][2] * -s + m[1][2] * c;
  r[1][3] = m[0][3] * -s + m[1][3] * c;

  memcpy(m[0],r[0],4*sizeof(float));
  memcpy(m[1],r[1],4*sizeof(float));
  }

const float* Matrix4x4::data() const{
  return &m[0][0];
  }

float Matrix4x4::at(int x, int y) const {
  return m[x][y];
  }

void Matrix4x4::setData( const float data[/*16*/]){
  memcpy(m, data, 16*sizeof(float) );
  }

void Matrix4x4::transpose(){
  for( int i=0; i<4; ++i )
    for( int r=0; r<i; ++r )
      std::swap( m[i][r], m[r][i] );
  }

void Matrix4x4::inverse(){
  for( int i=0; i<4; ++i )
    for( int r=0; r<i; ++r )
      std::swap( m[i][r], m[r][i] );
  NvInvMatf( m, m );
  for( int i=0; i<4; ++i )
    for( int r=0; r<i; ++r )
      std::swap( m[i][r], m[r][i] );
  }

void Matrix4x4::mul( const Matrix4x4& other ){
  NvMultMatf( m, m, other.m );
  }

void Matrix4x4::setData( float a11, float a12, float a13, float a14,
                         float a21, float a22, float a23, float a24,
                         float a31, float a32, float a33, float a34,
                         float a41, float a42, float a43, float a44 ){
  m[0][0] = a11;
  m[1][0] = a12;
  m[2][0] = a13;
  m[3][0] = a14;

  m[0][1] = a21;
  m[1][1] = a22;
  m[2][1] = a23;
  m[3][1] = a24;

  m[0][2] = a31;
  m[1][2] = a32;
  m[2][2] = a33;
  m[3][2] = a34;

  m[0][3] = a41;
  m[1][3] = a42;
  m[2][3] = a43;
  m[3][3] = a44;
  }


void Matrix4x4::project( float   x, float   y, float   z, float   w,
                         float &ox, float &oy, float &oz, float &ow ) const {
  float r[4], a[4] = {x,y,z,w};

  NvTransformHomPointf(r, m, a);
  ox = r[0];
  oy = r[1];
  oz = r[2];
  ow = r[3];
  }

void Matrix4x4::perspective(float angle, float aspect, float zNear, float zFar) {
  float range = float(tan( 3.141592654*(double(angle) / 2.0)/180.0 ) * zNear);
  float left = -range * aspect;
  float right = range * aspect;
  float bottom = -range;
  float top = range;

  for( int i=0; i<4; ++i )
    for( int r=0; r<4; ++r )
       m[i][r] = 0;

  m[0][0] = (float(2) * zNear) / (right - left);
  m[1][1] = (float(2) * zNear) / (top - bottom);
  m[2][2] = - (zFar + zNear) / (zFar - zNear);
  m[2][3] = - float(1);
  m[3][2] = - (float(2) * zFar * zNear) / (zFar - zNear);

  m[2][3] = 1;
  m[2][2] = zFar/(zFar-zNear);
  m[3][2] = -zNear*zFar/(zFar-zNear);
  }

void Tempest::Matrix4x4::set(int x, int y, float v) {
  m[x][y] = v;
  }

void Tempest::Matrix4x4::project(float &x, float &y, float &z, float &w) const {
  project( x,y,z,w,
           x,y,z,w );
  }

void Matrix4x4::project(float &x, float &y, float &z) const {
  float w = 1;
  project(x,y,z,w);
  x /= w;
  y /= w;
  z /= w;
  }
