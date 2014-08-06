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
  WinRt::getScreenRect( WinRt::getMainRtWidget(), w, h );
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
  quit = WinRt::nextEvent();
  render(window);
  return 0;
  }

int WinPhoneAPI::nextEvents( bool &quit ) {
  quit = WinRt::nextEvents();
  render( window );
  return 0;
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
  WinRt::setMainWidget(wx);
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
  T_ASSERT( sizeof( wchar_t ) == sizeof( char16_t ) );

  wchar_t flg[4] = {};
  for (int i = 0; i < 4 && mode[i]; ++i)
    flg[i] = mode[i]; 

  std::wstring str = WinRt::getAssetsFolder()+L"\\Assets\\"+(wchar_t*)fname;
  FILE* f = _wfopen( str.c_str(), flg );
  if( f )
    return (File*)f;
  return (File*)_wfopen( (wchar_t*)fname, flg );
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
