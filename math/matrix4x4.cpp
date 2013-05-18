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
/*
//! ObjectMatrix::pimpl
//! А ты как думал?
class Matrix4x4::pimpl{
  public:
    //! http://glm.g-truc.net/
    GLfloat m[4][4];
    glm::detail::tmat4x4<float> gm;
  };
*/

Matrix4x4::Matrix4x4(){//:matrix( (pimpl*)&rimpl[0] ){
    //new(rimpl) Matrix4x4::pimpl();
    //assert( sizeof(pimpl)==sizeof(float)*16 );

    // std::cout <<" size = " << sizeof(pimpl) << std::endl;
    identity();
    }

Matrix4x4::Matrix4x4( const Matrix4x4& other ){
           //  :matrix( (pimpl*)&rimpl[0] ){
    // new(rimpl) Matrix4x4::pimpl();
    setData( other.data() );
    }

Matrix4x4::Matrix4x4( const float data[/*16*/] ){
           //  :matrix( (pimpl*)&rimpl[0] ){
    // new(rimpl) Matrix4x4::pimpl();
    setData( data );
    }

Matrix4x4::Matrix4x4( float a11, float a12, float a13, float a14,
                      float a21, float a22, float a23, float a24,
                      float a31, float a32, float a33, float a34,
                      float a41, float a42, float a43, float a44 ){
         //  :matrix( (pimpl*)&rimpl[0] ){
  // new(rimpl) Matrix4x4::pimpl();
  setData(a11, a12, a13, a14,
          a21, a22, a23, a24,
          a31, a32, a33, a34,
          a41, a42, a43, a44 );
  }

Matrix4x4::~Matrix4x4(){
  //delete matrix;
  }

Matrix4x4& Matrix4x4::operator = ( const Matrix4x4& other ){
  //*matrix = *other.matrix;
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

void Matrix4x4::translate( const float v[/*3*/]){
  translate( v[0], v[1], v[2] );
  }

void Matrix4x4::translate(float x, float y, float z){
  //matrix->gm = glm::translate( matrix->gm,
  //                             glm::detail::tvec3<float>( x, y, z) );

  NvMultTranslateMatf( m, m, x, y, z );

  check();
  }

void Matrix4x4::scale( float x ){
  scale(x, x, x);
  }

void Matrix4x4::scale(float x, float y, float z){
  //matrix->gm = glm::scale( matrix->gm,
  //                         glm::detail::tvec3<float>( x, y, z) );
  NvMultScaleMatf( m, m, x, y, z );

  check();
  }

void Matrix4x4::rotate(float angle, float x, float y, float z){
  //matrix->gm = glm::rotate( matrix->gm,
  //                          angle,
  //                          glm::detail::tvec3<float>( x, y, z) );

  float ax[3] = {x,y,z};
  NvMultRotDegMatf( m, m, ax, angle );

  check();
  }

void Matrix4x4::rotateOZ( float angle ){
  rotate(angle, 0, 0, 1);
  }

const float* Matrix4x4::data() const{
  return (const float*)m;//WTF?
  //return glm::value_ptr( matrix->gm );
  }

float Matrix4x4::at(int x, int y) const {
  return m[x][y];
  }

void Matrix4x4::setData( const float data[/*16*/]){
  memcpy(m, data, 16*sizeof(float) );

  /*
  for( int i=0; i<4; ++i )
    for( int r=0; r<4; ++r )
      matrix->gm[i][r] = matrix->m[i][r];
  */

  check();
  }

void Matrix4x4::transpose(){
  for( int i=0; i<4; ++i )
    for( int r=0; r<i; ++r )
      std::swap( m[i][r], m[r][i] );

  //matrix->gm = glm::transpose( matrix->gm );

  check();
  }

void Matrix4x4::inverse(){
  for( int i=0; i<4; ++i )
    for( int r=0; r<i; ++r )
      std::swap( m[i][r], m[r][i] );
  NvInvMatf( m, m );
  for( int i=0; i<4; ++i )
    for( int r=0; r<i; ++r )
      std::swap( m[i][r], m[r][i] );

  //matrix->gm = glm::inverse( matrix->gm );

  check();
  }

void Matrix4x4::mul( const Matrix4x4& other ){
  NvMultMatf( m, m, other.m );
  //matrix->gm *= other.matrix->gm;

  check();
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

  /*
  for( int i=0; i<4; ++i )
    for( int r=0; r<4; ++r )
      matrix->gm[i][r] = matrix->m[r][i];
      */

  check();
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

  float range = tan( M_PI*(angle / float(2))/180.0 ) * zNear;
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

  /*
  matrix->gm = glm::perspective( angle, aspect, zNear, zFar );
  matrix->gm[2][3] = 1;
  matrix->gm[2][2] = zFar/(zFar-zNear);
  matrix->gm[3][2] = -zNear*zFar/(zFar-zNear);*/

  check();
  }

void Tempest::Matrix4x4::set(int x, int y, float v) {
  m[x][y]  = v;
  // matrix->gm[x][y] = v;

  check();
  }

void Matrix4x4::check() const {
  /*
  return;

  for( int i=0; i<16; ++i ){
    if( ((const float*)matrix->m)[i] != glm::value_ptr( matrix->gm )[i] ){
      for( int r=0; r<16; ++r )
        std::cout << ((const float*)matrix->m)[r] <<" ";
      std::cout << std::endl;

      for( int r=0; r<16; ++r )
        std::cout << glm::value_ptr( matrix->gm )[r] <<" ";
      std::cout << std::endl;

      //assert(0);
      }
    }*/
  }
