#include "abstractapi.h"

#include <cstring>
#include <Tempest/SystemAPI>
#include <algorithm>

using namespace Tempest;

AbstractAPI::Options::Options():displaySettings(-1, -1, 32, false){
  vSync    = false;
  //windowed = true;
  }

bool AbstractAPI::testDisplaySettings( void *hwnd, const DisplaySettings &d ) const {
  return SystemAPI::instance().testDisplaySettings( (SystemAPI::Window*)hwnd, d );
  }

bool AbstractAPI::setDisplaySettings( void *hwnd, const DisplaySettings &d ) const {
  return SystemAPI::instance().setDisplaySettings( (SystemAPI::Window*)hwnd, d );
  }

int AbstractAPI::mipCount(int width, int height, int depth) {
  return std::max( mipCount(width, height), mipCount(depth) );
  }

int AbstractAPI::mipCount(int width, int height) {
  return std::max(mipCount(width), mipCount(height));
  }

int AbstractAPI::mipCount(int width) {
  int mipCount = 0;

  while( width>1 ){
    width/=2;
    ++mipCount;
    }

  return mipCount;
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

int AbstractAPI::vertexCount(AbstractAPI::PrimitiveType t, const int pCount) {
  switch (t) {
    case AbstractAPI::Triangle:
      return pCount*3;

    case AbstractAPI::TriangleStrip:
    case AbstractAPI::TriangleFan:
    case AbstractAPI::LinesStrip:
      return pCount+2;

    case AbstractAPI::Lines:
      return pCount*2;

    case AbstractAPI::Points:
      return pCount;

    default:
      return pCount*3;
      break;
    }
  }


size_t AbstractAPI::primitiveCount(const size_t vert, AbstractAPI::PrimitiveType t) {
  if( t==AbstractAPI::TriangleStrip ||
      t==AbstractAPI::TriangleFan ||
      t==AbstractAPI::LinesStrip  )
    return vert-2;

  if( t==AbstractAPI::Lines )
    return vert/2;

  if( t==Points )
    return vert;

  return vert/3;
  }
