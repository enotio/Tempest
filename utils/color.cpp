#include "color.h"

using namespace Tempest;

Color::Color(){
  set( 1.0 );
  }

Color::Color( float c ){
  set( c );
  }

Color::Color(	float r, float g,
              float b, float a){
  set(r, g, b, a);
  }

void Color::set( float r,
                 float g,
                 float b,
                 float a){
  cdata[0] = r;
  cdata[1] = g;
  cdata[2] = b;
  cdata[3] = a;
  }

void Color::set( float rgba ){
  for(int i=0; i<4; ++i)
    cdata[i] = rgba;
  }

const float * Color::data() const{
  return cdata;
  }

Color& Color::operator = ( const Color & other){
  for(int i=0; i<4; ++i)
    cdata[i] = other.cdata[i];

  return *this;
  }

Color Color::operator + ( const Color & other){
  Color c;
  for(int i=0; i<4; ++i)
    c.cdata[i] = cdata[i]+other.cdata[i];

  return c;
  }

Color Color::operator - ( const Color & other){
  Color c;
  for(int i=0; i<4; ++i)
    c.cdata[i] = cdata[i]-other.cdata[i];

  return c;
  }

void Color::operator += ( const Color & other){
  for(int i=0; i<4; ++i)
    cdata[i] += other.cdata[i];

  }

void Color::operator -= ( const Color & other){
  for(int i=0; i<4; ++i)
    cdata[i] -= other.cdata[i];

  }

float Color::r() const{
  return data()[0];
  }

float Color::g() const{
  return data()[1];
  }

float Color::b() const{
  return data()[2];
  }

float Color::a() const{
  return data()[3];
  }
