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
                                                            const void * src,
                                                            BufferUsage u ) const {
  AbstractAPI::VertexBuffer* t = createVertexBuffer( d, size, vsize, u );

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
                                                          const void *src,
                                                          BufferUsage u ) const {
  AbstractAPI::IndexBuffer* t = createIndexBuffer( d, size, elSize, u );

  if( !t )
    return 0;

  if(src){
    void *pVertices = lockBuffer( d, t, 0, size*elSize );
    memcpy( pVertices, src, size*elSize );
    unlockBuffer( d, t );
    }

  return t;
  }

int AbstractAPI::vertexCount(AbstractAPI::PrimitiveType t, int pCount) {
  if( t==AbstractAPI::TriangleStrip ||
      t==AbstractAPI::TriangleFan ||
      t==AbstractAPI::LinesStrip  )
    return pCount+2;

  if( t==AbstractAPI::Lines )
    return pCount*2;

  if( t==Points )
    return pCount;

  return pCount*3;
  }
