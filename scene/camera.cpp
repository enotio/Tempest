#include "camera.h"

#include <Tempest/Matrix4x4>

using namespace Tempest;

Camera::Camera(){
  setPosition(0,0,0);

  setSpinX(0);
  setSpinY(0);

  setZoom(1);
  setPerespective(0);

  updateView();
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
  mView.translate( 0,0, dist );

  mView.rotate( sy, 1,0,0 );
  mView.rotate( sx, 0,0,1 );
  mView.translate( -pos[0], -pos[1], -pos[2] );

  mView.scale( m_zoom );
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

void Camera::setPerespective( bool use, int w, int h ){
  if( use )
    mProj.perspective( 45, double(w)/double(h), 0.1, 100.0 ); else
    mProj.identity();
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
