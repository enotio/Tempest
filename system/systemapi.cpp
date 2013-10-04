#include "systemapi.h"

#include "system/windowsapi.h"
#include "system/androidapi.h"
#include "ddsdef.h"

#include <Tempest/Window>
#include <cstring>
#include <iostream>
#include <locale>

#include <libpng/png.h>

using namespace Tempest;

SystemAPI &SystemAPI::instance() {
#ifdef __WIN32__
  static WindowsAPI api;
#endif

#ifdef __ANDROID__
  static AndroidAPI api;
#endif

  return api;
  }

Size SystemAPI::screenSize() {
  return instance().implScreenSize();
  }

std::string SystemAPI::loadText(const std::string &file) {
  return instance().loadText( file.data() );
  }

std::string SystemAPI::loadText(const std::wstring &file) {
  return instance().loadText( file.data() );
  }

std::string SystemAPI::loadText(const char *file) {
  return instance().loadTextImpl(file);
  }

std::string SystemAPI::loadText(const wchar_t *file) {
  return instance().loadTextImpl(file);
  }

std::vector<char> SystemAPI::loadBytes(const char *file) {
  return instance().loadBytesImpl(file);
  }

std::vector<char> SystemAPI::loadBytes(const wchar_t *file) {
  return instance().loadBytesImpl(file);
  }

bool SystemAPI::writeBytes(const wchar_t *file, const std::vector<char> &f) {
  return instance().writeBytesImpl(file, f);
  }

bool SystemAPI::loadImage( const wchar_t *file,
                           int &w, int &h,
                           int &bpp,
                           std::vector<unsigned char> &out) {
  return instance().loadImageImpl( file, w, h, bpp, out );
  }

bool SystemAPI::loadImage( const char *file,
                           int &w, int &h,
                           int &bpp,
                           std::vector<unsigned char> &out ) {
  return instance().loadImageImpl( file, w, h, bpp, out );
  }

bool SystemAPI::saveImage( const wchar_t *file,
                           int &w, int &h,
                           int &bpp,
                           std::vector<unsigned char> &in ) {
  return instance().saveImageImpl( file, w, h, bpp, in );
  }

bool SystemAPI::saveImage( const char *file,
                           int &w, int &h,
                           int &bpp,
                           std::vector<unsigned char> &in ) {
  return instance().saveImageImpl( file, w, h, bpp, in );
  }

void SystemAPI::mkKeyEvent( Tempest::Window *w,
                            KeyEvent& e,
                            Event::Type type ){
  if( type==Event::KeyDown ){
    processEvents(w, e, type);
    }
  if( type==Event::KeyUp ){
    processEvents(w, e, type);
    }
  if( type==Event::Shortcut ){
    processEvents(w, e, type);
    }
  }

void SystemAPI::mkMouseEvent(Tempest::Window *w, MouseEvent &e , Event::Type type ){
  if( w->pressedC.size() < size_t(e.mouseID+1) )
    w->pressedC.resize(e.mouseID+1);

  if( type==Event::MouseDown ){
    w->pressedC[e.mouseID] = 1;
    processEvents(w, e, type);
    }

  if( type==Event::MouseUp ){
    w->pressedC[e.mouseID] = 0;
    processEvents(w, e, type);
    }

  if( type==Event::MouseMove ){
    if( w->pressedC[e.mouseID] ){
      processEvents(w, e, Event::MouseDrag);

      if( e.isAccepted() )
        return;
      }

    processEvents(w, e, type);
    }

  if( type==Event::MouseWheel ){
    processEvents(w, e, type);
    }
  }

void SystemAPI::processEvents(Widget *w, MouseEvent &e, Event::Type type) {
  e.setType( type );

  if( type==Event::MouseDown )
    return w->rootMouseDownEvent(e);

  if( type==Event::MouseUp )
    return w->rootMouseUpEvent(e);

  if( type==Event::MouseMove )
    return w->rootMouseMoveEvent(e);

  if( type==Event::MouseWheel )
    return w->rootMouseWheelEvent(e);

  if( type==Event::MouseDrag)
    return w->rootMouseDragEvent(e);
  }

void SystemAPI::processEvents(Widget *w, KeyEvent &e, Event::Type type) {
  e.setType( type );

  if( type==Event::KeyDown )
    return w->rootKeyDownEvent(e);

  if( type==Event::KeyUp )
    return w->rootKeyUpEvent(e);

  if( type==Event::Shortcut )
    return w->rootShortcutEvent(e);
  }

void SystemAPI::sizeEvent( Tempest::Window *w, int cW, int cH ) {
  if( w->winW==cW &&
      w->winH==cH )
    return;

  w->winW = cW;
  w->winH = cH;

  if( w->isAppActive ){
    if( cW * cH ){
      w->resize( cW, cH );
      w->resizeIntent = false;
      }
    } else {
    w->resizeIntent = true;
    }
  }

void SystemAPI::activateEvent(Tempest::Window *w, bool a) {
  w->isAppActive = a;
  }

bool SystemAPI::isGraphicsContextAviable( Tempest::Window *) {
  return true;
  }

std::string SystemAPI::toUtf8(const std::wstring &str) {
  std::string r;
  r.assign( str.begin(), str.end() );

  return r;
  }

std::wstring SystemAPI::toWstring(const std::string &str) {
  std::wstring r;
  r.assign( str.begin(), str.end() );

  return r;
  }

const std::string &SystemAPI::androidActivityClass() {
  return instance().androidActivityClassImpl();
  }

Event::KeyType SystemAPI::translateKey(uint64_t scancode) {
  KeyInf &k = instance().ki;

  for( size_t i=0; i<k.keys.size(); ++i )
    if( k.keys[i].src==scancode )
      return k.keys[i].result;

  for( size_t i=0; i<k.k0.size(); ++i )
    if( k.k0[i].src <= scancode &&
                       scancode <=k.k0[i].src+9 ){
      auto dx = ( scancode-k.k0[i].src );
      return Event::KeyType( k.k0[i].result + dx );
      }

  auto literalsCount = (Event::K_Z - Event::K_A);
  for( size_t i=0; i<k.a.size(); ++i )
    if( k.a[i].src <= scancode &&
                      scancode <= k.a[i].src+literalsCount ){
      auto dx = ( scancode-k.a[i].src );
      return Event::KeyType( k.a[i].result + dx );
      }

  for( size_t i=0; i<k.f1.size(); ++i )
    if( k.f1[i].src <= scancode &&
                       scancode <= k.f1[i].src+k.fkeysCount ){
      auto dx = ( scancode-k.f1[i].src );
      return Event::KeyType( k.f1[i].result + dx );
      }

  return Event::K_NoKey;
  }

bool SystemAPI::writeBytesImpl(const wchar_t *file, const std::vector<char> &f) {
  return 0;
  }

bool SystemAPI::loadImageImpl( const char *file,
                               int &w, int &h, int &bpp,
                               std::vector<unsigned char> &out ) {
  std::wstring wstr;
  size_t s = 0;

  for( s = 0; file[s]; ++s )
    ;

  wstr.assign(file, file+s);

  return loadImageImpl(wstr.data(), w,h, bpp, out);
  }

bool SystemAPI::saveImageImpl( const char *file,
                               int &w, int &h, int &bpp,
                               std::vector<unsigned char> &out) {
  return saveImageImpl( toWstring(file).data(), w, h, bpp, out );
  }

bool SystemAPI::loadS3TCImpl( const std::vector<char> &img,
                              int &w,
                              int &h,
                              int &bpp,
                              std::vector<unsigned char> &out ) {
  DDSURFACEDESC2 ddsd;
  //std::vector<char> img = loadBytesImpl(data);
  const char* pos = &img[0];
  if( img.size()<4+sizeof(ddsd) )
    return false;

  if( strncmp( pos, "DDS ", 4 ) != 0 ) {
    return false;
    }

  //std::cout << sizeof(DWORD) << std::endl;
  pos+=4;
  memcpy(&ddsd, pos, sizeof(ddsd) );
  pos += sizeof(ddsd);

  int factor;
  switch( ddsd.ddpfPixelFormat.dwFourCC ) {
    case FOURCC_DXT1:
      bpp    = -1;
      factor = 2;
      break;

    case FOURCC_DXT3:
      bpp    = -3;
      factor = 4;
      break;

    case FOURCC_DXT5:
      bpp    = -5;
      factor = 4;
      break;

    default:
      return false;
    }

  if( ddsd.dwLinearSize == 0 ) {
    //return false;
    }

  w = ddsd.dwWidth;
  h = ddsd.dwHeight;
  int bufferSize = w*h/factor;

  if( ddsd.dwMipMapCount > 1 ){
    int mipW = w, mipH = h;
    for( size_t i=0; i<ddsd.dwMipMapCount; ++i ){
      if(mipW>1)
        mipW/=2;
      if(mipH>1)
        mipH/=2;

      bufferSize += mipW*mipH/factor;
      }
    }
  //if( ddsd.dwMipMapCount > 1 )
  //  bufferSize = ddsd.dwLinearSize * factor; else
    //bufferSize = ddsd.dwLinearSize;

  size_t sz = 4+sizeof(ddsd)+bufferSize;
  if( img.size() < sz )
    return false;

  out.reserve( bufferSize );
  out.resize ( bufferSize );
  memcpy( &out[0], pos, bufferSize );

  return true;
  }

bool SystemAPI::loadPngImpl( const std::vector<char> &d,
                             int &w, int &h,
                             int &bpp,
                             std::vector<unsigned char> &out ) {
  if( d.size()<8 )// 8 is the maximum size that can be checked
    return false;

  unsigned char* data = (unsigned char*)&d[0];
  if (png_sig_cmp( data, 0, 8))
    return false;
  data += 8;

  png_structp png_ptr;

  /* initialize stuff */
  png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

  if (!png_ptr)
    return 0;

  png_infop info_ptr = png_create_info_struct(png_ptr);
  if (!info_ptr)
    return 0;

  if( setjmp(png_jmpbuf(png_ptr)) )
    return 0;

  struct Reader{
    bool err;
    unsigned char* data;
    size_t sz;

    static void read( png_structp png_ptr,
                      png_bytep   outBytes,
                      png_size_t  byteCountToRead ){
      if( !png_ptr->io_ptr ){
        return;
        }

      Reader& r = *(Reader*)png_ptr->io_ptr;

      if( r.sz<byteCountToRead ){
        r.err = true;
        return;
        }

      r.sz -= byteCountToRead;

      for(; byteCountToRead>0; --byteCountToRead ){
        *outBytes = *r.data;
        ++r.data;
        ++outBytes;
        }
      }
    };

  Reader r;
  r.err  = false;
  r.data = data;
  r.sz   = d.size()-8;
  //png_init_io(png_ptr, data);
  png_set_read_fn( png_ptr, &r, &Reader::read );

  png_set_sig_bytes(png_ptr, 8);

  png_read_info(png_ptr, info_ptr);

  w = png_get_image_width (png_ptr, info_ptr);
  h = png_get_image_height(png_ptr, info_ptr);

  png_byte color_type = png_get_color_type(png_ptr, info_ptr);
  png_byte bit_depth  = png_get_bit_depth(png_ptr, info_ptr);

  if( color_type == PNG_COLOR_TYPE_PALETTE )
    png_set_palette_to_rgb(png_ptr);

  color_type = png_get_color_type(png_ptr, info_ptr);

  if( color_type==PNG_COLOR_TYPE_RGB )
    bpp=3;

  if( color_type==PNG_COLOR_TYPE_RGB_ALPHA )
    bpp=4;

  if( ( color_type==PNG_COLOR_TYPE_RGB ||
        color_type==PNG_COLOR_TYPE_RGB_ALPHA ) &&
      bit_depth==8 ){
    out.resize( w*h*bpp );
    //int number_of_passes =
        png_set_interlace_handling(png_ptr);
    png_read_update_info(png_ptr, info_ptr);

    if (setjmp(png_jmpbuf(png_ptr)))
      return false;

    png_bytep * row_pointers = new png_bytep[h];//(png_bytep*) malloc(sizeof(png_bytep) * h);
    //int rowBytes = png_get_rowbytes(png_ptr,info_ptr);
    for( int y=0; y<h; y++ )
      row_pointers[y] = &out[ y*w*bpp ];//(png_byte*) malloc(rowBytes);

    if( bpp==3 || bpp==4 )
      png_read_image(png_ptr, row_pointers); else
      r.err = true;

    delete (row_pointers);

    return !r.err;
    }

  return false;
  }

const std::string &SystemAPI::androidActivityClassImpl() {
  static const std::string cls = "com/tempest/engine/TempestActivity";
  return cls;
  }

void SystemAPI::setupKeyTranslate( const SystemAPI::TranslateKeyPair k[] ) {
  ki.keys.clear();
  ki.a. clear();
  ki.k0.clear();
  ki.f1.clear();

  for( size_t i=0; k[i].result!=Event::K_NoKey; ++i ){
    if( k[i].result==Event::K_A )
      ki.a.push_back(k[i]); else
    if( k[i].result==Event::K_0 )
      ki.k0.push_back(k[i]); else
    if( k[i].result==Event::K_F1 )
      ki.f1.push_back(k[i]); else
      ki.keys.push_back( k[i] );
    }

#ifndef __ANDROID__
  ki.keys.shrink_to_fit();
  ki.a. shrink_to_fit();
  ki.k0.shrink_to_fit();
  ki.f1.shrink_to_fit();
#endif
  }

void SystemAPI::setFuncKeysCount(int c) {
  ki.fkeysCount = c;
  }
