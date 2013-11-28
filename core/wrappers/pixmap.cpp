#include "pixmap.h"

#include <Tempest/SystemAPI>
#include <iostream>
#include "core/wrappers/atomic.h"

#include <squish/squish.h>
#include <Tempest/Assert>
#include <cstring>

using namespace Tempest;

struct Pixmap::MemPool{
  MemPool():count(0){
    T_ASSERT( sizeof(Pixmap::Pixel)==4 );
    data.reserve(128);
    ldata.reserve(8);
    }

  ~MemPool(){
    for( size_t i=0; i<data.size(); ++i )
      pool.free( data[i] );

    for( size_t i=0; i<ldata.size(); ++i )
      pool.free( ldata[i] );

    T_ASSERT_X( count==0, "pixmap leak detected" );
    }

  Pixmap::Data* alloc( int w, int h, int bpp ){
    ++count;

    size_t sz = w*h*bpp, sz64 = std::max<size_t>(64*64*4, sz);
    if( sz==0 ){
      Pixmap::Data* d = pool.alloc();
      //d->bytes.reserve( 64*64*4 );
      return d;
      }

    for( size_t i=0; i<data.size(); ++i ){
      std::vector<unsigned char>& b = data[i]->bytes;
      if( b.size()<=sz64 && sz64<=b.capacity()  ){
        Pixmap::Data* r = data[i];
        data[i] = data.back();
        data.pop_back();
        b.resize(sz);
        return r;
        }
      }

    for( size_t i=0; i<ldata.size(); ++i ){
      std::vector<unsigned char>& b = ldata[i]->bytes;
      if( b.size()<=sz64 && sz64<=b.capacity() ){
        Pixmap::Data* r = ldata[i];
        ldata[i] = ldata.back();
        ldata.pop_back();
        b.resize(sz);
        return r;
        }
      }

    Pixmap::Data* d = pool.alloc();

    if( w*h*bpp<=64*64*4 ){
      d->bytes.reserve( 64*64*4 );
      d->bytes.resize(w*h*bpp);
      } else {
      d->bytes.resize(w*h*bpp);
      d->bytes.reserve( 64*64*4 );
      }
    return d;
    }

  void free( Pixmap::Data* p ){
    --count;
    if( p->bytes.size() && p->bytes.capacity()<=64*64*4 && data.size()<128 ){
      data.push_back(p);
      return;
      }

    if( p->bytes.size() && p->bytes.capacity()<=512*512*4 && ldata.size()<16 ){
      ldata.push_back(p);
      return;
      }

    pool.free(p);
    }

  std::vector<Pixmap::Data*> data, ldata;
  size_t count;

  Tempest::MemPool<Pixmap::Data> pool;
  };

Pixmap::MemPool Pixmap::pool;

Tempest::MemPool<Pixmap::DbgManip::Ref> Pixmap::DbgManip::ref_pool;


Pixmap::DbgManip::Ref * Pixmap::DbgManip::newRef(){
  return ref_pool.alloc( T() );//new Ref( T() );
  }

Pixmap::DbgManip::Ref * Pixmap::DbgManip::newRef( const Ref * base ){
  Ref *r = newRef();
  r->data = pool.alloc( base->data->bytes.size(), 1, 1 );

  std::copy( base->data->bytes.begin(),
             base->data->bytes.end(),
             r->data->bytes.begin() );
  return r;
  }

void Pixmap::DbgManip::delRef( Ref * r ){
  pool.free(r->data);
  //delete r->data;
  ref_pool.free(r);
  //delete r;
  }

Pixmap::Pixmap() {
  rawPtr  = 0;
  mrawPtr = 0;
  }

Tempest::Pixmap::Pixmap(const std::string &p) {
  load(p);
  }

Tempest::Pixmap::Pixmap(const std::wstring &p) {
  load(p);
  }

Pixmap::Pixmap(int iw, int ih, bool alpha) {
  info.w = iw;
  info.h = ih;

  if( alpha ){
    info.bpp    = 4;
    info.format = Format_RGBA;
    }else{
    info.bpp    = 3;
    info.format = Format_RGB;
    }

  info.alpha = alpha;

  data.value() = pool.alloc(info.w,info.h,info.bpp);

  //data.value() = new Data();
  //data.value()->bytes.resize(mw*mh*bpp);
  rawPtr = &data.const_value()->bytes[0];
  mrawPtr = 0;

  T_ASSERT( sizeof(Pixel)==4 );
  }

Pixmap::Pixmap(const Pixmap &p):data(p.data) {
  info   = p.info;

  if( info.w>0 && info.h>0 )
    rawPtr = &data.const_value()->bytes[0]; else
    rawPtr = 0;
  mrawPtr = 0;
  }

Pixmap &Pixmap::operator = (const Pixmap &p) {
  data = p.data;
  info = p.info;

  if( info.w>0 && info.h>0 )
    rawPtr = &data.const_value()->bytes[0]; else
    rawPtr = 0;
  mrawPtr = 0;

  return *this;
  }

bool Pixmap::save(const std::string &f) {
  return save(f.data());
  }

bool Pixmap::save(const std::wstring &f) {
  return save(f.data());
  }

bool Pixmap::save(const char* f) {
  return implSave<const char*>(f);
  }

bool Pixmap::save(const wchar_t* f) {
  return implSave<const wchar_t*>(f);
  }

bool Pixmap::load(const std::string &f) {
  return load(f.data());
  }

bool Pixmap::load( const std::wstring &f ) {
  return load(f.data());
  }

bool Pixmap::load(const wchar_t *f) {
  return implLoad<const wchar_t*>(f);
  }

bool Pixmap::load(const char *f) {
  return implLoad<const char*>(f);
  }

template< class T >
bool Pixmap::implLoad( T f ) {
  Data * image = pool.alloc(0,0,0);
  ImgInfo tmpInfo;
  bool ok = SystemAPI::loadImage( f, tmpInfo, image->bytes );

  if( !ok ){
    pool.free(image);
    return false;
    }

  info = tmpInfo;
  this->data.value() = image;

  if( info.w>0 && info.h>0 )
    rawPtr = &this->data.const_value()->bytes[0]; else
    rawPtr = 0;
  mrawPtr = 0;
  return true;
  }

bool Pixmap::hasAlpha() const {
  return info.alpha;
  }

void Pixmap::addAlpha() {
  if( hasAlpha() )
    return;
  makeEditable();

  data.value()->bytes.resize( info.w*info.h*4);
  unsigned char * v = &data.value()->bytes[0];

  for( size_t i=info.w*info.h; i>0; --i ){
    unsigned char * p = &v[ 4*i-4 ];
    unsigned char * s = &v[ 3*i-3 ];
    for( int r=0; r<3; ++r )
      p[r] = s[r];
    p[3] = 255;
    }

  rawPtr   = &data.const_value()->bytes[0];
  mrawPtr  = 0;
  info.bpp = 4;
  info.alpha = true;
  }

void Pixmap::removeAlpha() {
  if( !hasAlpha() )
    return;
  makeEditable();

  unsigned char * v = &data.value()->bytes[0];

  for( int i=0; i<info.w*info.h; ++i ){
    unsigned char * p = &v[ 3*i ];
    unsigned char * s = &v[ 4*i ];
    for( int r=0; r<3; ++r )
      p[r] = s[r];
    }

  data.value()->bytes.resize(info.w*info.h*3);
  rawPtr   = &data.const_value()->bytes[0];
  mrawPtr  = 0;
  info.bpp = 3;
  info.alpha = true;
  }

void Pixmap::setupRawPtrs() const {
  verifyFormatEditable();

  if( true || !mrawPtr ){
    mrawPtr = &data.value()->bytes[0];
    rawPtr  = mrawPtr;
    }
  }

void Pixmap::setFormat( Pixmap::Format f ) {
  if( info.format==f )
    return;

  if( data.isNull() ){
    info.format = f;
    return;
    }

  (void)data.value();

  if( info.format==Format_RGB && f==Format_RGBA ){
    addAlpha();
    return;
    }

  if( info.format==Format_RGBA && f==Format_RGB ){
    removeAlpha();
    return;
    }

  if( info.format>=Format_DXT1 && info.format<=Format_DXT5 &&
      f==Format_RGB ){
    makeEditable();
    removeAlpha();
    return;
    }

  if( info.format>=Format_DXT1 && info.format<=Format_DXT5 &&
      f==Format_RGBA ){
    makeEditable();
    addAlpha();
    return;
    }

  if( info.format==Format_ETC1_RGB8 && f==Format_RGB ){
    makeEditable();
    return;
    }

  if( info.format==Format_ETC1_RGB8 && f==Format_RGBA ){
    makeEditable();
    addAlpha();
    return;
    }

  SystemAPI &api = SystemAPI::instance();

  if( info.format==Format_RGB || info.format==Format_RGBA ){
    for( size_t i=0; i<api.imageCodecCount(); ++i ){
      if( api.imageCodec(i).canConvertTo(info, f) ){
        api.imageCodec(i).fromRGB(info, data.value()->bytes);
        return;
        }
      }
    }

  T_WARNING_X(0, "Pixmap::makeEditable : no convarsion found");
  }

void Pixmap::makeEditable() {
  (void)data.value();

  if( info.format==Format_RGB ||
      info.format==Format_RGBA ){
    setupRawPtrs();
    return;
    }

  SystemAPI& api = SystemAPI::instance();
  for( size_t i=0; i<api.imageCodecCount(); ++i ){
    if( api.imageCodec(i).canConvertTo(info, Format_RGB) ){
      api.imageCodec(i).toRGB(info, data.value()->bytes, false);
      setupRawPtrs();
      return;
      }

    if( api.imageCodec(i).canConvertTo(info, Format_RGBA) ){
      api.imageCodec(i).toRGB(info, data.value()->bytes, true);
      setupRawPtrs();
      return;
      }
    }

  T_WARNING_X(0, "Pixmap::makeEditable : no convarsion found");
  }

template< class T >
bool Pixmap::implSave( T f ) {
  if( data.const_value()==0 )
    return false;

  Tempest::Detail::GuardBase< Detail::Ptr<Data*, DbgManip> > guard( data );
  (void)guard;

  bool ok = SystemAPI::saveImage( f,
                                  info,
                                  data.const_value()->bytes );
  return ok;
  }

void Pixmap::fill(const Pixmap::Pixel &p) {
  if( info.w==0 || info.h==0 )
    return;

  verifyFormatEditable();

  size_t sz = data.value()->bytes.size();
  mrawPtr   = &data.value()->bytes[0];
  rawPtr    = mrawPtr;

  unsigned char px[] = {p.r, p.g, p.b, p.a};

  if( info.bpp==3 ){
    for( size_t i=0; i<sz; i+=3 ){
      mrawPtr[i+0] = px[0];
      mrawPtr[i+1] = px[1];
      mrawPtr[i+2] = px[2];
      }
    } else {
    for( size_t i=0; i<sz; i+=4 ){
      mrawPtr[i+0] = px[0];
      mrawPtr[i+1] = px[1];
      mrawPtr[i+2] = px[2];
      mrawPtr[i+3] = px[3];
      }
    }
  }


PixEditor::PixEditor(Pixmap &p) : out(p) {
  }

void PixEditor::copy(int x, int y, const Pixmap &p) {
  int w = std::min(p.width(),  out.width()  - x );
  int h = std::min(p.height(), out.height() - y );

  int px = 0, py = 0;

  if( x<0 ){
    px = -x;
    w += x;
    x = 0;
    }

  if( y<0 ){
    py = -y;
    h += y;
    y = 0;
    }

  int xm = x+w, ym = y+h;

  if( p.hasAlpha() )
    out.addAlpha();

  out.setupRawPtrs();
  p.setupRawPtrs();

  if( p.info.bpp==out.info.bpp ){
    for( int r=y; r<ym; ++r, ++py ){
            unsigned char * ov = &out.mrawPtr[ (x  + r *out.info.w)*out.info.bpp ];
      const unsigned char * iv = &  p.rawPtr [ (px + py*  p.info.w)*  p.info.bpp ];

      memcpy(ov, iv, (xm-x)*p.info.bpp );
      }
    } else {
    for( int r=y; r<ym; ++r, ++py ){
      int lpx = px;
      for( int i=x; i<xm; ++i, ++lpx ){
        out.set(i,r, p.at(lpx,py) );
        }
      }
    }
  }

void PixEditor::draw(int x, int y, const Pixmap &p) {
  int w = std::min(p.width(),  out.width()  - x );
  int h = std::min(p.height(), out.height() - y );

  int px = 0, py = 0;

  if( x<0 ){
    px = -x;
    w += x;
    x = 0;
    }

  if( y<0 ){
    py = -y;
    h += y;
    y = 0;
    }

  int xm = x+w, ym = y+h;

  for( int r=y; r<ym; ++r, ++py ){
    int lpx = px;
    for( int i=x; i<xm; ++i, ++lpx ){
      out.set(i,r, p.at(lpx,py) );
      }
    }
  }

Pixmap::ImgInfo::ImgInfo():w(0), h(0), bpp(0), alpha(false), format(Format_RGB) {

  }
