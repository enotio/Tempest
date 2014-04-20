#include "linuxapi.h"

#ifdef __linux__
#include <X11/X.h>
#include <X11/Xlib.h>
#include <GL/glx.h>

#undef Always // in X11/X.h
#undef PSize

#include <Tempest/Window>
#include <Tempest/Event>
#include <Tempest/DisplaySettings>

#include <unordered_map>
#include <fstream>
#include <Tempest/Assert>

#include <iostream>
#include "core/wrappers/atomic.h"

typedef Window HWND;

using namespace Tempest;

static std::unordered_map<HWND, Tempest::Window*> wndWx;

static Display*  dpy  = 0;
static HWND      root = 0;

static const long event_mask = 0xFFFFFF;

static HWND pin( LinuxAPI::Window* w ){
  return *((HWND*)w);
  }

static void xProc(XEvent& xev, HWND &hWnd, bool &quit);

static Atom& wmDeleteMessage(){
  static Atom w  = XInternAtom( dpy, "WM_DELETE_WINDOW", 0);
  return w;
  }

static LinuxAPI::Window* X11_CreateWindow( int w, int h,
                                           Tempest::Window::ShowMode m ){
  HWND * win = new HWND;
  GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
  XVisualInfo * vi = glXChooseVisual(dpy, 0, att);
  XSetWindowAttributes    swa;

  Colormap cmap;
  cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);

  swa.colormap   = cmap;
  swa.event_mask = PointerMotionMask | ExposureMask |
                   ButtonPressMask |ButtonReleaseMask|
                   KeyPressMask | KeyReleaseMask;

  *win = XCreateWindow( dpy, root, 0, 0, w, h,
                        0, vi->depth, InputOutput, vi->visual,
                        CWColormap | CWEventMask, &swa );
  XSetWMProtocols(dpy, *win, &wmDeleteMessage(), 1);

  //XSelectInput(dpy, *win, );
  XStoreName(dpy, *win, "Tempest Application");

  XFreeColormap( dpy, cmap );
  XFree(vi);
  return (LinuxAPI::Window*)win;
  }

LinuxAPI::LinuxAPI() {
  TranslateKeyPair k[] = {
    { XK_KP_Left,   Event::K_Left   },
    { XK_KP_Right,  Event::K_Right  },
    { XK_KP_Up,     Event::K_Up     },
    { XK_KP_Down,   Event::K_Down   },

    { XK_Escape, Event::K_ESCAPE },
    { XK_BackSpace,   Event::K_Back   },
    { XK_Delete, Event::K_Delete },
    { XK_Insert, Event::K_Insert },
    { XK_Home,   Event::K_Home   },
    { XK_End,    Event::K_End    },
    { XK_Pause,  Event::K_Pause  },
    { XK_Return, Event::K_Return },

    { XK_F1,     Event::K_F1 },
    { 0x30,      Event::K_0  },
    { 0x41,      Event::K_A  },

    { 0,         Event::K_NoKey }
    };

  setupKeyTranslate(k);
  setFuncKeysCount(24);
  }

LinuxAPI::~LinuxAPI() {
  }

void *LinuxAPI::display() {
  return dpy;
  }

bool LinuxAPI::testDisplaySettings( const DisplaySettings & s ) {
  return 1;
  }

bool LinuxAPI::setDisplaySettings( const DisplaySettings &s ) {
  return 0;
  }

Size LinuxAPI::implScreenSize() {
  Screen * scr = XDefaultScreenOfDisplay(dpy);
  int w = XWidthOfScreen (scr);
  int h = XHeightOfScreen(scr);

  return Size(w,h);
  }

void LinuxAPI::startApplication(ApplicationInitArgs *) {
  dpy = XOpenDisplay(NULL);

  if(dpy == NULL) {
    T_ASSERT_X( dpy, "cannot connect to X server!");
    exit(0);
    }

  root = DefaultRootWindow(dpy);
  }

void LinuxAPI::endApplication() {
  XCloseDisplay(dpy);
  }

static void render( Tempest::Window* w ){
  if( w->showMode()!=Tempest::Window::Minimized && w->isActive() )
    w->render();
  }

int LinuxAPI::nextEvent(bool &quit) {
  XEvent xev;
  memset(&xev,0,sizeof(xev));

  if( !XPending(dpy) ){
    XNextEvent(dpy, &xev);
    for( auto i=wndWx.begin(); i!=wndWx.end(); ++i )
      render( i->second );
    return 0;
    }

  xProc( xev, xev.xclient.window, quit);
  //Sleep(0);
  return 0;
  }

int LinuxAPI::nextEvents(bool &quit) {
  XEvent xev;
  memset(&xev,0,sizeof(xev));

  while( !quit ){
    if( XPending( dpy ) ){
      XNextEvent(dpy, &xev);
      xProc( xev, xev.xclient.window, quit );
      } else {
      for( auto i=wndWx.begin(); i!=wndWx.end(); ++i )
        render( i->second );
      return 0;
      }
    }

  return 0;
  }

LinuxAPI::Window *LinuxAPI::createWindow(int w, int h) {
  return X11_CreateWindow(w, h, Tempest::Window::Normal );
  }

SystemAPI::Window *LinuxAPI::createWindowMaximized() {
  return X11_CreateWindow( 800, 600, Tempest::Window::Maximized );
  }

SystemAPI::Window *LinuxAPI::createWindowMinimized() {
  return X11_CreateWindow( 800, 600, Tempest::Window::Minimized );
  }

SystemAPI::Window *LinuxAPI::createWindowFullScr() {
  return X11_CreateWindow( 800, 600, Tempest::Window::FullScreen );
  }

Widget* LinuxAPI::addOverlay(WindowOverlay *ov) {
  if( wndWx.empty() ){
    delete ov;
    return 0;
    }

  Tempest::Window* w = wndWx.begin()->second;
  SystemAPI::addOverlay(w, ov);
  return ov;
  }

Point LinuxAPI::windowClientPos( SystemAPI::Window * hWnd ) {
  XWindowAttributes xwa;
  XGetWindowAttributes(dpy, pin(hWnd), &xwa);

  return Point( xwa.x, xwa.y );
  }

Size LinuxAPI::windowClientRect( SystemAPI::Window * hWnd ) {
  XWindowAttributes xwa;
  XGetWindowAttributes(dpy, pin(hWnd), &xwa);

  return Size( xwa.width, xwa.height );
  }

void LinuxAPI::deleteWindow( Window *w ) {
  XDestroyWindow(dpy, pin(w));
  wndWx.erase( pin(w) );
  delete (HWND*)w;
  }

void LinuxAPI::show(Window *hWnd) {
  XMapWindow(dpy, pin(hWnd) );
  }

void LinuxAPI::setGeometry( Window *hw, int x, int y, int w, int h ) {
  XMoveResizeWindow( dpy, *((::Window*)hw), x, y, w, h );
  }

void LinuxAPI::bind( Window *w, Tempest::Window *wx ) {
  wndWx[ pin(w) ] = wx;
  }

LinuxAPI::CpuInfo LinuxAPI::cpuInfoImpl(){
  CpuInfo info;
  memset(&info, 0, sizeof(info));

  info.cpuCount = 1;
  return info;
  }

LinuxAPI::File *LinuxAPI::fopenImpl( const char *fname, const char *mode ) {
  return SystemAPI::fopenImpl( fname, mode );
  }

LinuxAPI::File *LinuxAPI::fopenImpl( const char16_t *fname, const char *mode ) {
  return SystemAPI::fopenImpl( toUtf8(fname).data(), mode );
  }

static Event::MouseButton toButton( XButtonEvent& msg ){
  if( msg.button==Button1 )
    return Event::ButtonLeft;

  if( msg.button==Button3 )
    return Event::ButtonRight;

  if( msg.button==Button2 )
    return Event::ButtonMid;

  return Event::ButtonNone;
  }

static Tempest::KeyEvent makeKeyEvent( XKeyEvent& k,
                                       bool scut = false ){
  Tempest::KeyEvent::KeyType e = SystemAPI::translateKey(k.keycode);

  if( !scut ){
    if( Event::K_0<=e && e<= Event::K_9 )
      e = Tempest::KeyEvent::K_NoKey;

    if( Event::K_A<=e && e<= Event::K_Z )
      e = Tempest::KeyEvent::K_NoKey;
    }

  return Tempest::KeyEvent( e );
  }

void xProc( XEvent& xev, HWND &hWnd, bool &quit ) {
    Tempest::Window* w = 0;
    std::unordered_map<HWND, Tempest::Window*>::iterator i
        = wndWx.find( hWnd );

    if( i!= wndWx.end() )
      w = i->second;

    if( !w )
      return;

    {
      HWND root;
      int x, y;
      unsigned ww, hh, border, depth;

      if( XGetGeometry(dpy, hWnd, &root, &x, &y, &ww, &hh, &border, &depth) ){
        SystemAPI::moveEvent( w, x, y );
        SystemAPI::sizeEvent( w, ww, hh );
        }
    }

    std::cout <<"xev = " << xev.type << std::endl;
    switch( xev.type ) {
      case Expose:
        if ( xev.xexpose.count == 0){
          render( w );
          }
        break;

    /*
      case WM_CHAR:
      {
         Tempest::KeyEvent e = Tempest::KeyEvent( uint32_t(wParam) );

         DWORD wrd[3] = {
           VK_RETURN,
           VK_BACK,
           0
           };

         if( 0 == *std::find( wrd, wrd+2, wParam) ){
           Tempest::KeyEvent ed( e.key, e.u16, Event::KeyDown );
           SystemAPI::emitEvent(w, ed);

           Tempest::KeyEvent eu( e.key, e.u16, Event::KeyUp );
           SystemAPI::emitEvent(w, eu);
           }
      }
      break;*/

      case KeyPressMask:
      {
         SystemAPI::emitEvent( w,
                               makeKeyEvent( xev.xkey ),
                               makeKeyEvent( xev.xkey, true),
                               Event::KeyDown );
      }
      break;

      case KeyRelease:
      {
         SystemAPI::emitEvent( w,
                               makeKeyEvent( xev.xkey ),
                               makeKeyEvent( xev.xkey, true),
                               Event::KeyUp );
      }
      break;


      case ButtonPress:
      case ButtonRelease: {
        bool isWheel = false;
        if( xev.type==ButtonPress && XPending(dpy) &&
            (xev.xbutton.button == Button4 || xev.xbutton.button == Button5) ){
          XEvent ev;
          XPeekEvent(dpy, &ev);
          isWheel = (ev.type==ButtonRelease);
          }

        if( isWheel ){
          int ticks = 0;
          if( xev.xbutton.button == Button4 ) {
            ticks = 1;
            }
          else if ( xev.xbutton.button == Button5 ) {
            ticks = -1;
            }
          Tempest::MouseEvent e( xev.xbutton.x,
                                 xev.xbutton.y,
                                 Tempest::Event::ButtonNone,
                                 ticks,
                                 0,
                                 Event::MouseWheel );
          SystemAPI::emitEvent(w, e);
          } else {
          MouseEvent e( xev.xbutton.x,
                        xev.xbutton.y,
                        toButton( xev.xbutton ),
                        0,
                        0,
                        xev.type==ButtonPress ? Event::MouseDown : Event::MouseUp );
          SystemAPI::emitEvent(w, e);
          }
        }
        break;

      case MotionNotify: {
        MouseEvent e( xev.xmotion.x,
                      xev.xmotion.y,
                      Event::ButtonNone,
                      0,
                      0,
                      Event::MouseMove  );
        std::cout << "motion" << std::endl;
        SystemAPI::emitEvent(w, e);
        }
        break;
/*
      case WM_ACTIVATEAPP:
      {
          bool a = (wParam==TRUE);
          SystemAPI::activateEvent(w,a);

          if( !a && w->isFullScreenMode() ){
            ShowWindow( hWnd, SW_MINIMIZE );
            }
      }
      break;*/

      case ClientMessage:{
        if( xev.xclient.data.l[0] == long(wmDeleteMessage()) ){
          Tempest::CloseEvent e;
          SystemAPI::emitEvent(w, e);
          if( !e.isAccepted() )
            quit = true;
          }
        }
        break;

      case DestroyNotify:{
        quit = true;
        }
        break;

      default: break;
      }

    return;
    }

#endif
