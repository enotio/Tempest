#ifndef VERTEXBUFFERHOLDER_H
#define VERTEXBUFFERHOLDER_H

#include <Tempest/AbstractHolder>
//#include <Tempest/VertexBuffer>

namespace Tempest{

class VertexBufferBase;
class Device;

template< class Vertex >
class VertexBuffer;

class VertexBufferHolder : public AbstractHolder
                                    < Tempest::VertexBufferBase,
                                      AbstractAPI::VertexBuffer > {
  public:
    typedef AbstractHolder< Tempest::VertexBufferBase,
                            AbstractAPI::VertexBuffer  > BaseType;
    VertexBufferHolder( Device &d );
    ~VertexBufferHolder();

    template< class Vertex >
    VertexBuffer<Vertex> load( const Vertex v[], int count,
                               AbstractAPI::BufferFlag flg = AbstractAPI::BF_NoFlags ){
      VertexBuffer<Vertex> obj( *this, count );

      if( count > 0 ){
        createObject( obj.data.value(), (const char*)v,
                      count, sizeof(Vertex), flg );
        } else
        obj.data.value() = 0;

      return obj;
      }

    template< class Vertex >
    VertexBuffer<Vertex> load( const std::vector<Vertex>& v,
                               AbstractAPI::BufferFlag flg = AbstractAPI::BF_NoFlags ){
      return this->load( v.data(), int(v.size()), flg );
      }

  protected:
    typedef AbstractAPI::VertexBuffer DescriptorType;

    virtual void createObject( AbstractAPI::VertexBuffer*& t,
                               const char *src,
                               int size, int vsize,
                               AbstractAPI::BufferFlag flg = AbstractAPI::BF_NoFlags );
    virtual void deleteObject( AbstractAPI::VertexBuffer* t );

    virtual AbstractAPI::VertexBuffer* allocBuffer( size_t size, size_t vsize,
                                                    const void *src,
                                                    AbstractAPI::BufferFlag flg = AbstractAPI::BF_NoFlags );
    virtual AbstractAPI::VertexBuffer* allocBuffer( size_t size, size_t vsize,
                                                    const void *src,
                                                    AbstractAPI::BufferUsage u,
                                                    AbstractAPI::BufferFlag flg = AbstractAPI::BF_NoFlags);


    virtual void  reset( AbstractAPI::VertexBuffer* t );
    virtual AbstractAPI::VertexBuffer*
                  restore( AbstractAPI::VertexBuffer* t );
    virtual AbstractAPI::VertexBuffer*
                  copy( AbstractAPI::VertexBuffer* t );

    char* lockBuffer  (AbstractAPI::VertexBuffer* t, int b, int sz );
    char* bufferData  (AbstractAPI::VertexBuffer* t);
    void  unlockBuffer(AbstractAPI::VertexBuffer* t );
    void  updateBuffer(AbstractAPI::VertexBuffer* t, const void* data, int b, int sz);

  private:
    VertexBufferHolder( const VertexBufferHolder &h ) = delete;
    VertexBufferHolder& operator = ( const VertexBufferHolder& ) = delete;

    template< class I, class Vertex >
    void get( const VertexBuffer<Vertex> & vbo,
              I begin, I end,
              int b ){
      Vertex *v = (Vertex*)lockBuffer( vbo.data.const_value(), b,
                                       sizeof(Vertex)*(end-begin) );

      while( begin!=end ){
        *begin = *v;
        ++v;
        ++begin;
        }

      unlockNWBuffer( vbo.data.const_value() );
      }

    template< class Vertex >
    const Vertex* bufferData( const VertexBuffer<Vertex> & vbo ){
      const Vertex *v = (const Vertex*)bufferData( vbo.data.const_value() );
      return v;
      }

    struct Data;
    Data  *data;
    void  unlockNWBuffer(AbstractAPI::VertexBuffer* t );

  friend class VertexBufferBase;

  template< class V >
  friend class VertexBuffer;
  };

}

#endif // VERTEXBUFFERHOLDER_H
