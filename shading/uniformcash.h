#ifndef UNIFORMCASH_H
#define UNIFORMCASH_H

#include <vector>

#include <Tempest/AbstractAPI>
#include <Tempest/Matrix4x4>

namespace Tempest{
namespace Detail{

template< class Layout >
class UniformCash {
  public:
    UniformCash(){
      texture.reserve(32);
      vec1.reserve(32);
      vec2.reserve(32);
      vec3.reserve(32);
      vec4.reserve(32);
      matrix.reserve(32);
      }

    template< int s >
    struct Vec{
      double data[s];

      bool operator == ( const Vec<s> & v ) const {
        for( int i=0; i<s; ++i )
          if( v.data[i]!=data[i])
            return false;
        return true;
        }
      };

    template< class T >
    struct Node{
      T data;
      Layout layout;
      };

    std::vector< Node<float  > >   vec1;
    std::vector< Node<Vec<2> > >   vec2;
    std::vector< Node<Vec<3> > >   vec3;
    std::vector< Node<Vec<4> > >   vec4;
    std::vector< Node<size_t> >    texture;
    std::vector< Node<Matrix4x4> > matrix;

    void reset(){
      texture.clear();
      vec1.clear();
      vec2.clear();
      vec3.clear();
      vec4.clear();
      matrix.clear();
      }

    template< class T >
    bool fetchTmp( std::vector< Node<T> > & v,
                   const Layout & layout, const T& data ){
      for( size_t i=0; i<v.size(); ++i ){
        if( v[i].layout == layout ){
          if( v[i].data == data ){
            return true;
            } else {
            v[i].data = data;
            return false;
            }
          }
        }

      Node<T> n;
      n.layout = layout;
      n.data   = data;

      v.push_back(n);
      return false;
      }

    bool fetch( const Layout & layout, size_t data ){
      return fetchTmp( texture, layout, data );
      }

    bool fetch( const Layout & layout, const Matrix4x4& data ){
      return fetchTmp( matrix, layout, data );
      }

    bool fetch( const Layout & layout, const float data[], int l ){
      if( l==4 ){
        Vec<4> v;
        std::copy( data, data+4, v.data );
        return fetchTmp( vec4, layout, v );
        }

      if( l==2 ){
        Vec<2> v;
        std::copy( data, data+2, v.data );
        //return 0;
        return fetchTmp( vec2, layout, v );
        }

      if( l==3 ){
        Vec<3> v;
        std::copy( data, data+3, v.data );
        return fetchTmp( vec3, layout, v );
        }

      if( l==1 ){
        return fetchTmp( vec1, layout, *data );
        }

      return false;
      }

  };

}
}

#endif // UNIFORMCASH_H
