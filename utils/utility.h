#ifndef UTILITY_H
#define UTILITY_H

namespace Tempest{

  struct Point{
    Point():x(0), y(0) {}
    explicit Point( int ix, int iy=0 ):x(ix), y(iy) {}
    int x, y;

    Point& operator -= ( const Point & p );
    Point& operator += ( const Point & p );

    Point operator + ( const Point & p ) const;
    Point operator - ( const Point & p ) const;

    Point operator * ( double f ) const;
    Point operator * ( float f ) const;
    Point operator * ( int   f ) const;

    Point& operator *= ( double f );
    Point& operator *= ( float  f );
    Point& operator *= ( int    f );

    Point operator / ( double f ) const;
    Point operator / ( float f ) const;
    Point operator / ( int   f ) const;

    Point& operator /= ( double f );
    Point& operator /= ( float  f );
    Point& operator /= ( int    f );

    Point operator - () const;

    double manhattanLength() const;

    bool operator ==( const Point & other ) const;
    bool operator !=( const Point & other ) const;
    };

  struct Size{
    Size( int is = 0 ):w(is), h(is) {}
    Size( int ix, int iy ):w(ix), h(iy) {}
    int w, h;

    Point toPoint() const;
    bool  isEmpty() const;

    bool operator ==( const Size & other ) const;
    bool operator !=( const Size & other ) const;
    };


  struct Rect{
    Rect():x(0), y(0), w(0), h(0) {}
    Rect( int ix,
          int iy,
          int iw,
          int ih )
      :x(ix), y(iy), w(iw), h(ih) {}
    int x,y,w,h;

    Point pos() const;
    Size  size() const;

    Rect intersected( const Rect& ) const;

    bool contains( const Point & p ) const;
    bool contains( int x, int y ) const;

    bool contains( const Point & p, bool border ) const;
    bool contains( int x, int y, bool border ) const;

    bool isEmpty() const;

    bool operator ==( const Rect & other ) const;
    bool operator !=( const Rect & other ) const;
    };

  }

#endif // UTILITY_H
