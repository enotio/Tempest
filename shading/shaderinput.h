#ifndef SHADERINPUT_H
#define SHADERINPUT_H

#include <vector>
#include <string>

#include <Tempest/Texture2d>
#include <Tempest/Matrix4x4>

#include <Tempest/Uniform>
#include <Tempest/VertexDeclaration>

namespace Tempest{

class ShaderInput {
  public:
    void set( const char* name, const Texture2d & t );
    void set( const char* name, const Matrix4x4 & t );

    void set( const char* name, float x );
    void set( const char* name, float x, float y );
    void set( const char* name, float x, float y, float z );
    void set( const char* name, float x, float y, float z, float w );

    void set( const char* name, const float *xyzw, int l );

    void set( const Uniform<float[2]> & u );
    void set( const Uniform<float[3]> & u );
    void set( const Uniform<Texture2d> & u );

  //private:
    template< int s >
    struct Vec{
      float v[s];

      bool operator == ( const Vec<s>& t ) const {
        for( int i=0; i<s; ++i )
          if( v[i]!=t.v[i] )
            return 0;

        return 1;
        }

      bool operator != ( const Vec<s>& t ) const {
        for( int i=0; i<s; ++i )
          if( v[i]!=t.v[i] )
            return 1;

        return 0;
        }
      };

    template< class T >
    struct Chunk{
      Chunk(){
        names. reserve(32);
        values.reserve(32);
        id.    reserve(32);
        }

      std::vector<std::string>   names;
      std::vector<T>             values;
      mutable std::vector<void*> id;

      void set( const char* name, const T & t ){
        for( size_t i=0; i<names.size(); ++i )
          if( name==names[i] ){
            if( values[i] != t ){
              values[i] =  t;
              id[i]     = (void*)size_t(-1);
              }
            return;
            }

        names.push_back( name );
        values.push_back(t);
        id.push_back(0);
        }

      void resetID() const {
        for( size_t i=0; i<id.size(); ++i )
          id[i] = (void*)size_t(-1);
        }
      };

    Chunk< Vec<1> >    v1;
    Chunk< Vec<2> >    v2;
    Chunk< Vec<3> >    v3;
    Chunk< Vec<4> >    v4;
    Chunk< const Texture2d* > tex;
    Chunk< Matrix4x4 > mat;

    void resetID() const{
      v1.resetID();
      v2.resetID();
      v3.resetID();
      v4.resetID();

      tex.resetID();
      mat.resetID();
      }
  };

}

#endif // SHADERINPUT_H
