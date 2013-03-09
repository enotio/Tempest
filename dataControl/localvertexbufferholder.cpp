#include "localvertexbufferholder.h"

#include <cstring>

#include <Tempest/Device>

using namespace Tempest;

LocalVertexBufferHolder::LocalVertexBufferHolder( Tempest::Device &d )
                        :Tempest::VertexBufferHolder(d) {
  nonFreed.reserve(128);
  dynVBOs .reserve(128);

  setReserveSize( 32*8092 );

  needToRestore = false;
  }

LocalVertexBufferHolder::~LocalVertexBufferHolder() {
  reset();
  }

void LocalVertexBufferHolder::setReserveSize(int sz) {
  reserveSize = sz;
  }

void LocalVertexBufferHolder::reset() {
  needToRestore = true;

  for( size_t i=0; i<nonFreed.size(); ++i )
    deleteObject( nonFreed[i] );

  nonFreed.clear();
  //dynVBOs .clear();
  //for( size_t i=0; i<dynVBOs.size(); ++i )
    //reset( dynVBOs[i].data.handle );

  VertexBufferHolder::BaseType::reset();
  }

bool LocalVertexBufferHolder::restore() {
  for( size_t i=0; i<dynVBOs.size(); ++i )
    dynVBOs[i].data.restoreIntent = true;

  bool ok = VertexBufferHolder::BaseType::restore();
  needToRestore = false;
  return ok;
  }

void LocalVertexBufferHolder::presentEvent() {
  collect( nonFreed );
  }

void LocalVertexBufferHolder::collect(std::vector<NonFreed> &nonFreed) {
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

void LocalVertexBufferHolder::deleteObject(LocalVertexBufferHolder::NonFreed &obj) {
  Tempest::VertexBufferHolder::deleteObject( obj.data.handle );
  }

void LocalVertexBufferHolder::createObject( AbstractAPI::VertexBuffer*& t,
                                            const char *src,
                                            int size, int vsize ) {
  if( needToRestore ){
    AbstractAPI::VertexBuffer* old = t;
    VertexBufferHolder::createObject(t, src, size, vsize );

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
  d.memSize       = nearPOT( size*vsize );
  d.restoreIntent = false;

  for( size_t i=0; i<nonFreed.size(); ++i ){
    d.handle = nonFreed[i].data.handle;

    NonFreedData& x = nonFreed[i].data;
    if( d.memSize   <= x.memSize &&
        d.memSize*4 >= x.memSize ){
      dynVBOs.push_back( nonFreed[i] );
      nonFreed[i] = nonFreed.back();
      nonFreed.pop_back();

      t = dynVBOs.back().data.handle;
      {
        int sz = dynVBOs.back().data.memSize;
        char *pVertices = lockBuffer( t, 0, sz);
        memcpy( pVertices, src, size*vsize );

        std::fill( pVertices+size*vsize, pVertices+sz, 0 );
        unlockBuffer( t );
        }

      return;
      }
    }

  std::vector<char> tmp( d.memSize );
  std::copy( src, src+size*vsize, tmp.begin() );
  std::fill( tmp.begin()+size*vsize, tmp.end(), 0 );

  Tempest::VertexBufferHolder::createObject( t, &tmp[0], d.memSize, 1 );

  if( !t )
    return;

  d.handle = t;

  NonFreed nf;
  nf.data = d;
  nf.collectIteration = 0;
  nf.userPtr = 0;

  dynVBOs.push_back( nf );
  }

void LocalVertexBufferHolder::deleteObject(AbstractAPI::VertexBuffer *t ) {
  for( size_t i=0; i<dynVBOs.size(); ++i )
    if( dynVBOs[i].data.handle==t ){
      dynVBOs[i].collectIteration = 0;
      nonFreed.push_back( dynVBOs[i] );

      dynVBOs[i] = dynVBOs.back();
      dynVBOs.pop_back();

      return;
      }

  Tempest::VertexBufferHolder::deleteObject(t);
  }

int LocalVertexBufferHolder::nearPOT(int x) {
  int r = 1;
  for( int i=0; r<=reserveSize; ++i ){
    if( r>=x )
      return r;
    r *= 2;
    }

  return x;
  }
