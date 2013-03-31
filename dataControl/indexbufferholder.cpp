#include "indexbufferholder.h"

#include <Tempest/Texture2d>
#include <Tempest/Device>

#include <map>

#include <cstring>

using namespace Tempest;

struct IndexBufferHolder::Data {
  struct LDData{
    int vsize, size;

    int lockBegin, lockSize;
    size_t minID;
    std::vector< char > data;
    // const void * data;
    };

  std::map< AbstractAPI::IndexBuffer*, LDData* > ibos, restore;
  typedef std::map< AbstractAPI::IndexBuffer*, LDData* >::iterator Iterator;
  };

IndexBufferHolder::IndexBufferHolder( Device& d):BaseType(d) {
  data = new Data();
  }

IndexBufferHolder::~IndexBufferHolder(){
  delete data;
  }

IndexBufferHolder::IndexBufferHolder( const IndexBufferHolder& h)
                   :BaseType( h.device() ) {
  }

void IndexBufferHolder::createObject( AbstractAPI::IndexBuffer*& t,
                                       const char * src,
                                       int size, int vsize ){
  int c = 100;
  t = 0;
  while( !t && c>0 ){
    t = device().createIndexBuffer( size, vsize );
    --c;
    }

  if( !t )
    return;

  {
    void *pVertices = device().lockBuffer( t, 0, size*vsize );
    memcpy( pVertices, src, size*vsize );
    device().unlockBuffer( t );
    }

  Data::LDData *d = new Data::LDData;
  d->vsize = vsize;
  d->size  = size;
  //d.data  = src;

  d->data.resize( size*vsize );
  std::copy( src, src + size*vsize, d->data.begin() );

  data->ibos[t] = d;
  }

void IndexBufferHolder::deleteObject( AbstractAPI::IndexBuffer* t ){
  Data::Iterator i = data->ibos.find(t);

  if( i!=data->ibos.end() ){
    delete i->second;
    data->ibos.erase(i);
    }

  device().deleteIndexBuffer(t);
  }

void IndexBufferHolder::reset( AbstractAPI::IndexBuffer* t ){
  Data::Iterator i = data->ibos.find(t);
  data->restore[t] = i->second;

  data->ibos.erase(i);
  device().deleteIndexBuffer(t);
  }

AbstractAPI::IndexBuffer* IndexBufferHolder::restore( AbstractAPI::IndexBuffer* t ){
  Data::LDData* ld = data->restore[t];
  data->restore.erase(t);

  createObject( t, &ld->data[0], ld->size, ld->vsize );

  delete ld;
  return t;
  }

AbstractAPI::IndexBuffer* IndexBufferHolder::copy( AbstractAPI::IndexBuffer* t ){
  Data::LDData& ld = *data->ibos[t];

  AbstractAPI::IndexBuffer* ret = 0;
  createObject( ret, &ld.data[0], ld.size, ld.vsize );
  return ret;
  }

char *IndexBufferHolder::lockBuffer( AbstractAPI::IndexBuffer *t,
                                     int b, int sz ) {
  Data::LDData & ld = *data->ibos[t];
  ld.lockBegin = b;
  ld.lockSize  = sz;

  return &ld.data[0];//(char*)device().lockBuffer( t, b, sz);
  }

void IndexBufferHolder::unlockBuffer(AbstractAPI::IndexBuffer *t) {
  Data::LDData & ld = *data->ibos[t];

  char *v = (char*)device().lockBuffer( t, ld.lockBegin, ld.lockSize );
  memcpy( v, (&ld.data[0])+ld.lockBegin, ld.lockSize );
  device().unlockBuffer(t);
  }

void IndexBufferHolder::unlockNWBuffer(AbstractAPI::IndexBuffer *t) {

  }
