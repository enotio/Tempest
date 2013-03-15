#include "pixmap.h"

#ifndef __ANDROID__
#include <IL/il.h>
#endif

#ifdef __ANDROID__
//#include <SDL.h>
#endif

#include <iostream>

#include "core/wrappers/atomic.h"

using namespace Tempest;


Pixmap::DbgManip::Ref * Pixmap::DbgManip::newRef( const Ref * base ){
  return new Ref( new Data(*base->data) );
  }

void Pixmap::DbgManip::delRef( Ref * r ){
  delete r->data;
  delete r;
  }

Pixmap::Pixmap() {
  w = 0;
  h = 0;

  bpp = 3;
  rawPtr  = 0;
  mrawPtr = 0;
  }

Tempest::Pixmap::Pixmap(const std::string &p) {
  w = 0;
  h = 0;

  bpp = 3;

  load(p);
  }


Pixmap::Pixmap(int iw, int ih, bool alpha) {
  w = iw;
  h = ih;

  if( alpha )
    bpp = 4; else
    bpp = 3;

  data.value() = new Data();
  data.value()->bytes.resize(w*h*bpp);
  rawPtr = &data.const_value()->bytes[0];
  mrawPtr = 0;
  }

Pixmap::Pixmap(const Pixmap &p):data(p.data) {
  w   = p.w;
  h   = p.h;
  bpp = p.bpp;

  if( w>0 && h>0 )
    rawPtr = &data.const_value()->bytes[0]; else
    rawPtr = 0;
  mrawPtr = 0;
  }

Pixmap &Pixmap::operator =(const Pixmap &p) {
  data = p.data;

  w   = p.w;
  h   = p.h;
  bpp = p.bpp;

  if( w>0 && h>0 )
    rawPtr = &data.const_value()->bytes[0]; else
    rawPtr = 0;
  mrawPtr = 0;

  return *this;
  }

bool Pixmap::load( const std::string &f ) {
#ifdef __ANDROID__
   
#else
  initImgLib();
  bool ok = true;

  ILuint	id;
  ilGenImages ( 1, &id );
  ilBindImage ( id );

  if( ilLoadImage ( f.data() ) ){
    int format = ilGetInteger( IL_IMAGE_FORMAT );

    int size_of_pixel = 1;
    bpp = 3;

    int idx[5][4] = {
      { 0, 1, 2, 3 }, //IL_RGB, IL_RGBA
      { 0, 0, 0, 0 }, //IL_ALPHA
      { 2, 1, 0, 3 },
      { 2, 1, 0, 3 },
      { 0, 0, 0, 1 }
      };
    int *ix = idx[0];


    switch( format ){
      case IL_RGB:   size_of_pixel = 3; break;
      case IL_RGBA:  size_of_pixel = 4; bpp = 4; break;
      case IL_ALPHA: size_of_pixel = 1; ix = idx[1]; break;

      case IL_BGR:   size_of_pixel = 3; ix = idx[2]; break;
      case IL_BGRA:  size_of_pixel = 4; ix = idx[3]; break;
      case IL_LUMINANCE_ALPHA:
        size_of_pixel = 4; ix = idx[4];
        break;
      default: ok = false;
      }

    if( ok ){
      ILubyte * data = ilGetData();
      Data * image = new Data();

      w = ilGetInteger ( IL_IMAGE_WIDTH      );
      h = ilGetInteger ( IL_IMAGE_HEIGHT     );

      if( ilGetInteger ( IL_IMAGE_TYPE ) == IL_UNSIGNED_BYTE )
        initRawData<ILubyte,   1>( *image, data, size_of_pixel, ix ); else
        initRawData<ILdouble, 255>( *image, data, size_of_pixel, ix );

      this->data.value() = image;

      if( w>0 && h>0 )
        rawPtr = &this->data.const_value()->bytes[0]; else
        rawPtr = 0;
      mrawPtr = 0;
      }
    } else {
    ok = false;
    //ilGetError();
    }

  ilDeleteImages( 1, &id );

  return ok;
#endif
  }
/*
int Pixmap::width() const {
  return w;
  }

int Pixmap::height() const {
  return h;
  }

Pixmap::Pixel Pixmap::at(int x, int y) const {
  unsigned char * p = &data.const_value()->bytes[ (x + y*w)*bpp ];
  Pixel r;

  r.r = p[0];
  r.g = p[1];
  r.b = p[2];

  if( bpp==4 )
    r.a = p[3]; else
    r.a = 255;

  return r;
  }*/

bool Pixmap::hasAlpha() const {
  return bpp==4;
  }

void Pixmap::addAlpha() {
  if( bpp==4 )
    return;

  data.value()->bytes.resize(w*h*4);
  unsigned char * v = &data.value()->bytes[0];

  for( size_t i=w*h; i>0; --i ){
    unsigned char * p = &v[ 4*i-4 ];
    unsigned char * s = &v[ 3*i-3 ];
    for( int r=0; r<3; ++r )
      p[r] = s[r];
    p[3] = 255;
    }

  rawPtr = &data.const_value()->bytes[0];
  mrawPtr = 0;
  }

void Pixmap::initImgLib() {
#ifndef __ANDROID__
  Tempest::Detail::Atomic::begin();

  static bool wasInit = false;
  if( !wasInit )
    ilInit();
  ilEnable(IL_FILE_OVERWRITE);

  wasInit = true;

  Tempest::Detail::Atomic::end();
#endif
  }

template< class ChanelType, int mul >
void Pixmap::initRawData( Data & d, void * input, int pixSize, int * ix ){
  ChanelType * img = reinterpret_cast<ChanelType*>(input);

  d.bytes.resize( bpp*w*h );

  for( int i=0; i<w; ++i )
    for( int r=0; r<h; ++r ){
      ChanelType * raw = &img[ pixSize*(i+r*w) ];
      for( int q=0; q<bpp; ++q ){
        d.bytes[ bpp*(i+r*w)+q ] = ( raw[ ix[q] ]*mul );
        }

      }
  }

bool Pixmap::save( const std::string &f ) {
  if( data.const_value()==0 )
    return false;

  initImgLib();
#ifndef __ANDROID__
  Tempest::Detail::Atomic::begin();
  ILuint	id;
  ilGenImages ( 1, &id );
  ilBindImage ( id );

  ilTexImage( w, h, 1,
              bpp,
              hasAlpha() ? IL_RGBA : IL_RGB,
              IL_UNSIGNED_BYTE,
              data.const_value()->bytes.data() );

  ILubyte * img = ilGetData();

  int h2 = h/2;
  for( int i=0; i<w; ++i )
    for( int r=0; r<h2; ++r ){
      ILubyte * raw1 = &img[ bpp*(i+r*w) ];
      ILubyte * raw2 = &img[ bpp*(i+(h-r-1)*w) ];

      for( int q=0; q<bpp; ++q )
        std::swap( raw1[q], raw2[q] );
      }

  bool ok = ilSaveImage( f.data() );
  ilDeleteImages( 1, &id );
  Tempest::Detail::Atomic::end();

  return ok;
#endif
  return 0;
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
