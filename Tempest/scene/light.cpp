#include "light.h"

#include <cmath>

using namespace Tempest;

DirectionLight::DirectionLight(){
  setDirection(0,0,-1);
  setColor    ( Color(0.7f) );
  setAblimient( Color(0.3f) );
  }

void DirectionLight::setDirection( double x, double y, double z ){
  double l = sqrt( x*x + y*y + z*z );
  direction[0] = x/l;
  direction[1] = y/l;
  direction[2] = z/l;
  }

double DirectionLight::xDirection() const{
  return direction[0];
  }

double DirectionLight::yDirection() const{
  return direction[1];
  }

double DirectionLight::zDirection() const{
  return direction[2];
  }

Color DirectionLight::color() const{
  return cl;
  }

void DirectionLight::setColor( const Color & c ){
  cl = c;
  }


Color DirectionLight::ablimient() const{
  return ao;
  }

void DirectionLight::setAblimient( const Color & c ){
  ao = c;
  }
