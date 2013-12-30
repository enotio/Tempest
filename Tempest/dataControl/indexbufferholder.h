#ifndef INDEXBUFFERHOLDER_H
#define INDEXBUFFERHOLDER_H

#include <Tempest/AbstractHolder>
#include <algorithm>

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
    IndexBuffer<Index> load( const Index v[], int count,
                             AbstractAPI::BufferFlag flg = AbstractAPI::BF_NoFlags ){
      IndexBuffer<Index> obj( *this, count );

      if( count )
        createObject( obj.data.value(), (const char*)v,
                      count, sizeof(Index), flg ); else
        obj.data.value() = 0;

      return obj;
      }

    template< class Index >
    IndexBuffer<Index> load( const std::vector<Index>& ibo,
                             AbstractAPI::BufferFlag flg = AbstractAPI::BF_NoFlags ){
      return this->load( &ibo[0], ibo.size(), flg );
      }

  protected:
    typedef AbstractAPI::IndexBuffer DescriptorType;

    virtual void createObject(AbstractAPI::IndexBuffer*& t,
                               const char *src,
                               int size, int vsize , AbstractAPI::BufferFlag flg);
    virtual void deleteObject( AbstractAPI::IndexBuffer* t );


    virtual AbstractAPI::IndexBuffer* allocBuffer( size_t size, size_t vsize,
                                                   const void *src );
    virtual AbstractAPI::IndexBuffer* allocBuffer( size_t size, size_t vsize,
                                                   const void *src,
                                                   AbstractAPI::BufferUsage u,
                                                   AbstractAPI::BufferFlag flg = AbstractAPI::BF_NoFlags );



    virtual void  reset( AbstractAPI::IndexBuffer* t );
    virtual AbstractAPI::IndexBuffer*
                  restore( AbstractAPI::IndexBuffer* t );
    virtual AbstractAPI::IndexBuffer*
                  copy( AbstractAPI::IndexBuffer* t );

    char* lockBuffer  (AbstractAPI::IndexBuffer* t, int b, int sz );
    void  unlockBuffer(AbstractAPI::IndexBuffer* t );

  private:
    IndexBufferHolder( const IndexBufferHolder &h );
    void operator = ( const IndexBufferHolder& ){}

    template< class I, class Index >
    void get( const IndexBuffer<Index> & vbo,
              I begin, I end,
              int b ){
      Index *v = (Index*)lockBuffer( vbo.data.const_value(), b,
                                     sizeof(Index)*(end-begin) );

      while( begin!=end ){
        *begin = *v;
        ++v;
        ++begin;
        }

      unlockNWBuffer( vbo.data.const_value() );
      }

    struct Data;
    Data  *data;
    void  unlockNWBuffer(AbstractAPI::IndexBuffer* t );

  friend class IndexBufferBase;

  template< class I >
  friend class IndexBuffer;
  };

}

#endif // INDEXBUFFERHOLDER_H
