#ifndef MEMPOOL_H
#define MEMPOOL_H

#include <cstring>
#include <memory>
#include <vector>

#include <Tempest/Assert>
#include "../core/wrappers/atomic.h"

namespace Tempest {

template< class T >
class MemPool {
  public:
    MemPool(){
      aviable.reserve(32);
      data.reserve(32);
      }

    ~MemPool(){
      for( size_t i=0; i<data.size(); ++i )
        delete data[i];
      }

    template< class ... Args >
    T* alloc( Args... a ){
      Detail::Guard guard(spin);
      (void)guard;

      if( aviable.size() ){
        Block *b = aviable.back();
        T* tmp = b->alloc(a...);
        if( b->freedCount==0 )
          aviable.pop_back();
        return tmp;
        }

      data.push_back( new Block() );
      aviable.push_back( data.back() );
      return data.back()->alloc(a...);
      }

    void free( T* t ){
      Detail::Guard guard(spin);
      (void)guard;

      for( size_t i=data.size(); i>0; ){
        --i;
        if( data[i]->free(t) ){
          if( aviable.size()>1 && data[i]->freedCount==256 ){
            for( size_t r=0; r<aviable.size(); ++r )
              if( aviable[r]==data[i] ){
                aviable[r] = aviable.back();
                aviable.pop_back();

                delete data[i];
                for( size_t q=i; q+1<data.size(); ++q )
                  data[q] = data[q+1];

                data.pop_back();
                return;
                }
            }

          if( data[i]->freedCount==1 )
            aviable.push_back( data[i] );
          return;
          }
        }

      T_WARNING_X(0, "bad free");
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
    std::vector< Block* > data, aviable;
    Detail::Spin spin;
    };

}

#endif // MEMPOOL_H
