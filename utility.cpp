#include "utility.h"

#include <algorithm>

using namespace Tempest;

Point Rect::pos() const{
  return Point(x,y);
  }

Size Rect::size() const {
  return Size(w,h);
  }

Rect Rect::intersected(const Rect &r) const {
  Rect re;
  re.x = std::max( x, r.x );
  re.y = std::max( y, r.y );

  re.w = std::min( x+w, r.x+r.w ) - re.x;
  re.h = std::min( y+h, r.y+r.h ) - re.y;

  re.w = std::max(0, re.w);
  re.h = std::max(0, re.h);

  return re;
  }

bool Rect::contains(const Point &p) const {
  return contains(p.x, p.y);
  }

bool Rect::contains(int px, int py) const {
  return ( x<px && px<x+w ) && ( y<py && py<y+h );
  }

bool Rect::contains(const Point &p, bool border) const {
  return contains(p.x, p.y, border);
  }

bool Rect::contains(int px, int py, bool border) const {
  if( border )
    return ( x<=px && px<=x+w ) && ( y<=py && py<=y+h ); else
    return contains(x,y);
  }

bool Rect::isEmpty() const {
  return w<=0 || h<=0;
  }

bool Rect::operator ==(const Rect &other) const {
  return x==other.x && y==other.y && w==other.w && h==other.h;
  }

bool Rect::operator !=(const Rect &other) const {
  return !(*this==other);
  }

Point Size::toPoint() const {
  return Point(w,h);
  }

bool Size::operator ==(const Size &other) const {
  return w==other.w && h==other.h;
  }

bool Size::operator !=(const Size &other) const {
  return w!=other.w || h!=other.h;
  }

Point &Point::operator -=( const Point &p ) {
  x -= p.x;
  y -= p.y;

  return *this;
  }

Point &Point::operator +=(const Point &p) {
  x += p.x;
  y += p.y;

  return *this;
  }

Point Point::operator +(const Point &p) const {
  return Point( p.x+x, p.y+y );
  }

Point Point::operator -(const Point &p) const {
  return Point( x-p.x, y-p.y );
  }

Point Point::operator -() const {
  return Point( -x, -y );
  }

bool Point::operator ==(const Point &other) const {
  return x==other.x && y==other.y;
  }

bool Point::operator !=(const Point &other) const {
  return x!=other.x || y!=other.y;
  }
