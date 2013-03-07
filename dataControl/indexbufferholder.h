#ifndef INDEXBUFFERHOLDER_H
#define INDEXBUFFERHOLDER_H

#include <Tempest/AbstractHolder>

namespace Tempest{

class IndexBufferBase;
class Device;

template< class Index >
class IndexBuffer;

class IndexBufferHolder : public AbstractHolder
                                    < Tempest::IndexBufferBase,
                                      AbstractAPI::IndexBuffer > {
  public:
    typedef AbstractHolder< Tempest::IndexBufferBase,
                            AbstractAPI::IndexBuffer  > BaseType;
    IndexBufferHolder( Device &d );
    ~IndexBufferHolder();

    template< class Index >
    IndexBuffer<Index> load( const Index v[], int count ){
      IndexBuffer<Index> obj( *this, count );

      createObject( obj.data.value(), (const char*)v,
                    count, sizeof(Index) );
      return obj;
      }

  protected:
    virtual void createObject( AbstractAPI::IndexBuffer*& t,
                               const char *src,
                               int size, int vsize );
    virtual void deleteObject( AbstractAPI::IndexBuffer* t );


    virtual void  reset( AbstractAPI::IndexBuffer* t );
    virtual AbstractAPI::IndexBuffer*
                  restore( AbstractAPI::IndexBuffer* t );
    virtual AbstractAPI::IndexBuffer*
                  copy( AbstractAPI::IndexBuffer* t );
  private:
    IndexBufferHolder( const IndexBufferHolder &h );
    void operator = ( const IndexBufferHolder& ){}

    struct Data;
    Data  *data;

  friend class IndexBufferBase;
  };

}

#endif // INDEXBUFFERHOLDER_H
