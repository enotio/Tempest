#include "systemapi.h"

#include "system/windowsapi.h"
#include "system/androidapi.h"
#include "ddsdef.h"

#include <Tempest/Window>
#include <cstring>
#include <iostream>

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

std::string SystemAPI::loadText(const char *file) {
  return instance().loadTextImpl(file);
  }

std::vector<char> SystemAPI::loadBytes(const char *file) {
  return instance().loadBytesImpl(file);
  }

bool SystemAPI::loadImage( const char *file,
                                   int &w, int &h,
                                   int &bpp,
                                   std::vector<unsigned char> &out ) {
  return instance().loadImageImpl( file, w, h, bpp, out );
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

bool SystemAPI::loadS3TCImpl( const char *file,
                                      int &w,
                                      int &h,
                                      int &bpp,
                                      std::vector<unsigned char> &out ) {
  DDSURFACEDESC2 ddsd;
  std::vector<char> img = loadBytesImpl(file);
  char* pos = &img[0];
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

