#ifndef LOCALOBJECTPOOL_H
#define LOCALOBJECTPOOL_H

#include <vector>

namespace Tempest{

template< class T >
struct LocalObjectPool{
  LocalObjectPool(){
    data.reserve(256);
    maxColletIterations = 3;
    }

  void push( const T& t ){
    data.push_back(t);
    data.back().collectIteration = 0;
    }

  template< class D, class H >
  T find( const D& x, H& h, bool (H::*f)(const D&, const D&) ){
    for( size_t i=data.size(); i>0;  ){
      --i;
      if( (h.*f)(data[i].data, x) ) {
        T r = data[i];
        data[i] = data.back();
        data.pop_back();
        return r;
        }
      }

    T t;
    t.data.handle = 0;
    return t;
    }

  template< class H >
  void collect( H& h, void(H::*f)(T&), bool(H::*deleteCond)(T&) ){
    for( size_t i=0; i<data.size(); ){
      ++data[i].collectIteration;

      if( data[i].collectIteration > maxColletIterations &&
          (h.*deleteCond)(data[i])
          /*&& data[i].data.memSize > h.minVboSize*/ ){
        deleteObject( data[i], h, f );
        data[i] = data.back();
        data.pop_back();
        } else {
        ++i;
        }
      }
    }

  template< class H >
  void reset( H& h, void(H::*f)(T&) ){
    for( size_t i=0; i<data.size(); ++i )
      deleteObject( data[i], h, f );

    data.clear();
    }

  template< class H >
  void deleteObject( T& t, H& h, void(H::*f)(T&) ){
    (h.*f)(t);
    }

  size_t size() const{
    return data.size();
    }

  std::vector< T > data;
  unsigned maxColletIterations;
  };

}

#endif // LOCALOBJECTPOOL_H
