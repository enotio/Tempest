#include "camera.h"

#include <Tempest/Matrix4x4>
#include <utility>

using namespace Tempest;

Camera::Camera(){
  dist   = 4;
  pos[0] = 0;
  pos[1] = 0;
  pos[2] = 0;

  sx = 0;
  sy = 0;

  m_zoom = 1;
  updateView();

  setPerspective(0);
  }

Matrix4x4 Camera::view() const {
  return mView;
  }

void Camera::setPosition( double x, double y, double z ){
  pos[0] = x;
  pos[1] = y;
  pos[2] = z;

  updateView();
  }

double Camera::x() const{
  return pos[0];
  }

double Camera::y() const{
  return pos[1];
  }

double Camera::z() const{
  return pos[2];
  }

void Camera::setDistance(double d) {
  dist = d;

  updateView();
  }

double Camera::distance() const {
  return dist;
  }

void Camera::updateView(){
  mView.identity();
  mView.translate( 0,0, float(dist) );

  mView.rotate( float(sy), 1,0,0 );
  mView.rotate( float(sx), 0,0,1 );
  mView.scale( float(m_zoom) );
  mView.translate( -float(pos[0]), -float(pos[1]), -float(pos[2]) );
  }

Matrix4x4 Camera::projective() const {
  return mProj;
  }

void Camera::setSpinX( double x ){
  sx = x;
  updateView();
  }

void Camera::setSpinY( double y ){
  sy = y;
  updateView();
  }

void Camera::setZoom( double z ){
  m_zoom = z;
  updateView();
  }

void Camera::setPerspective( bool use, int w, int h ){
  if( use )
    setPerspective( w,h ); else
    mProj.identity();
  }

void Camera::setPerspective( int w, int h, float use, float zmin, float zmax ) {
  if( zmin>zmax )
    std::swap(zmin, zmax);

  mProj.perspective( use, float(double(w)/double(h)), zmin, zmax );
  }

double Camera::spinX() const{
  return sx;
  }

double Camera::spinY() const{
  return sy;
  }

double Camera::zoom() const{
  return m_zoom;
  }
