#ifndef VERTEXBUFFERHOLDER_H
#define VERTEXBUFFERHOLDER_H

#include <Tempest/AbstractHolder>
//#include <MyGL/VertexBuffer>

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
    VertexBuffer<Vertex> load( const Vertex v[], int count ){
      VertexBuffer<Vertex> obj( *this, count );

      if( count > 0 ){
        createObject( obj.data.value(), (const char*)v,
                      count, sizeof(Vertex) );
        }

      return obj;
      }

  protected:
    virtual void createObject( AbstractAPI::VertexBuffer*& t,
                               const char *src,
                               int size, int vsize );
    virtual void deleteObject( AbstractAPI::VertexBuffer* t );


    virtual void  reset( AbstractAPI::VertexBuffer* t );
    virtual AbstractAPI::VertexBuffer*
                  restore( AbstractAPI::VertexBuffer* t );
    virtual AbstractAPI::VertexBuffer*
                  copy( AbstractAPI::VertexBuffer* t );

    char* lockBuffer  (AbstractAPI::VertexBuffer* t, int b, int sz );
    void  unlockBuffer(AbstractAPI::VertexBuffer* t );

  private:
    VertexBufferHolder( const VertexBufferHolder &h );
    void operator = ( const VertexBufferHolder& ){}

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

    struct Data;
    Data  *data;
    void  unlockNWBuffer(AbstractAPI::VertexBuffer* t );

  friend class VertexBufferBase;

  template< class V >
  friend class VertexBuffer;
  };

}

#endif // VERTEXBUFFERHOLDER_H
