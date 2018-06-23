#include "pixmap.h"

#include <Tempest/SystemAPI>
#include <iostream>
#include "core/wrappers/atomic.h"

#include <squish/squish.h>
#include <Tempest/Assert>
#include <Tempest/File>
#include <cstring>
#include <algorithm>
#include <mutex>

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

    //T_ASSERT_X( count==0, "pixmap leak detected" );
    }

  Pixmap::Data* alloc( int w, int h, int bpp ){
    std::lock_guard<std::mutex> guard(sync);
    ++count;

    size_t sz = w*h*bpp, sz64 = std::max<size_t>(64*64*4, sz);
    if( sz==0 ){
      Pixmap::Data* d = pool.alloc();
      //d->bytes.reserve( 64*64*4 );
      return d;
      }

    for( size_t i=0; i<data.size(); ++i ){
      std::vector<uint8_t>& b = data[i]->bytes;
      if( b.size()<=sz64 && sz64<=b.capacity()  ){
        Pixmap::Data* r = data[i];
        data[i] = data.back();
        data.pop_back();
        b.resize(sz);
        return r;
        }
      }

    for( size_t i=0; i<ldata.size(); ++i ){
      std::vector<uint8_t>& b = ldata[i]->bytes;
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
      d->bytes.resize(sz);
      } else {
      d->bytes.resize(sz);
      d->bytes.reserve( 64*64*4 );
      }
    return d;
    }

  void free( Pixmap::Data* p ){
    std::lock_guard<std::mutex> guard(sync);
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

  std::mutex                     sync;
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

Pixmap::Pixmap(const char *p) {
  load(p);
  }

Tempest::Pixmap::Pixmap(const std::u16string &p) {
  load(p);
  }

Pixmap::Pixmap(const char16_t *p) {
  load(p);
  }

Pixmap::Pixmap(int iw, int ih, bool alpha) {
  info.w = iw;
  info.h = ih;
  info.mipLevels=1;

  if( alpha ){
    info.bpp    = 4;
    info.format = Format_RGBA;
    }else{
    info.bpp    = 3;
    info.format = Format_RGB;
    }

  info.alpha = alpha;

  imgData.value() = pool.alloc(info.w,info.h,info.bpp);

  //data.value() = new Data();
  //data.value()->bytes.resize(mw*mh*bpp);
  if(imgData.const_value()->bytes.size()>0)
    rawPtr = &imgData.const_value()->bytes[0];
  mrawPtr = 0;

  T_ASSERT( sizeof(Pixel)==4 );
  }

Pixmap::Pixmap(const Pixmap &p):imgData(p.imgData) {
  info   = p.info;

  if( info.w>0 && info.h>0 )
    rawPtr = &imgData.const_value()->bytes[0]; else
    rawPtr = 0;
  mrawPtr = 0;
  }

Pixmap &Pixmap::operator = (const Pixmap &p) {
  imgData = p.imgData;
  info    = p.info;

  if( info.w>0 && info.h>0 )
    rawPtr = &imgData.const_value()->bytes[0]; else
    rawPtr = 0;
  mrawPtr = 0;

  return *this;
  }

bool Pixmap::save(const std::string &f, const char *ext)  const {
  return save(f.data(),ext);
  }

bool Pixmap::save(const std::u16string &f, const char *ext)  const {
  return save(f.data(),ext);
  }

bool Pixmap::save(const char* f, const char *ext)  const {
  WFile fn(f);
  return save(fn,ext);
  }

bool Pixmap::save(const char16_t* f, const char *ext)  const {
  WFile fn(f);
  return save(fn,ext);
  }

bool Pixmap::load(const std::string &f) {
  return load(f.data());
  }

bool Pixmap::load( const std::u16string &f ) {
  return load(f.data());
  }

bool Pixmap::load(const char16_t *f) {
  RFile file(f);
  return load(file);
  }

bool Pixmap::load(const char *f) {
  RFile file(f);
  return load(file);
  }

bool Pixmap::save( ODevice& f, const char* ext ) const {
  if( imgData.const_value()==0 )
    return false;

  Tempest::Detail::GuardBase< Detail::Ptr<Data*, DbgManip> > guard( imgData );
  (void)guard;

  bool ok = SystemAPI::saveImage( f, info, ext,
                                  imgData.const_value()->bytes );
  return ok;
  }

bool Pixmap::load( IDevice& file ) {
  Data * image = pool.alloc(0,0,0);
  ImgInfo tmpInfo;
  bool ok = SystemAPI::loadImage( file, tmpInfo, image->bytes );

  if( !ok ){
    *this = Pixmap();
    pool.free(image);
    return false;
    }

  info = tmpInfo;
  imgData = Detail::Ptr<Data*, DbgManip>();
  imgData.value() = image;

  if( info.w>0 && info.h>0 )
    rawPtr = &imgData.const_value()->bytes[0]; else
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

  auto& vec = imgData.value()->bytes;
  vec.resize(info.w*info.h*4);
  uint8_t * v = &vec[0];

  for( size_t i=vec.size()/4; i>0; ){
    --i;
    uint8_t * p = &v[ 4*i ];
    uint8_t * s = &v[ 3*i ];

    p[0] = s[0];
    p[1] = s[1];
    p[2] = s[2];
    p[3] = 255;
    }

  rawPtr      = &vec[0];
  mrawPtr     = 0;
  info.bpp    = 4;
  info.format = Format_RGBA;
  info.alpha  = true;
  }

void Pixmap::removeAlpha() {
  if( !hasAlpha() )
    return;
  makeEditable();

  uint8_t * v = &imgData.value()->bytes[0];

  for( int i=0; i<info.w*info.h; ++i ){
    uint8_t * p = &v[ 3*i ];
    uint8_t * s = &v[ 4*i ];
    for( int r=0; r<3; ++r )
      p[r] = s[r];
    }

  imgData.value()->bytes.resize(info.w*info.h*3);
  rawPtr   = &imgData.const_value()->bytes[0];
  mrawPtr  = 0;
  info.bpp = 3;
  info.format = Format_RGB;
  info.alpha = true;
  }

void Pixmap::setupRawPtrs() const {
  verifyFormatEditable();

  if( true || !mrawPtr ){
    mrawPtr = &imgData.value()->bytes[0];
    rawPtr  = mrawPtr;
    }
  }

void Pixmap::setFormat( Pixmap::Format f ) {
  if( info.format==f )
    return;

  if( imgData.isNull() ){
    info.format = f;
    return;
    }

  (void)imgData.value();

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
        api.imageCodec(i).fromRGB(info, imgData.value()->bytes);
        return;
        }
      }
    }

  T_WARNING_X(0, "Pixmap::makeEditable : no convarsion found");
  }

void Pixmap::makeEditable() {
  (void)imgData.value();

  if( info.format==Format_RGB ||
      info.format==Format_RGBA ){
    setupRawPtrs();
    return;
    }

  SystemAPI& api = SystemAPI::instance();
  for( size_t i=0; i<api.imageCodecCount(); ++i ){
    if( api.imageCodec(i).canConvertTo(info, Format_RGB) ){
      api.imageCodec(i).toRGB(info, imgData.value()->bytes, false);
      info.mipLevels = 0;
      setupRawPtrs();
      return;
      }

    if( api.imageCodec(i).canConvertTo(info, Format_RGBA) ){
      api.imageCodec(i).toRGB(info, imgData.value()->bytes, true);
      info.mipLevels = 0;
      setupRawPtrs();
      return;
      }
    }

  T_WARNING_X(0, "Pixmap::makeEditable : no conversion found");
  }

void Pixmap::fill(const Pixmap::Pixel &p) {
  if( info.w==0 || info.h==0 )
    return;

  verifyFormatEditable();

  size_t sz = imgData.value()->bytes.size();
  mrawPtr   = &imgData.value()->bytes[0];
  rawPtr    = mrawPtr;

  uint8_t px[] = {p.r, p.g, p.b, p.a};

  if( !info.alpha ){
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
            uint8_t * ov = &out.mrawPtr[ (x  + r *out.info.w)*out.info.bpp ];
      const uint8_t * iv = &  p.rawPtr [ (px + py*  p.info.w)*  p.info.bpp ];

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

void Tempest::Pixmap::downsample() {
  verifyFormatEditable();
  ImageCodec::downsample(info,imgData.value()->bytes);
  }

inline static uint32_t nextPot( uint32_t v ){
  v--;
  v |= v >> 1;
  v |= v >> 2;
  v |= v >> 4;
  v |= v >> 8;
  v |= v >> 16;
  v++;

  return v;
  }

void Pixmap::toPOT(int maxSize) {
  verifyFormatEditable();

  uint32_t sz = maxSize;
  ImageCodec::resize( info,
                      imgData.value()->bytes,
                      std::min( nextPot(info.w)/2, sz ),
                      std::min( nextPot(info.h)/2, sz ) );
  }


Pixmap::ImgInfo::ImgInfo()
  :w(0), h(0), bpp(0), mipLevels(0), alpha(false), format(Format_RGB) {
  }
