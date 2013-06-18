#ifndef OBJECTMATRIX_H
#define OBJECTMATRIX_H

namespace Tempest{

class Matrix4x4{
  public:
    Matrix4x4();
    Matrix4x4( const Matrix4x4& other );
    Matrix4x4( const float data[/*16*/] );
    Matrix4x4( float a11, float a12, float a13, float a14,
               float a21, float a22, float a23, float a24,
               float a31, float a32, float a33, float a34,
               float a41, float a42, float a43, float a44 );
    ~Matrix4x4();

    void identity();

    void translate( const float vec[/*3*/]);

    void translate(float x, float y, float z);


    void scale( float x );
    void scale(float x, float y, float z);

    void rotate(float angle, float x, float y, float z);
    void rotateOZ( float angle );

    const float *data() const;
    float at( int x, int y ) const;
    void  set( int x, int y, float v );

    void setData( const float data[/*16*/]);
    void setData( float a11, float a12, float a13, float a14,
                  float a21, float a22, float a23, float a24,
                  float a31, float a32, float a33, float a34,
                  float a41, float a42, float a43, float a44 );

    void transpose();
    void inverse();
    void mul( const Matrix4x4& other );

    void project(float   x, float   y, float   z, float   w,
                 float &ox, float &oy, float &oz, float &ow ) const;

    void perspective( float angle, float aspect, float zNear, float zFar);

    Matrix4x4& operator = ( const Matrix4x4& other );
    bool operator == ( const Matrix4x4& other ) const;
    bool operator != ( const Matrix4x4& other ) const{
      return !( *this==other );
      }
  private:
    float m[4][4];

    void check() const;
  };

}

#endif // OBJECTMATRIX_H
