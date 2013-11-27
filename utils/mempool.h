#ifndef MEMPOOL_H
#define MEMPOOL_H

#include <cstring>
#include <memory>
#include <vector>

#include "../core/wrappers/atomic.h"

namespace Tempest {

template< class T >
class MemPool {
  public:
    MemPool(){

      }

    ~MemPool(){
      for( size_t i=0; i<data.size(); ++i )
        delete data[i];
      }

    template< class ... Args >
    T* alloc( Args... a ){
      Detail::Guard guard(spin);
      (void)guard;

      for( size_t i=data.size(); i>0; ){
        --i;
        if( data[i]->freedCount>0 ){
          Block *b = data[i];
          T* tmp = b->alloc(a...);
          for( size_t r=i; r+1<data.size(); ++r )
            data[r] = data[r+1];

          data.back() = b;
          return tmp;
          }
        }

      data.push_back( new Block() );
      return data.back()->alloc(a...);
      }

    void free( T* t ){
      Detail::Guard guard(spin);
      (void)guard;

      for( size_t i=data.size(); i>0; ){
        --i;
        if( data[i]->free(t) ){
          return;
          }
        }
      }

  private:
    struct TX{
      char d[ sizeof(T) ];

      template< class ... Args >
      void construct( Args... arg ){
        new (d) T(arg...);
        }

      void destruct(){
        T* t = (T*)d;
        t->~T();
        }
      };

    struct Block{
      TX data[256];
      unsigned char freed[256];
      int freedCount;

      Block(){
        freedCount = 256;
        for( int i=0; i<freedCount; ++i )
          freed[i] = 255 - i;
        }

      template< class ... Args >
      T* alloc( Args... a ){
        --freedCount;
        size_t pos = freed[ freedCount ];
        data[pos].construct( a... );

        return (T*)(&data[pos]);
        }

      bool free( T* t ){
        if( (T*)data > t )
          return false;

        size_t pos = t - ((T*)data);
        if( pos < 256 ){
          freed[ freedCount ] = pos;
          ++freedCount;

          data[pos].destruct();
          return true;
          }

        return false;
        }
      };
    std::vector< Block* > data;
    Detail::Spin spin;
    };

}

#endif // MEMPOOL_H
