#include "pixmap.h"

#include <Tempest/SystemAPI>
#include <iostream>
#include "core/wrappers/atomic.h"

#include <squish/squish.h>
#include <cassert>
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
  }

Tempest::Pixmap::Pixmap(const std::string &p) {
  mw = 0;
  mh = 0;

  bpp = 3;
  frm = Format_RGB;

  load(p);

  assert( sizeof(Pixel)==4 );
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

bool Pixmap::load( const std::string &f ) {
  int w = 0, h = 0, bpp = 0;

  Data * image = new Data();
  bool ok = SystemAPI::loadImage( f.data(), w, h, bpp, image->bytes );

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

  rawPtr = &data.const_value()->bytes[0];
  mrawPtr = 0;
  }

void Pixmap::makeEditable() {
  //squish::u8 pixels[16*4];  // 16 pixels of input
  squish::u8 block[8];      // 8 bytes of output

  size_t sz = mw*mh, s = sz;
  if( frm==Format_DXT1 ){
    sz /= 2;
    s *= 4;
    } else {
    s *= 4;
    }

  data.value()->bytes.resize(s);
  squish::u8* px = (squish::u8*)(&data.value()->bytes[0]);

  for( size_t i=sz-8, w = s-16*4; i>0; i-=8, w-=16*4 ){
    memcpy( px+i, block, 8 );
    squish::Decompress( px+w, block, squish::kDxt1 );
    }

  rawPtr  = &data.value()->bytes[0];
  mrawPtr = &data.value()->bytes[0];
  frm = Format_RGBA;
  bpp = 4;

  save("./dbg.png");
  //for( size_t i=mrawPtr+; i)
  }

bool Pixmap::save( const std::string &f ) {
  if( data.const_value()==0 )
    return false;

  Tempest::Detail::Atomic::begin();
  bool ok = SystemAPI::saveImage( f.data(),
                                          mw, mh, bpp,
                                          data.const_value()->bytes );
  Tempest::Detail::Atomic::end();
  return ok;
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
