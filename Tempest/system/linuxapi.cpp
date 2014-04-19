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

LinuxAPI::LinuxAPI() {
  /*
  TranslateKeyPair k[] = {
    { VK_LEFT,   Event::K_Left   },
    { VK_RIGHT,  Event::K_Right  },
    { VK_UP,     Event::K_Up     },
    { VK_DOWN,   Event::K_Down   },

    { VK_ESCAPE, Event::K_ESCAPE },
    { VK_BACK,   Event::K_Back   },
    { VK_DELETE, Event::K_Delete },
    { VK_INSERT, Event::K_Insert },
    { VK_HOME,   Event::K_Home   },
    { VK_END,    Event::K_End    },
    { VK_PAUSE,  Event::K_Pause  },
    { VK_RETURN, Event::K_Return },

    { VK_F1,     Event::K_F1 },
    { 0x30,      Event::K_0  },
    { 0x41,      Event::K_A  },

    { 0,         Event::K_NoKey }
    };

  setupKeyTranslate(k);*/
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
  HWND * win = new HWND;
  GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
  XVisualInfo * vi = glXChooseVisual(dpy, 0, att);
  XSetWindowAttributes    swa;

  Colormap                cmap;
  cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);

  swa.colormap   = cmap;
  swa.event_mask = event_mask;

  *win = XCreateWindow( dpy, root, 0, 0, w, h,
                        0, vi->depth, InputOutput, vi->visual,
                        CWColormap | CWEventMask, &swa );
  XSetWMProtocols(dpy, *win, &wmDeleteMessage(), 1);

  XMapWindow(dpy, *win);
  XStoreName(dpy, *win, "Tempest Application");

  return (Window*)win;
  }

SystemAPI::Window *LinuxAPI::createWindowMaximized() {
  return 0;
  }

SystemAPI::Window *LinuxAPI::createWindowMinimized() {
  return 0;
  }

SystemAPI::Window *LinuxAPI::createWindowFullScr() {
  return 0;
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
  XDestroyWindow(dpy, *((::Window*)w));
  //DestroyWindow( (HWND)w );
  wndWx.erase( pin(w) );
  }

void LinuxAPI::show(Window *hWnd) {
  /*
  Tempest::Window* w = 0;
  std::unordered_map<LinuxAPI::Window*, Tempest::Window*>::iterator i
      = wndWx.find( (LinuxAPI::Window*)hWnd );

  if( i!= wndWx.end() )
    w = i->second;

  if( !w )
    return;

  if( w->showMode()==Tempest::Window::FullScreen )
    return;

  HWND hwnd = (HWND)hWnd;

  switch( w->showMode() ){
    case Tempest::Window::Normal:
    case Tempest::Window::FullScreen:
      ShowWindow( hwnd, SW_NORMAL );
      break;

    case Tempest::Window::Minimized:
      ShowWindow( hwnd, SW_MINIMIZE );
      break;

    default:
      ShowWindow( hwnd, SW_MAXIMIZE );
      break;
    }

  UpdateWindow( hwnd );
  */
  }

void LinuxAPI::setGeometry( Window *hw, int x, int y, int w, int h ) {
  /*
  DWORD wflags = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
  RECT r = {0,0,w,h};
  AdjustWindowRect(&r, wflags, false);

  LONG lStyles = GetWindowLong( (HWND)hw, GWL_STYLE );

  if( lStyles & WS_MINIMIZE )
    return;

  if( lStyles & WS_MAXIMIZE )
    return;
*/
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

    std::cout <<"xev = " << xev.type << std::endl;
    switch( xev.type ) {
      case Expose:
        render( w );
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
        if( xev.type==ButtonPress && XPending(dpy) ){
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
                        xev.type==ButtonPress ? Event::MouseDown:Event::MouseUp );
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
      case WM_MOVING:
        if( w ){
          RECT rpos = {0,0,0,0};
          GetWindowRect( hWnd, &rpos );
          SystemAPI::moveEvent( w, rpos.left, rpos.top );
          }
        break;

      case WM_SIZE:{
          RECT rpos = {0,0,0,0};
          GetWindowRect( hWnd, &rpos );

          RECT rectWindow;
          GetClientRect( HWND(hWnd), &rectWindow);
          int cW = rectWindow.right  - rectWindow.left;
          int cH = rectWindow.bottom - rectWindow.top;

          if( w ){
            int smode = int( w->showMode() );

            if( wParam==SIZE_RESTORED )
              smode = Window::Normal;

            if( wParam==SIZE_MAXIMIZED ||
                wParam==SIZE_MAXSHOW   )
              smode = Window::Maximized;

            if( wParam==SIZE_MINIMIZED )
              smode = Window::Minimized;

            SystemAPI::setShowMode( w, smode);

            if( wParam!=SIZE_MINIMIZED ){
              SystemAPI::sizeEvent( w, cW, cH );
              //GetWindowRect( HWND(hWnd), &rectWindow );
              SystemAPI::moveEvent( w, rpos.left, rpos.top );
              }
            }
          }
        break;

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

      /*
      case WM_DESTROY: {
        PostQuitMessage(0);
        }
        break;*/

      default: break;
      }

    return;
    }

#endif
