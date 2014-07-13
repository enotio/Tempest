#include "winphoneapi.h"

#ifdef __WINDOWS_PHONE__
#include <windows.h>
#include <wrl.h>

#include <Tempest/Window>
#include <Tempest/Event>
#include <Tempest/DisplaySettings>

#include <unordered_map>
#include <fstream>
#include <Tempest/Assert>

#include <iostream>
#include "core/wrappers/atomic.h"

#include <win_rt_api_binding.h>

static void runApplicatiion(){
  //CoreApplication::Run( nullptr );
  }

static Tempest::Window* window = 0;

using namespace Tempest;

WinPhoneAPI::WinPhoneAPI() {
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

  setupKeyTranslate(k);
  setFuncKeysCount(24);
  }

WinPhoneAPI::~WinPhoneAPI() {
  }

int WinPhoneAPI::preMain( int(_cdecl*func_main )(int, const char**) ){
  return WinRt::startApplication( func_main );
  }

bool WinPhoneAPI::testDisplaySettings( Window*, const DisplaySettings & s ) {
  return false;
  }

bool WinPhoneAPI::setDisplaySettings( Window* w, const DisplaySettings &s ) {
  return false;
  }
 
Size WinPhoneAPI::implScreenSize() {
  int w=0, h=0;
  //WinRt::getScreenSize( w, h );
  return Size(w,h);
  }

void WinPhoneAPI::startApplication(ApplicationInitArgs *) {

  }

void WinPhoneAPI::endApplication() {

  }

static void render( Tempest::Window* w ){
  if( w->showMode()!=Tempest::Window::Minimized && w->isActive() )
    w->render();
  }

int WinPhoneAPI::nextEvent(bool &quit) {
  return 0;
  /*
  MSG uMsg;
  memset(&uMsg,0,sizeof(uMsg));

  if( PeekMessage( &uMsg, NULL, 0, 0, PM_REMOVE ) ){
    if( uMsg.message==WM_QUIT )
      quit = 1;

    TranslateMessage( &uMsg );
    DispatchMessage ( &uMsg );
    Sleep(0);
    return uMsg.wParam;
    } else {
    for( auto i=wndWx.begin(); i!=wndWx.end(); ++i )
      render( i->second );

    return 0;
    }
  */
  }

int WinPhoneAPI::nextEvents( bool &quit ) {
  return 0;
  /*
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
  */
  }

WinPhoneAPI::Window *WinPhoneAPI::createWindow(int w, int h) {
  (void)w;
  (void)h;
  return createWindowFullScr();
  }

SystemAPI::Window *WinPhoneAPI::createWindowMaximized() {
  return createWindowFullScr();
  }

SystemAPI::Window *WinPhoneAPI::createWindowMinimized() {
  return createWindowFullScr();
  }

SystemAPI::Window *WinPhoneAPI::createWindowFullScr() {
  return 0;// (Window*)WinRt::getWindow();// (Window*)hwnd;
  }
 
Widget* WinPhoneAPI::addOverlay(WindowOverlay *ov) {
  if( window==0 ){
    delete ov;
    return 0;
    }

  SystemAPI::addOverlay(window, ov );
  return ov;
  }

Point WinPhoneAPI::windowClientPos( SystemAPI::Window * hWnd ) {
  return Point( 0, 0 );
  }

Size WinPhoneAPI::windowClientRect( SystemAPI::Window * hWnd ) {
  return screenSize();
  }

void WinPhoneAPI::deleteWindow( Window *w ) {
  window = 0;
  }

void WinPhoneAPI::show(Window *hWnd) {
  }

void WinPhoneAPI::setGeometry( Window *hw, int x, int y, int w, int h ) {
  }

void WinPhoneAPI::bind( Window *w, Tempest::Window *wx ) {
  window = wx;
  }

WinPhoneAPI::CpuInfo WinPhoneAPI::cpuInfoImpl(){
  CpuInfo info;
  memset(&info, 0, sizeof(info));

  info.cpuCount = 1;
  return info;
  }

struct WinPhoneAPI::WinFile{
  HANDLE h;
  DWORD  pos, size;
  };

WinPhoneAPI::File *WinPhoneAPI::fopenImpl( const char *fname, const char *mode ) {
  return fopenImpl( toUtf16(fname).data(), mode );
  }

WinPhoneAPI::File *WinPhoneAPI::fopenImpl( const char16_t *fname, const char *mode ) {
  return (SystemAPI::File*)0;
  }

size_t WinPhoneAPI::readDataImpl(SystemAPI::File *f, char *dest, size_t count) {
  return 0;
  }

size_t WinPhoneAPI::peekImpl(SystemAPI::File *f, size_t skip, char *dest, size_t count) {
  return 0;
  }

size_t WinPhoneAPI::writeDataImpl(SystemAPI::File *f, const char *data, size_t count) {
  return 0;
  }

void WinPhoneAPI::flushImpl(SystemAPI::File *f) {
  //WinFile *fn = (WinFile*)f;
  }

size_t WinPhoneAPI::skipImpl(SystemAPI::File *f, size_t count) {
  return 0;
  }

bool WinPhoneAPI::eofImpl(SystemAPI::File *f) {
  WinFile *fn = (WinFile*)f;
  return fn->pos==fn->size;
  }

size_t WinPhoneAPI::fsizeImpl( File *f ){
  WinFile *fn = (WinFile*)f;
  return fn->size;
  }

void WinPhoneAPI::fcloseImpl(SystemAPI::File *f) {
  CloseHandle( ((WinFile*)f)->h );
  delete ((WinFile*)f);
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
#endif
