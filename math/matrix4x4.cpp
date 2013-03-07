#include "matrix4x4.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_projection.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cmath>
#include <cstring>
#include <new>

// #include <iostream>

using namespace Tempest;

//! ObjectMatrix::pimpl
//! А ты как думал?
class Matrix4x4::pimpl{
  public:
    //! http://glm.g-truc.net/
    glm::detail::tmat4x4<double> m;
  };

Matrix4x4::Matrix4x4():matrix( (pimpl*)&rimpl[0] ){
    new(rimpl) Matrix4x4::pimpl();
    assert( sizeof(pimpl)==sizeof(double)*16 );

    // std::cout <<" size = " << sizeof(pimpl) << std::endl;
    identity();
    }

Matrix4x4::Matrix4x4( const Matrix4x4& other )
             :matrix( (pimpl*)&rimpl[0] ){
    new(rimpl) Matrix4x4::pimpl();
    setData( other.data() );
    }

Matrix4x4::Matrix4x4( const double data[/*16*/] )
             :matrix( (pimpl*)&rimpl[0] ){
    new(rimpl) Matrix4x4::pimpl();
    setData( data );
    }

Matrix4x4::Matrix4x4( double a11, double a12, double a13, double a14,
                      double a21, double a22, double a23, double a24,
                      double a31, double a32, double a33, double a34,
                      double a41, double a42, double a43, double a44 )
             :matrix( (pimpl*)&rimpl[0] ){
    new(rimpl) Matrix4x4::pimpl();
    setData(a11, a12, a13, a14,
            a21, a22, a23, a24,
            a31, a32, a33, a34,
            a41, a42, a43, a44 );
    }

Matrix4x4::~Matrix4x4(){
    //delete matrix;
    }

Matrix4x4& Matrix4x4::operator = ( const Matrix4x4& other ){
    *matrix = *other.matrix;
    return *this;
    }

bool Matrix4x4::operator == ( const Matrix4x4& other ) const{
    for( int i=0; i<16; ++i )
      if( rimpl[i]!=other.rimpl[i] )
        return false;

    return true;
    }

void Matrix4x4::identity(){
    setData( 1, 0, 0, 0,
             0, 1, 0, 0,
             0, 0, 1, 0,
             0, 0, 0, 1 );
    }

void Matrix4x4::translate( const double v[/*3*/]){
    translate( v[0], v[1], v[2]);
    }

void Matrix4x4::translate(double x, double y, double z){
    matrix->m = glm::translate( matrix->m,
                                glm::detail::tvec3<double>( x, y, z) );
    }

void Matrix4x4::scale( double x ){
    scale(x, x, x);
    }

void Matrix4x4::scale(double x, double y, double z){
    matrix->m = glm::scale( matrix->m,
                            glm::detail::tvec3<double>( x, y, z) );
    }

void Matrix4x4::rotate(double angle, double x, double y, double z){
    matrix->m = glm::rotate( matrix->m,
                             angle,
                             glm::detail::tvec3<double>( x, y, z) );
    }

void Matrix4x4::rotateOZ( double angle ){
    rotate(angle, 0, 0, 1);
    }

const double* Matrix4x4::data() const{
  return glm::value_ptr( matrix->m );
  }

double Matrix4x4::at(int x, int y) const {
  return matrix->m[x][y];
  }

void Matrix4x4::setData( const double data[/*16*/]){/*
    for(int i=0; i<4; ++i)
      for(int r=0; r<4; ++r)
        matrix->m[i][r] = data[i*4+r];*/
    memcpy(glm::value_ptr(matrix->m), data, 16*sizeof(double) );
    }

void Matrix4x4::transpose(){
    matrix->m = glm::transpose( matrix->m );
    }

void Matrix4x4::inverse(){
    matrix->m = glm::inverse( matrix->m );
    }

void Matrix4x4::mul( const Matrix4x4& other ){
    //matrix->m = other.matrix->m * matrix->m;
    matrix->m *= other.matrix->m;
    //matrix->m = other.matrix->m;
    }

void Matrix4x4::setData( double a11, double a12, double a13, double a14,
                         double a21, double a22, double a23, double a24,
                         double a31, double a32, double a33, double a34,
                         double a41, double a42, double a43, double a44 ){
    matrix->m[0][0] = a11;
    matrix->m[0][1] = a12;
    matrix->m[0][2] = a13;
    matrix->m[0][3] = a14;

    matrix->m[1][0] = a21;
    matrix->m[1][1] = a22;
    matrix->m[1][2] = a23;
    matrix->m[1][3] = a24;

    matrix->m[2][0] = a31;
    matrix->m[2][1] = a32;
    matrix->m[2][2] = a33;
    matrix->m[2][3] = a34;

    matrix->m[3][0] = a41;
    matrix->m[3][1] = a42;
    matrix->m[3][2] = a43;
    matrix->m[3][3] = a44;
    }


void Matrix4x4::project( double   x, double   y, double   z, double   w,
                         double &ox, double &oy, double &oz, double &ow ) const {
  glm::detail::tvec4<double> r = (matrix->m)*glm::detail::tvec4<double>(x,y,z,w);

  ox = r.x;
  oy = r.y;
  oz = r.z;
  ow = r.w;
  }

void Matrix4x4::perspective(double angle, double aspect, double zNear, double zFar) {
  //glm::detail::tvec4<double> r = glm::detail::tvec4<double>(x,y,z,w)*(matrix->m);
  matrix->m = glm::perspective( angle, aspect, zNear, zFar );

  matrix->m[2][3] = 1;
  matrix->m[2][2] = zFar/(zFar-zNear);
  matrix->m[3][2] = -zNear*zFar/(zFar-zNear);
  }

void Tempest::Matrix4x4::set(int x, int y, double v) const {
  matrix->m[x][y] = v;
  }
