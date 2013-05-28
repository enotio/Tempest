#ifndef INDEXBUFFER_H
#define INDEXBUFFER_H

#include <Tempest/Device>
#include <Tempest/CopyWhenNeedPtr>
#include <Tempest/AbstractHolder>
#include <Tempest/IndexBufferHolder>

#include <cassert>

namespace Tempest{

class IndexBufferBase{
  public:
    virtual ~IndexBufferBase(){}
  };

template< class Index >
class IndexBuffer : public IndexBufferBase {
  public:
    typedef AbstractHolder< Tempest::IndexBufferBase,
                            AbstractAPI::IndexBuffer > Holder;

    IndexBuffer(): m_size(0), m_first(0),
                   data( Holder::ImplManip(0) ){}

    IndexBuffer( Holder &h, int s ): m_size(s), m_first(0),
                                     data( h.makeManip() ){
      }

    virtual ~IndexBuffer(){
      }

    int size() const {
      return m_size;
      }

    IndexBuffer<Index> slice( int first, int size ) const {
      assert( first+size <= m_size );

      IndexBuffer<Index> tmp = *this;
      tmp.m_first += first;
      tmp.m_size   = size;

      return tmp;
      }

    IndexBuffer<Index> slice( int first ) const {
      return slice( first, size()-first );
      }

    template< class I >
    void get( I begin, I end, int b ) const {
      if( end-begin==0 )
        return;

      ((IndexBufferHolder&)data.manip.holder()).get( *this, begin, end, b );
      }

    size_t handle() const {
      return reinterpret_cast<size_t>( data.const_value() );
      }
  private:
    int      m_size, m_first;

    typedef Detail::Ptr<AbstractAPI::IndexBuffer*, Holder::ImplManip> Ptr;
    Ptr data;

  friend class Device;
  friend class IndexBufferHolder;
  };

}

#endif // INDEXBUFFER_H
