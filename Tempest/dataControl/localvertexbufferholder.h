#ifndef LOCALVERTEXBUFFERHOLDER_H
#define LOCALVERTEXBUFFERHOLDER_H

#include <Tempest/VertexBufferHolder>
#include <Tempest/IndexBufferHolder>
#include <Tempest/LocalObjectPool>

#include <vector>
#include <cstring>

namespace Tempest {

template< class Holder >
class LocalBufferHolder : public Holder {
  public:
    LocalBufferHolder( Tempest::Device &d,
                       AbstractAPI::BufferUsage bu = AbstractAPI::BU_Dynamic )
      :Holder(d), bufferUsage(bu) {
      dynVBOs .reserve(2048);

      setReserveSize( 0xFFFF );
      maxReserved = -1;
      minVboSize  = 4*1024;

      needToRestore = false;
      pcollect      = false;
      }

    ~LocalBufferHolder(){
      reset();
      }

    void setMaxCollectIterations(int c){
      nonFreed.maxColletIterations = c;
      }

    int  maxCollectIterations() const {
      return nonFreed.maxColletIterations;
      }

    void setReserveSize( int sz ){
      reserveSize = sz;
      }

    void setMaxReservedCount( int s ) {
      maxReserved = s;
      }

    int maxReservedCount() const {
      return maxReserved;
      }

    void pauseCollect( bool p ){
      pcollect = p;
      }

    bool isCollectPaused() const{
      return pcollect;
      }

  protected:
    int reserveSize, maxReserved;
    int minVboSize;

    struct NonFreedData {
      typename Holder::DescriptorType* handle;
      int  memSize, elSize;
      bool restoreIntent;
      };

    struct NonFreed{
      NonFreedData data;
      int collectIteration;
      void * userPtr;
      };

    virtual void reset(){
      needToRestore = true;
      nonFreed.reset( *this, &LocalBufferHolder::deleteObject );
      Holder::BaseType::reset();
      }

    virtual bool restore() {
      for( size_t i=0; i<dynVBOs.size(); ++i )
        dynVBOs[i].data.restoreIntent = true;

      bool ok = Holder::BaseType::restore();
      needToRestore = false;
      return ok;
      }

    using Holder::reset;
    using Holder::restore;

    virtual void presentEvent() {
      if( pcollect )
        return;

      nonFreed.collect( *this,
                        &LocalBufferHolder::deleteObject,
                        &LocalBufferHolder::deleteCond );
      }

    void deleteObject( NonFreed& obj ) {
      Holder::deleteObject( obj.data.handle );
      }

    bool deleteCond( NonFreed& obj ){
      return obj.data.memSize > minVboSize;
      }

  private:
    bool validAs( const NonFreedData& x, const NonFreedData &d ){
      return d.memSize  <= x.memSize &&
             (d.memSize <= minVboSize || d.memSize*4 >= x.memSize) &&
             d.elSize    == x.elSize;
      }

    void createObject( typename Holder::DescriptorType*& t,
                       const char *src,
                       int size, int vsize,
                       AbstractAPI::BufferFlag flg = AbstractAPI::BF_NoFlags ) {
      if( flg & AbstractAPI::BF_NoReadback )
        flg = AbstractAPI::BufferFlag(flg ^ AbstractAPI::BF_NoReadback);

      if( needToRestore ){
        typename Holder::DescriptorType* old = t;
        Holder::createObject(t, src, size, vsize, flg );

        for( size_t i=0; i<dynVBOs.size(); ++i )
          if( dynVBOs[i].data.handle==old &&
              dynVBOs[i].data.restoreIntent ){
            dynVBOs[i].data.handle = t;
            dynVBOs[i].data.restoreIntent = false;
            return;
            }
        return;
        }

      NonFreedData d;
      d.memSize       = std::max( minVboSize,
                                  vsize*(nearPOT( size*vsize )/vsize) );
      d.restoreIntent = false;
      d.elSize        = vsize;

      NonFreed x = nonFreed.find(d, *this, &LocalBufferHolder::validAs );
      if( x.data.handle ){
        dynVBOs.push_back( x );
        t = dynVBOs.back().data.handle;
        {
          int   cpySz = size*vsize;
          char *pVertices = this->lockBuffer( t, 0, cpySz);

          std::copy( src, src+cpySz, pVertices );
          this->unlockBuffer( t );
          }

        return;
        }

      std::vector<char> tmp( d.memSize );
      std::copy( src, src+size*vsize, tmp.begin() );
      std::fill( tmp.begin()+size*vsize, tmp.end(), 0 );

      Holder::createObject( t, &tmp[0], d.memSize/d.elSize, d.elSize, flg );

      if( !t )
        return;

      d.handle = t;

      NonFreed nf;
      nf.data = d;
      nf.collectIteration = 0;
      nf.userPtr = 0;

      dynVBOs.push_back( nf );
      }

    void deleteObject( typename Holder::DescriptorType* t ) {
      for( size_t i=0; i<dynVBOs.size(); ++i )
        if( dynVBOs[i].data.handle==t ){
          if( maxReserved>=0 && int(nonFreed.size())>=maxReserved ){
            Holder::deleteObject(t);
            } else {
            nonFreed.push( dynVBOs[i] );
            }

          dynVBOs[i] = dynVBOs.back();
          dynVBOs.pop_back();

          return;
          }

      Holder::deleteObject(t);
      }

    typename Holder::DescriptorType* allocBuffer( size_t size,
                                                  size_t vsize,
                                                  const void *src,
                                                  AbstractAPI::BufferFlag flg = AbstractAPI::BF_NoFlags ){
      return Holder::allocBuffer(size, vsize, src, bufferUsage, flg);
      }

    std::vector< NonFreed >    dynVBOs;
    LocalObjectPool<NonFreed>  nonFreed;

    int nearPOT( int x ) {
      int r = 1;
      for( int i=0; r<=reserveSize; ++i ){
        if( r>=x )
          return r;
        r *= 2;
        }

      return x;
      }

    bool needToRestore;
    bool pcollect;
    AbstractAPI::BufferUsage bufferUsage;
  };

struct LocalVertexBufferHolder: LocalBufferHolder< Tempest::VertexBufferHolder > {
  LocalVertexBufferHolder( Tempest::Device &d )
    :LocalBufferHolder< Tempest::VertexBufferHolder >(d){}
  LocalVertexBufferHolder( Tempest::Device &d,
                           AbstractAPI::BufferUsage bu )
    :LocalBufferHolder< Tempest::VertexBufferHolder >(d, bu){}
  };

struct LocalIndexBufferHolder: LocalBufferHolder< Tempest::IndexBufferHolder  > {
  LocalIndexBufferHolder( Tempest::Device &d )
    :LocalBufferHolder< Tempest::IndexBufferHolder >(d){}
  LocalIndexBufferHolder( Tempest::Device &d,
                           AbstractAPI::BufferUsage bu )
    :LocalBufferHolder< Tempest::IndexBufferHolder >(d, bu){}
  };
}

#endif // LOCALVERTEXBUFFERHOLDER_H
