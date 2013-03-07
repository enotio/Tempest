#ifndef OBJECTMATRIX_H
#define OBJECTMATRIX_H

namespace Tempest{

class Matrix4x4{
  public:
    Matrix4x4();
    Matrix4x4( const Matrix4x4& other );
    Matrix4x4( const double data[/*16*/] );
    Matrix4x4( double a11, double a12, double a13, double a14,
               double a21, double a22, double a23, double a24,
               double a31, double a32, double a33, double a34,
               double a41, double a42, double a43, double a44 );
    ~Matrix4x4();

    void identity();

    void translate( const double vec[/*3*/]);

    void translate(double x, double y, double z);


    void scale( double x );
    void scale(double x, double y, double z);

    void rotate(double angle, double x, double y, double z);
    void rotateOZ( double angle );

    const double *data() const;
    double at( int x, int y ) const;
    void  set( int x, int y, double v ) const;

    void setData( const double data[/*16*/]);
    void setData( double a11, double a12, double a13, double a14,
                  double a21, double a22, double a23, double a24,
                  double a31, double a32, double a33, double a34,
                  double a41, double a42, double a43, double a44 );

    void transpose();
    void inverse();
    void mul( const Matrix4x4& other );

    void project(double   x, double   y, double   z, double   w,
                 double &ox, double &oy, double &oz, double &ow ) const;

    void perspective( double angle, double aspect, double zNear, double zFar);

    Matrix4x4& operator = ( const Matrix4x4& other );
    bool operator == ( const Matrix4x4& other ) const;
  private:
    class pimpl;
    Matrix4x4::pimpl * matrix;
    double rimpl[ 16 ];
  };

}

#endif // OBJECTMATRIX_H
