#include "linuxapi.h"

#ifdef __linux__
#include <X11/X.h>
#include <X11/Xlib.h>

#include <Tempest/Window>
#include <Tempest/Event>
#include <Tempest/DisplaySettings>

#include <unordered_map>
#include <fstream>
#include <Tempest/Assert>

#include <iostream>
#include "core/wrappers/atomic.h"

using namespace Tempest;

static std::unordered_map<LinuxAPI::Window*, Tempest::Window*> wndWx;

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

bool LinuxAPI::testDisplaySettings( const DisplaySettings & s ) {
  return 1;
  }

bool LinuxAPI::setDisplaySettings( const DisplaySettings &s ) {
  return 0;
  }

Size LinuxAPI::implScreenSize() {
  DEVMODE mode;
  EnumDisplaySettings( 0, ENUM_CURRENT_SETTINGS, &mode );
  int w = mode.dmPelsWidth;
  int h = mode.dmPelsHeight;

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

int LinuxAPI::nextEvent(bool &quit) {
  XEvent xev;
  XNextEvent(dpy, &xev);

  if(xev.type == Expose) {
    for( auto i=wndWx.begin(); i!=wndWx.end(); ++i )
      render( i->second );

    return 0;
    } else {
    if( uMsg.message==WM_QUIT )
      quit = 1;

    TranslateMessage( &uMsg );
    DispatchMessage ( &uMsg );
    Sleep(0);
    return uMsg.wParam;
    }
  }

static void render( Tempest::Window* w ){
  if( w->showMode()!=Tempest::Window::Minimized && w->isActive() )
    w->render();
  }

int LinuxAPI::nextEvents(bool &quit) {
  MSG uMsg;
  memset(&uMsg,0,sizeof(uMsg));
  int r = 0;

  while( !quit ){
    if( PeekMessage( &uMsg, NULL, 0, 0, PM_REMOVE ) ){
      if( uMsg.message==WM_QUIT )
        quit = 1;

      TranslateMessage( &uMsg );
      DispatchMessage ( &uMsg );

      if( uMsg.message==WM_QUIT )
        r = uMsg.wParam;
      } else {
      for( auto i=wndWx.begin(); i!=wndWx.end(); ++i )
        render( i->second );

      Sleep(1);
      return r;
      }
    }

  return r;
  }

LinuxAPI::Window *LinuxAPI::createWindow(int w, int h) {
  ::Window win;
  win = XCreateWindow( dpy, root, 0, 0, w, h,
                       0, vi->depth, InputOutput, vi->visual,
                       CWColormap | CWEventMask, &swa ;
  Atom wmDeleteMessage = XInternAtom( dpy, "WM_DELETE_WINDOW", 0);
  XSetWMProtocols(dpy, win, &wmDeleteMessage, 1);

  XMapWindow(dpy, win);
  XStoreName(dpy, win, "Tempest Application");

  return (Window*)hwnd;
  }

SystemAPI::Window *LinuxAPI::createWindowMaximized() {
  int w = GetSystemMetrics(SM_CXFULLSCREEN),
      h = GetSystemMetrics(SM_CYFULLSCREEN);
  DEVMODE mode;
  EnumDisplaySettings( 0, ENUM_CURRENT_SETTINGS, &mode );
  w = mode.dmPelsWidth;
  h = mode.dmPelsHeight;

  DWORD wflags    = WS_OVERLAPPEDWINDOW | WS_POPUP | WS_VISIBLE;
  DWORD dwExStyle = WS_EX_APPWINDOW;

  wflags |= WS_CLIPSIBLINGS |	WS_CLIPCHILDREN;

  HWND hwnd = CreateWindowEx( dwExStyle,
                              L"Tempest_Window_Class",
                              L"Tempest_Window_Class",
                              wflags,
                              0, 0,
                              w, h,
                              NULL,
                              NULL,
                              GetModuleHandle(0), NULL );

  ShowWindow( hwnd, SW_MAXIMIZE );
  UpdateWindow( hwnd );
  return (Window*)hwnd;
  }

SystemAPI::Window *LinuxAPI::createWindowMinimized() {
  int w = GetSystemMetrics(SM_CXFULLSCREEN),
      h = GetSystemMetrics(SM_CYFULLSCREEN);
  DEVMODE mode;
  EnumDisplaySettings( 0, ENUM_CURRENT_SETTINGS, &mode );
  w = mode.dmPelsWidth;
  h = mode.dmPelsHeight;

  DWORD wflags = WS_OVERLAPPEDWINDOW | WS_POPUP | WS_VISIBLE;
  HWND hwnd = CreateWindowEx( 0,
                              L"Tempest_Window_Class",
                              L"Tempest_Window_Class",
                              wflags,
                              0, 0,
                              w, h,
                              NULL, NULL,
                              GetModuleHandle(0), NULL );

  return (Window*)hwnd;
  }

SystemAPI::Window *LinuxAPI::createWindowFullScr() {
  int w = GetSystemMetrics(SM_CXFULLSCREEN),
      h = GetSystemMetrics(SM_CYFULLSCREEN);
  DEVMODE mode;
  EnumDisplaySettings( 0, ENUM_CURRENT_SETTINGS, &mode );
  w = mode.dmPelsWidth;
  h = mode.dmPelsHeight;

  DWORD wflags = WS_POPUP;
  HWND hwnd = CreateWindowEx( 0,
                              L"Tempest_Window_Class",
                              L"Tempest_Window_Class",
                              wflags,
                              0, 0,
                              w, h,
                              NULL, NULL,
                              GetModuleHandle(0),
                              NULL );

  ShowWindow( hwnd, SW_NORMAL );
  UpdateWindow( hwnd );
  return (Window*)hwnd;
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
  RECT rectWindow;
  GetClientRect( HWND(hWnd), &rectWindow);
  return Point(rectWindow.left,rectWindow.top);
  }

Size LinuxAPI::windowClientRect( SystemAPI::Window * hWnd ) {
  RECT rectWindow;
  GetClientRect( HWND(hWnd), &rectWindow);
  int cW = rectWindow.right  - rectWindow.left;
  int cH = rectWindow.bottom - rectWindow.top;

  return Size(cW,cH);
  }

void LinuxAPI::deleteWindow( Window *w ) {
  DestroyWindow( (HWND)w );
  wndWx.erase(w);
  }

void LinuxAPI::show(Window *hWnd) {
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
  }

void LinuxAPI::setGeometry( Window *hw, int x, int y, int w, int h ) {
  DWORD wflags = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
  RECT r = {0,0,w,h};
  AdjustWindowRect(&r, wflags, false);

  LONG lStyles = GetWindowLong( (HWND)hw, GWL_STYLE );

  if( lStyles & WS_MINIMIZE )
    return;

  if( lStyles & WS_MAXIMIZE )
    return;

  MoveWindow( (HWND)hw,
              x,
              y,
              r.right-r.left,
              r.bottom-r.top,
              false );
  }

void LinuxAPI::bind( Window *w, Tempest::Window *wx ) {
  wndWx[w] = wx;
  }

LinuxAPI::CpuInfo LinuxAPI::cpuInfoImpl(){
  CpuInfo info;
  memset(&info, 0, sizeof(info));

  info.cpuCount = 1;
  return info;
  }

LinuxAPI::File *LinuxAPI::fopenImpl( const char *fname, const char *mode ) {
  return fopenImpl( fname, mode );
  }

LinuxAPI::File *LinuxAPI::fopenImpl( const char16_t *fname, const char *mode ) {
  return fopenImpl( toUtf8(fname).data(), mode );
  }

static Event::MouseButton toButton( UINT msg ){
  if( msg==WM_LBUTTONDOWN ||
      msg==WM_LBUTTONUP )
    return Event::ButtonLeft;

  if( msg==WM_RBUTTONDOWN  ||
      msg==WM_RBUTTONUP)
    return Event::ButtonRight;

  if( msg==WM_MBUTTONDOWN ||
      msg==WM_MBUTTONUP )
    return Event::ButtonMid;

  return Event::ButtonNone;
  }

static Tempest::KeyEvent makeKeyEvent( WPARAM k,
                                       bool scut = false ){
  Tempest::KeyEvent::KeyType e = SystemAPI::translateKey(k);

  if( !scut ){
    if( Event::K_0<=e && e<= Event::K_9 )
      e = Tempest::KeyEvent::K_NoKey;

    if( Event::K_A<=e && e<= Event::K_Z )
      e = Tempest::KeyEvent::K_NoKey;
    }

  return Tempest::KeyEvent( e );
  }

LRESULT CALLBACK WindowProc( HWND   hWnd,
                             UINT   msg,
                             WPARAM wParam,
                             LPARAM lParam ) {
    //return DefWindowProc( hWnd, msg, wParam, lParam );

    Tempest::Window* w = 0;
    std::unordered_map<LinuxAPI::Window*, Tempest::Window*>::iterator i
        = wndWx.find( (LinuxAPI::Window*)hWnd );

    if( i!= wndWx.end() )
      w = i->second;

    if( !w )
      return DefWindowProc( hWnd, msg, wParam, lParam );

    switch( msg ) {
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
      break;

      case WM_KEYDOWN:
      {
         SystemAPI::emitEvent( w,
                               makeKeyEvent(wParam),
                               makeKeyEvent(wParam, true),
                               Event::KeyDown );
      }
      break;

      case WM_KEYUP:
      {
         SystemAPI::emitEvent( w,
                               makeKeyEvent(wParam),
                               makeKeyEvent(wParam, true),
                               Event::KeyUp );
      }
      break;


      case WM_LBUTTONDOWN:
      case WM_MBUTTONDOWN:
      case WM_RBUTTONDOWN: {
        MouseEvent e( LOWORD (lParam),
                      HIWORD (lParam),
                      toButton(msg),
                      0,
                      0,
                      Event::MouseDown );
        SystemAPI::emitEvent(w, e);
        }
        break;

      case WM_LBUTTONUP:
      case WM_RBUTTONUP:
      case WM_MBUTTONUP: {
        MouseEvent e( LOWORD (lParam),
                      HIWORD (lParam),
                      toButton(msg),
                      0,
                      0,
                      Event::MouseUp  );
        SystemAPI::emitEvent(w, e);
        }
        break;

      case WM_MOUSEMOVE: {
        MouseEvent e( LOWORD (lParam),
                      HIWORD (lParam),
                      Event::ButtonNone,
                      0,
                      0,
                      Event::MouseMove  );
        SystemAPI::emitEvent(w, e);
        }
        break;

       case WM_MOUSEWHEEL:{
          POINT p;
          p.x = LOWORD (lParam);
          p.y = HIWORD (lParam);

          ScreenToClient(hWnd, &p);

          Tempest::MouseEvent e( p.x, p.y,
                                 Tempest::Event::ButtonNone,
                                 GET_WHEEL_DELTA_WPARAM(wParam),
                                 0,
                                 Event::MouseWheel );
          SystemAPI::emitEvent(w, e);
          //w->mouseWheelEvent(e);
          }
        break;

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

          if( !a ){
            //ChangeDisplaySettings(&defaultMode, 0);
            } else {
            //ChangeDisplaySettings(&appMode, appDevModeFlg);
            }
      }
      break;

      case WM_CLOSE:{
        Tempest::CloseEvent e;
        SystemAPI::emitEvent(w, e);
        if( !e.isAccepted() )
          PostQuitMessage(0);
        }
        break;
      case WM_DESTROY: {
        PostQuitMessage(0);
        }
        break;
/*
      case WM_SETCURSOR:
      // If the window is minimized, draw hCurs1.
      // If the window is not minimized, draw the
      // default cursor (class cursor).

        if (IsIconic(hWnd)) {
          //SetCursor(hCurs1);
          break;
          }
        break;*/

      default: {
        return DefWindowProc( hWnd, msg, wParam, lParam );
        }
      }

    return 0;
  }

#endif
