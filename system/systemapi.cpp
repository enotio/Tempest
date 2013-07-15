#include "systemapi.h"

#include "system/windowsapi.h"
#include "system/androidapi.h"
#include "ddsdef.h"

#include <Tempest/Window>
#include <cstring>
#include <iostream>

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

void SystemAPI::mkMouseEvent( Tempest::Window *w, MouseEvent &e , int type ) {
  if( type==0 ){
    //++w->pressedC;
    w->pressedC = 1;
    w->mouseDownEvent(e);
    }

  if( type==1 ){
    //--w->pressedC;
    w->pressedC = 0;
    w->mouseUpEvent(e);
    }

  if( type==2 ){
    //--w->pressedC;
    if( w->pressedC ){
      w->mouseDragEvent(e);

      if( e.isAccepted() )
        return;
      }

    w->mouseMoveEvent(e);
    }
  //if( w-> )
  }

void SystemAPI::sizeEvent( Tempest::Window *w,
                                   int /*winW*/, int /*winH*/,
                                   int   cW, int   cH ) {
  if( w->winW==cW &&
      w->winH==cH )
    return;

  w->winW = cW;
  w->winH = cH;

  if( w->isAppActive ){
    if( cW * cH ){
      w->resize( cW, cH );

      //SizeEvent e( cW, cH, winW, winH );
      //w->resizeEvent( e );
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

