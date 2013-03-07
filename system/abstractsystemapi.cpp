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
