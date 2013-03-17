#include "abstractsystemapi.h"

#include "system/windowsapi.h"
#include "system/androidapi.h"

#include <Tempest/Window>

using namespace Tempest;

AbstractSystemAPI &AbstractSystemAPI::instance() {
#ifdef __WIN32__
  static WindowsAPI api;
#endif

#ifdef __ANDROID__
  static AndroidAPI api;
#endif

  return api;
  }

std::string AbstractSystemAPI::loadText(const char *file) {
  return instance().loadTextImpl(file);
  }

bool AbstractSystemAPI::loadImage(const char *file,
                                   int &w, int &h,
                                   int &bpp,
                                   std::vector<unsigned char> &out ) {
  return instance().loadImageImpl( file, w, h, bpp, out );
  }

bool AbstractSystemAPI::saveImage( const char *file,
                                   int &w, int &h,
                                   int &bpp,
                                   std::vector<unsigned char> &in ) {
  return instance().saveImageImpl( file, w, h, bpp, in );
  }

void AbstractSystemAPI::mkMouseEvent( Tempest::Window *w, MouseEvent &e , int type ) {
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
    if( w->pressedC )
      w->mouseDragEvent(e);

    if( e.isAccepted() )
      return;

    w->mouseUpEvent(e);
    }
  //if( w-> )
  }
