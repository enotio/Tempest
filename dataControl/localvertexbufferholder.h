#ifndef LOCALVERTEXBUFFERHOLDER_H
#define LOCALVERTEXBUFFERHOLDER_H

#include <Tempest/VertexBufferHolder>
#include <Tempest/IndexBufferHolder>
#include <vector>
#include <cstring>

namespace Tempest {

template< class Holder >
class LocalBufferHolder : public Holder {
  public:
    LocalBufferHolder( Tempest::Device &d )
      :Holder(d){
      nonFreed.reserve(128);
      dynVBOs .reserve(128);

      setReserveSize( 0xFFFF );
      maxReserved = -1;

      needToRestore = false;
      }

    ~LocalBufferHolder(){
      reset();
      }

    void setReserveSize( int sz ){
      reserveSize = sz;
      }

    void setMaxReservedCount( int s ) {
      maxReserved = s;
      }

  protected:
    int reserveSize, maxReserved;

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

      for( size_t i=0; i<nonFreed.size(); ++i )
        deleteObject( nonFreed[i] );

      nonFreed.clear();
      //dynVBOs .clear();
      //for( size_t i=0; i<dynVBOs.size(); ++i )
        //reset( dynVBOs[i].data.handle );

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
      collect( nonFreed );
      }

    virtual void collect( std::vector< NonFreed >& nonFreed ) {
      for( size_t i=0; i<nonFreed.size(); ){
        ++nonFreed[i].collectIteration;

        if( nonFreed[i].collectIteration > 3 ){
          deleteObject( nonFreed[i] );
          nonFreed[i] = nonFreed.back();
          nonFreed.pop_back();
          } else {
          ++i;
          }

        }
      }

    void deleteObject( NonFreed& obj ) {
      Holder::deleteObject( obj.data.handle );
      }

  private:
    void createObject( typename Holder::DescriptorType*& t,
                       const char *src,
                       int size, int vsize ) {
      if( needToRestore ){
        typename Holder::DescriptorType* old = t;
        Holder::createObject(t, src, size, vsize );

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
      d.memSize       = vsize*(nearPOT( size*vsize )/vsize);
      d.restoreIntent = false;
      d.elSize        = vsize;

      for( size_t i=0; i<nonFreed.size(); ++i ){
        d.handle = nonFreed[i].data.handle;

        NonFreedData& x = nonFreed[i].data;
        if( d.memSize   <= x.memSize &&
            d.memSize*4 >= x.memSize &&
            d.elSize    == x.elSize ){
          dynVBOs.push_back( nonFreed[i] );
          nonFreed[i] = nonFreed.back();
          nonFreed.pop_back();

          t = dynVBOs.back().data.handle;
          {
            int   sz    = dynVBOs.back().data.memSize,
                  cpySz = size*vsize;
            char *pVertices = this->lockBuffer( t, 0, sz);

            std::copy( src, src+cpySz, pVertices );
            std::fill( pVertices+cpySz, pVertices+sz, 0 );
            this->unlockBuffer( t );
            }

          return;
          }
        }

      std::vector<char> tmp( d.memSize );
      std::copy( src, src+size*vsize, tmp.begin() );
      std::fill( tmp.begin()+size*vsize, tmp.end(), 0 );

      Holder::createObject( t, &tmp[0], d.memSize/d.elSize, d.elSize );

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
          dynVBOs[i].collectIteration = 0;
          nonFreed.push_back( dynVBOs[i] );

          dynVBOs[i] = dynVBOs.back();
          dynVBOs.pop_back();

          if( maxReserved>=0 && int(nonFreed.size())>maxReserved ){
            collect(nonFreed);
            }

          return;
          }

      Holder::deleteObject(t);
      }

    typename Holder::DescriptorType* allocBuffer( size_t size, size_t vsize,
                                                  const void *src ){
      return Holder::allocBuffer(size, vsize, src, AbstractAPI::BU_Dynamic);
      }

    std::vector< NonFreed > nonFreed, dynVBOs;

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
  };

struct LocalVertexBufferHolder: LocalBufferHolder< Tempest::VertexBufferHolder > {
  LocalVertexBufferHolder( Tempest::Device &d ):LocalBufferHolder< Tempest::VertexBufferHolder >(d){}
  };

struct LocalIndexBufferHolder: LocalBufferHolder< Tempest::IndexBufferHolder  > {
  LocalIndexBufferHolder( Tempest::Device &d ):LocalBufferHolder< Tempest::IndexBufferHolder >(d){}
  };
}

#endif // LOCALVERTEXBUFFERHOLDER_H
