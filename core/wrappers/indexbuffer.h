#ifndef INDEXBUFFER_H
#define INDEXBUFFER_H

#include <Tempest/Device>
#include <Tempest/CopyWhenNeedPtr>
#include <Tempest/AbstractHolder>

namespace Tempest{

class IndexBufferBase{
  public:
    virtual ~IndexBufferBase(){}
  };

template< class Vertex >
class IndexBuffer : public IndexBufferBase {
  public:
    typedef AbstractHolder< Tempest::IndexBufferBase,
                            AbstractAPI::IndexBuffer > Holder;

    IndexBuffer(): m_size(0),
                    data( Holder::ImplManip(0) ){}

    IndexBuffer( Holder &h, int s ): m_size(s),
              data( h.makeManip() ){
      }

    virtual ~IndexBuffer(){
      }

    int size() const {
      return m_size;
      }
  private:
    int      m_size;

    typedef Detail::Ptr<AbstractAPI::IndexBuffer*, Holder::ImplManip> Ptr;
    Ptr data;

  friend class Device;
  friend class IndexBufferHolder;
  };

}

#endif // INDEXBUFFER_H
