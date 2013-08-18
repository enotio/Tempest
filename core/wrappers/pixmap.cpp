#include "pixmap.h"

#include <Tempest/SystemAPI>
#include <iostream>
#include "core/wrappers/atomic.h"

#include <squish/squish.h>
#include <Tempest/Assert>
#include <cstring>

using namespace Tempest;


Pixmap::DbgManip::Ref * Pixmap::DbgManip::newRef( const Ref * base ){
  return new Ref( new Data(*base->data) );
  }

void Pixmap::DbgManip::delRef( Ref * r ){
  delete r->data;
  delete r;
  }

Pixmap::Pixmap() {
  mw = 0;
  mh = 0;

  bpp = 3;
  rawPtr  = 0;
  mrawPtr = 0;

  frm = Format_RGB;

  T_ASSERT( sizeof(Pixel)==4 );
  }

Tempest::Pixmap::Pixmap(const std::string &p) {
  mw = 0;
  mh = 0;

  bpp = 3;
  frm = Format_RGB;

  load(p);

  T_ASSERT( sizeof(Pixel)==4 );
  }

Tempest::Pixmap::Pixmap(const std::wstring &p) {
  mw = 0;
  mh = 0;

  bpp = 3;
  frm = Format_RGB;

  load(p);

  T_ASSERT( sizeof(Pixel)==4 );
  }

Pixmap::Pixmap(int iw, int ih, bool alpha) {
  mw = iw;
  mh = ih;

  if( alpha ){
    bpp = 4;
    frm = Format_RGBA;
    }else{
    bpp = 3;
    frm = Format_RGB;
    }

  data.value() = new Data();
  data.value()->bytes.resize(mw*mh*bpp);
  rawPtr = &data.const_value()->bytes[0];
  mrawPtr = 0;

  T_ASSERT( sizeof(Pixel)==4 );
  }

Pixmap::Pixmap(const Pixmap &p):data(p.data) {
  mw   = p.mw;
  mh   = p.mh;
  bpp  = p.bpp;
  frm  = p.frm;

  if( mw>0 && mh>0 )
    rawPtr = &data.const_value()->bytes[0]; else
    rawPtr = 0;
  mrawPtr = 0;
  }

Pixmap &Pixmap::operator =(const Pixmap &p) {
  data = p.data;

  mw  = p.mw;
  mh  = p.mh;
  bpp = p.bpp;
  frm = p.frm;

  if( mw>0 && mh>0 )
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
  int w = 0, h = 0, bpp = 0;

  Data * image = new Data();
  bool ok = SystemAPI::loadImage( f, w, h, bpp, image->bytes );

  if( !ok ){
    delete image;
    return false;
    }

  switch( bpp ){
    case -5: frm = Format_DXT5; break;
    case -3: frm = Format_DXT3; break;
    case -1: frm = Format_DXT1; break;
    case  3: frm = Format_RGB;  break;
    case  4: frm = Format_RGBA; break;
    }

  this->mw  = w;
  this->mh  = h;
  this->bpp = bpp;

  this->data.value() = image;

  if( w>0 && h>0 )
    rawPtr = &this->data.const_value()->bytes[0]; else
    rawPtr = 0;
  mrawPtr = 0;
  return true;
  }

bool Pixmap::hasAlpha() const {
  return frm!=Format_RGB;
  }

void Pixmap::addAlpha() {
  if( hasAlpha() )
    return;

  data.value()->bytes.resize(mw*mh*4);
  unsigned char * v = &data.value()->bytes[0];

  for( size_t i=mw*mh; i>0; --i ){
    unsigned char * p = &v[ 4*i-4 ];
    unsigned char * s = &v[ 3*i-3 ];
    for( int r=0; r<3; ++r )
      p[r] = s[r];
    p[3] = 255;
    }

  rawPtr  = &data.const_value()->bytes[0];
  mrawPtr = 0;
  bpp     = 4;
  }

void Pixmap::removeAlpha() {
  if( !hasAlpha() )
    return;

  unsigned char * v = &data.value()->bytes[0];

  for( int i=0; i<mw*mh; ++i ){
    unsigned char * p = &v[ 3*i ];
    unsigned char * s = &v[ 4*i ];
    for( int r=0; r<3; ++r )
      p[r] = s[r];
    }

  data.value()->bytes.resize(mw*mh*3);
  rawPtr  = &data.const_value()->bytes[0];
  mrawPtr = 0;
  bpp     = 3;
  }

void Pixmap::setFormat( Pixmap::Format f ) {
  if( frm==f )
    return;

  if( frm==Format_RGB && f==Format_RGBA )
    addAlpha();

  if( frm==Format_RGBA && f==Format_RGB )
    removeAlpha();

  if( frm>=Format_DXT1 && frm<=Format_DXT5 &&
      f==Format_RGB ){
    makeEditable();
    removeAlpha();
    }

  if( frm>=Format_DXT1 && frm<=Format_DXT5 &&
      f==Format_RGBA ){
    makeEditable();
    addAlpha();
    }

  if( ( frm == Format_RGB || frm == Format_RGBA ) &&
      f>=Format_DXT1 && f<=Format_DXT5 ){
    toS3tc(f);
    }

  frm = f;
  }

void Pixmap::makeEditable() {
  squish::u8 pixels[4][4][4];
  int compressType = squish::kDxt1;

  size_t sz = mw*mh, s = sz;
  if( frm==Format_DXT1 ){
    compressType = squish::kDxt1;
    sz /= 2;
    s *= 4;
    } else {
    if( frm==Format_DXT5 )
      compressType = squish::kDxt5; else
      compressType = squish::kDxt3;
    s *= 4;
    }

  std::vector<unsigned char> ddsv = data.value()->bytes;
  data.value()->bytes.resize(s);

  unsigned char* px  = (&data.value()->bytes[0]);
  unsigned char* dds = (&ddsv[0]);
  bpp = 4;

  for( int i=0; i<width(); i+=4 )
    for( int r=0; r<height(); r+=4 ){
      int pos = ((i/4) + (r/4)*width()/4)*8;
      squish::Decompress( (squish::u8*)pixels,
                          &dds[pos], compressType );

      for( int x=0; x<4; ++x )
        for( int y=0; y<4; ++y ){
          unsigned char * v = &px[ (i+x + (mh-(r+y)-1)*mw)*bpp ];
          std::copy( pixels[y][x], pixels[y][x]+4, v);
          }
      }

  rawPtr  = &data.value()->bytes[0];
  mrawPtr = &data.value()->bytes[0];
  frm = Format_RGBA;
  }

void Pixmap::toS3tc( Format /*f*/ ) {
  squish::u8 px[4][4][4];
  unsigned char* p  = (&data.value()->bytes[0]);

  std::vector<squish::u8> d;
  d.resize( mw*mh/2 );

  for( int i=0; i<mw; i+=4 )
    for( int r=0; r<mh; r+=4 ){
      for( int y=0; y<4; ++y )
        for( int x=0; x<4; ++x ){
          std::copy( p+((x + i + (y+r)*mw)*bpp),
                     p+((x + i + (y+r)*mw)*bpp)+bpp,
                     px[y][x] );
          }

      int pos = ((i/4) + (r/4)*mw/4)*8;
      squish::Compress( (squish::u8*)px,
                        &d[pos], squish::kDxt1 );
      }

  data.value()->bytes = d;
  frm = Format_DXT1;
  }

void Pixmap::toETC() {

  }

template< class T >
bool Pixmap::implSave( T f ) {
  if( data.const_value()==0 )
    return false;

  Tempest::Detail::Atomic::begin();
  bool ok = SystemAPI::saveImage( f,
                                  mw, mh, bpp,
                                  data.const_value()->bytes );
  Tempest::Detail::Atomic::end();
  return ok;
  }

void Pixmap::fill(const Pixmap::Pixel &p) {
  if( mw==0 || mh==0 )
    return;

  verifyFormatEditable();

  size_t sz = data.value()->bytes.size();
  mrawPtr   = &data.value()->bytes[0];
  rawPtr    = mrawPtr;

  unsigned char px[] = {p.r, p.g, p.b, p.a};

  if( bpp==3 ){
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

  for( int i=x; i<xm; ++i, ++px ){
    int lpy = py;
    for( int r=y; r<ym; ++r, ++lpy ){
      out.set(i,r, p.at(px,lpy) );
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

  for( int i=x; i<xm; ++i, ++px ){
    int lpy = py;
    for( int r=y; r<ym; ++r, ++lpy ){
      out.set(i,r, p.at(px,lpy) );
      }
    }
  }


