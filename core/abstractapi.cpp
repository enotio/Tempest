#include "abstractapi.h"

#include <cstring>

using namespace Tempest;

AbstractAPI::Options::Options() {
  vSync    = false;
  windowed = true;
  }


AbstractAPI::VertexBuffer* AbstractAPI::createVertexBuffer( AbstractAPI::Device *d,
                                                            size_t size,
                                                            size_t vsize,
                                                            const void * src ) const {
  AbstractAPI::VertexBuffer* t = createVertexBuffer( d, size, vsize );

  if( !t )
    return 0;

  if(src){
    void *pVertices = lockBuffer( d, t, 0, size*vsize );
    memcpy( pVertices, src, size*vsize );
    unlockBuffer( d, t );
    }

  return t;
  }

AbstractAPI::IndexBuffer *AbstractAPI::createIndexBuffer( AbstractAPI::Device *d,
                                                          size_t size,
                                                          size_t elSize,
                                                          const void *src ) const {
  AbstractAPI::IndexBuffer* t = createIndexBuffer( d, size, elSize );

  if( !t )
    return 0;

  if(src){
    void *pVertices = lockBuffer( d, t, 0, size*elSize );
    memcpy( pVertices, src, size*elSize );
    unlockBuffer( d, t );
    }

  return t;
  }
