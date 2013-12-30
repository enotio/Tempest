#ifndef VERTEXBUFFER_H
#define VERTEXBUFFER_H

#include <Tempest/Device>
#include <Tempest/CopyWhenNeedPtr>
#include <Tempest/AbstractHolder>
#include <Tempest/Assert>
#include <Tempest/VertexBufferHolder>

namespace Tempest{

class VertexBufferHolder;

class VertexBufferBase{
  public:
    virtual ~VertexBufferBase(){}
  };

template< class Vertex >
class VertexBuffer : public VertexBufferBase {
  public:
    typedef AbstractHolder< Tempest::VertexBufferBase,
                            AbstractAPI::VertexBuffer > Holder;

    VertexBuffer(): m_size(0), m_first(0),
                    data( Holder::ImplManip(0) ){}

    VertexBuffer( Holder &h, int size ): m_size(size), m_first(0),
              data( h.makeManip() ){
      }

    virtual ~VertexBuffer(){
      }

    int size() const {
      return m_size;
      }

    VertexBuffer<Vertex> slice( int first, int size ) const {
      T_ASSERT( first+size <= m_size );

      VertexBuffer<Vertex> tmp = *this;
      tmp.m_first += first;
      tmp.m_size   = size;

      return tmp;
      }

    VertexBuffer<Vertex> slice( int first ) const {
      return slice( first, size()-first );
      }

    template< class I >
    void get( I begin, I end, int b ) const {
      if( end-begin==0 )
        return;

      ((VertexBufferHolder&)data.manip.holder()).get( *this, begin, end, b );
      }

    const Vertex* const_data() const {
      return ((VertexBufferHolder&)data.manip.holder()).bufferData( *this );
      }

    size_t handle() const {
      return reinterpret_cast<size_t>( data.const_value() );
      }
  private:
    int      m_size, m_first;

    typedef Detail::Ptr<AbstractAPI::VertexBuffer*, Holder::ImplManip> Ptr;
    Ptr data;

  friend class Device;
  friend class VertexBufferHolder;
  };

}

#endif // VERTEXBUFFER_H
