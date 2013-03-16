#include "vertexbufferholder.h"

#include <Tempest/Texture2d>
#include <Tempest/Device>

#include <map>

#include <cstring>

using namespace Tempest;

struct VertexBufferHolder::Data {
  struct LDData{
    int vsize, size;

    int lockBegin, lockSize;
    std::vector< char > data;
    // const void * data;
    };

  std::map< AbstractAPI::VertexBuffer*, LDData* > vbos, restore;
  typedef std::map< AbstractAPI::VertexBuffer*, LDData* >::iterator Iterator;
  };

VertexBufferHolder::VertexBufferHolder( Device& d):BaseType(d) {
  data = new Data();
  }

VertexBufferHolder::~VertexBufferHolder(){
  delete data;
  }

VertexBufferHolder::VertexBufferHolder( const VertexBufferHolder& h)
  :BaseType( h.device() ) {
  data = new Data();
  }

void VertexBufferHolder::createObject( AbstractAPI::VertexBuffer*& t,
                                       const char * src,
                                       int size, int vsize ){
  int c = 100;
  t = 0;
  while( !t && c>0 ){
    t = device().createVertexbuffer( size, vsize );
    --c;
    }

  if( !t )
    return;

  {
    void *pVertices = device().lockBuffer( t, 0, size*vsize );
    memcpy( pVertices, src, size*vsize );
    device().unlockBuffer( t );
    }

  Data::LDData *d = new Data::LDData();
  d->vsize = vsize;
  d->size  = size;
  d->lockBegin = 0;
  d->lockSize  = 0;
  //d.data  = src;

  d->data.resize( size*vsize );
  std::copy( src, src + size*vsize, d->data.begin() );

  data->vbos[t] = d;
  }

void VertexBufferHolder::deleteObject( AbstractAPI::VertexBuffer* t ){
  Data::Iterator i = data->vbos.find(t);
  delete i->second;

  data->vbos.erase(i);
  device().deleteVertexBuffer(t);
  }

void VertexBufferHolder::reset( AbstractAPI::VertexBuffer* t ){
  Data::Iterator i = data->vbos.find(t);
  data->restore[t] = i->second;

  data->vbos.erase(i);
  device().deleteVertexBuffer(t);
  }

AbstractAPI::VertexBuffer* VertexBufferHolder::restore( AbstractAPI::VertexBuffer* t ){
  Data::LDData* ld = data->restore[t];
  data->restore.erase(t);

  createObject( t, &ld->data[0], ld->size, ld->vsize );
  delete ld;

  return t;
  }

AbstractAPI::VertexBuffer* VertexBufferHolder::copy( AbstractAPI::VertexBuffer* t ){
  Data::LDData& ld = *data->vbos[t];

  AbstractAPI::VertexBuffer* ret = 0;
  createObject( ret, &ld.data[0], ld.size, ld.vsize );
  return ret;
  }

char *VertexBufferHolder::lockBuffer( AbstractAPI::VertexBuffer *t,
                                      int b, int sz ) {
  Data::LDData & ld = *data->vbos[t];
  ld.lockBegin = b;
  ld.lockSize  = sz;

  return &ld.data[0];//(char*)device().lockBuffer( t, b, sz);
  }

void VertexBufferHolder::unlockBuffer(AbstractAPI::VertexBuffer *t) {
  Data::LDData & ld = *data->vbos[t];

  char *v = (char*)device().lockBuffer( t, ld.lockBegin, ld.lockSize );
  memcpy( v, (&ld.data[0])+ld.lockBegin, ld.lockSize );
  device().unlockBuffer(t);
  }

void VertexBufferHolder::unlockNWBuffer(AbstractAPI::VertexBuffer *t) {

  }
