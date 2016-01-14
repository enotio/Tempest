#include <Tempest/Platform>
#ifdef __IOS__

#define _XOPEN_SOURCE

#include "iosapi.h"
#import  <UIKit/UIKit.h>
#include <unordered_map>
#include <algorithm>

#include <Tempest/Window>
#include <Tempest/Event>

#include "appdelegate.h"
#include "thirdparty/utf8cpp/utf8.h"
#include <OpenGLES/EAGL.h>
#include <OpenGLES/ES2/gl.h>
#include <pthread.h>

#include <ucontext.h>
#include <unistd.h>
#include <mach/mach_host.h>
#include <atomic>

using namespace Tempest;

static const uint keyTable[26]={
  0/*
  kVK_ANSI_A,
  kVK_ANSI_B,
  kVK_ANSI_C,
  kVK_ANSI_D,
  kVK_ANSI_E,
  kVK_ANSI_F,
  kVK_ANSI_G,
  kVK_ANSI_H,
  kVK_ANSI_I,
  kVK_ANSI_J,
  kVK_ANSI_K,
  kVK_ANSI_L,
  kVK_ANSI_M,
  kVK_ANSI_N,
  kVK_ANSI_O,
  kVK_ANSI_P,
  kVK_ANSI_Q,
  kVK_ANSI_R,
  kVK_ANSI_S,
  kVK_ANSI_T,
  kVK_ANSI_U,
  kVK_ANSI_V,
  kVK_ANSI_W,
  kVK_ANSI_X,
  kVK_ANSI_Y,
  kVK_ANSI_Z
  */
  };
static std::unordered_map<id, Tempest::Window*> wndWx;

struct iOSAPI::Fiber  {
  ucontext_t  fib;
  jmp_buf     jmp;
  };

struct iOSAPI::FiberCtx  {
  void(*      fnc)(void*);
  void*       ctx;
  jmp_buf*    cur;
  ucontext_t* prv;
  };

struct iOSAPI::PBox {
  enum {
    sz = (sizeof(void*)+sizeof(int)-1)/sizeof(int)
    };
  int val[2];

  void set(void* v){
    memcpy(val,&v,sizeof(v));
    }
  };

volatile bool appQuit = false;
iOSAPI::Fiber mainContext, appleContext;
static char   appleStack[1*1024*1024];

enum MacEvent {
  EventMove = Event::Custom+1,
  EventMinimize,
  EventDeMinimize
  };

static struct State {
  union Ev{
    ~Ev(){}

    Event          noEvent;
    SizeEvent      size;// = SizeEvent(0,0);
    MouseEvent     mouse;
    KeyEvent       key;
    CloseEvent     close;
    Tempest::Point move;
    } event = {Event()};

  volatile Event::Type eventType=Event::NoEvent;
  volatile void*       window   =nullptr;
  volatile bool        ctrl=false;
  volatile bool        command=false;
  } state;

static void fiberStartFnc(int v0,int v1)  {
  iOSAPI::PBox ptr;
  ptr.val[0] = v0;
  ptr.val[1] = v1;

  iOSAPI::FiberCtx* ctx;
  memcpy(&ctx,&ptr.val,sizeof(void*));

  void (*ufnc)(void*) = ctx->fnc;
  void* uctx = ctx->ctx;
  if(_setjmp(*ctx->cur) == 0)  {
    ucontext_t tmp;
    swapcontext(&tmp, ctx->prv);
    }
  ufnc(uctx);
  }

inline static void createFiber(iOSAPI::Fiber& fib, void(*ufnc)(void*), void* uctx, char* stk, size_t ssize)  {
  getcontext(&fib.fib);

  fib.fib.uc_stack.ss_sp   = stk;
  fib.fib.uc_stack.ss_size = ssize;
  fib.fib.uc_link = 0;

  ucontext_t tmp;
  iOSAPI::FiberCtx ctx = {ufnc, uctx, &fib.jmp, &tmp};

  T_ASSERT_X(iOSAPI::PBox::sz<=2,"x86;x64 code");
  iOSAPI::PBox ptr;
  ptr.set(&ctx);

  makecontext(&fib.fib, (void(*)())fiberStartFnc, 2, ptr.val[0], ptr.val[1]);
  swapcontext(&tmp, &fib.fib);
  }

inline static void switch2Fiber(iOSAPI::Fiber& fib, iOSAPI::Fiber& prv) {
  if(_setjmp(prv.jmp) == 0)
    _longjmp(fib.jmp, 1);
  }

@interface AppDelegate : NSObject <UIApplicationDelegate> {}
@end
@implementation AppDelegate

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
  [self.window makeKeyAndVisible];
  //[(EAGLView*)self.window.rootViewController.view startAnimation];
  return YES;
  }

- (void)applicationWillResignActive:(UIApplication *)application {
  //[(EAGLView*)self.window.rootViewController.view stopAnimation];
  }


- (void)applicationDidEnterBackground:(UIApplication *)application {
  //[(EAGLView*)self.window.rootViewController.view stopAnimation];
  }


- (void)applicationWillEnterForeground:(UIApplication *)application {
  //[(EAGLView*)self.window.rootViewController.view startAnimation];
  }


- (void)applicationDidBecomeActive:(UIApplication *)application  {
  //[(EAGLView*)self.window.rootViewController.view startAnimation];
  }


- (void)applicationWillTerminate:(UIApplication *)application {
  //[(EAGLView*)self.window.rootViewController.view stopAnimation];
  }

@end

@interface EAGLView : UIView
  @property (nonatomic) BOOL closeEventResult;
@end

@implementation EAGLView

-(void) processEvent:(UIEvent *)event {
  uint type = [event type];
  int x=0,y=0;//TODO
  switch(type){
    case UIEventTypeTouches:{
      state.eventType = Event::MouseDown;
      new (&state.event.mouse)
        MouseEvent ( x,
                     y,
                     Event::ButtonLeft,
                     0,
                     0,
                     Event::MouseDown
                     );
      iOSAPI::swapContext();
      }
      break;

    case UIEventTypeMotion: {
      state.eventType = Event::MouseMove;
      new (&state.event.mouse)
        MouseEvent( x,
                    y,
                    Event::ButtonNone,
                    0,
                    0,
                    Event::MouseMove  );
      iOSAPI::swapContext();
      }
      break;
    }
  }

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event {
  [self processEvent:event];
  }

- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event {
  [self processEvent:event];
  }

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event {
  [self processEvent:event];
  }

@end

static id createWindow(Window::ShowMode mode){
  UIWindow *window = [[ UIWindow alloc ] initWithFrame: [ [ UIScreen mainScreen ] bounds ] ];
  window.backgroundColor = [ UIColor whiteColor ];
  [ window makeKeyAndVisible ];

  UIViewController* vc = [[UIViewController alloc]initWithNibName:nil bundle:nil];
  window.rootViewController = vc;

  //[winController release];
  return window;
  }

static void render( Tempest::Window* w ){
  if( w->showMode()!=Tempest::Window::Minimized && w->isActive() )
    w->render();
  }

iOSAPI::iOSAPI(){
  /*
  TranslateKeyPair k[] = {
    { kVK_LeftArrow,   Event::K_Left   },
    { kVK_RightArrow,  Event::K_Right  },
    { kVK_UpArrow,     Event::K_Up     },
    { kVK_DownArrow,   Event::K_Down   },

    { kVK_Escape,        Event::K_ESCAPE },
    { kVK_Delete,        Event::K_Back   },
    { kVK_ForwardDelete, Event::K_Delete },
    { kVK_Control,       Event::K_Control },
    //{ kVK_Insert,        Event::K_Insert },
    { kVK_Home,          Event::K_Home   },
    { kVK_End,           Event::K_End    },
    //{ kVK_Pause,         Event::K_Pause  },
    { kVK_Return,        Event::K_Return },

    //{ kVK_F1,     Event::K_F1 },
    //{ kVK_ANSI_0, Event::K_0  },
    //{ kVK_ANSI_A, Event::K_A  },

    { 0,          Event::K_NoKey }
    };*/

  //setupKeyTranslate(k);
  //setFuncKeysCount(20);
  }

iOSAPI::~iOSAPI() {
  }

bool iOSAPI::testDisplaySettings(SystemAPI::Window*, const DisplaySettings &) {
  return true;
  }

bool iOSAPI::setDisplaySettings(SystemAPI::Window *, const DisplaySettings &) {
  return false;
  }

Tempest::Size iOSAPI::implScreenSize() {
  return Size(1440,980);
  }

static void appleMain(void*){
  static std::string app="application";
  char * argv[2] = {
    &app[0],nullptr
    };
  UIApplicationMain( 1, argv, nil, NSStringFromClass( [ AppDelegate class ] ) );
  }

void iOSAPI::startApplication(SystemAPI::ApplicationInitArgs *) {
  createFiber(appleContext,appleMain,nullptr,appleStack,sizeof(appleStack));
  switch2Fiber(appleContext,mainContext);
  }

void iOSAPI::endApplication() {
  //swapcontext(&main_context2,&appleContext);
  }

bool iOSAPI::processEvent(){
  if(wndWx.size()==0)
    return false;

  Tempest::Window* w  = wndWx.begin()->second;
  id               wi = wndWx.begin()->first;

  uint type = state.eventType;
  state.eventType = Event::NoEvent;

  switch( type ) {
    case Event::MouseDown:
    case Event::MouseMove:
    case Event::MouseUp:
    case Event::MouseWheel:{
      MouseEvent& e = state.event.mouse;
      MouseEvent ex( e.x,
                     w->h() - e.y,
                     e.button,
                     e.delta,
                     e.mouseID,
                     Event::Type(type)
                     );
      SystemAPI::emitEvent( w, ex );
      }
      break;
    case Event::KeyDown:{
      Tempest::KeyEvent k = KeyEvent(state.event.key.key,Event::Type(type));
      SystemAPI::emitEvent( w,k,k,Event::Type(type) );
      }
      break;
    case Event::KeyUp:{
      Tempest::KeyEvent k = KeyEvent(state.event.key.key,Event::Type(type));
      uint32_t key = state.event.key.u16;
      SystemAPI::emitEvent( w,k,k,Event::Type(type) );

      //WM_CHAR
      Tempest::KeyEvent e = Tempest::KeyEvent( key );
      uint16_t wrd[4] = {
        Event::K_Return,
        Event::K_Back,
        Event::K_Control,
        0
        };

      if( 0 == *std::find( wrd, wrd+3, k.key) && e.u16 ){
        Tempest::KeyEvent ed( Event::K_NoKey, e.u16, Event::KeyDown );
        SystemAPI::emitEvent(w, ed);

        Tempest::KeyEvent eu( Event::K_NoKey, e.u16, Event::KeyUp );
        SystemAPI::emitEvent(w, eu);
        }
      }
      break;
    case Event::Resize:
      SystemAPI::sizeEvent( w, state.event.size.w, state.event.size.h );
      break;
    case Event::Close: {
      CloseEvent ev = state.event.close;
      SystemAPI::emitEvent( w, ev );
      //((TempestWindow*)wi).closeEventResult = ev.isAccepted() ? NO : YES;
      if(!ev.isAccepted())
        [wi close];
      }
      break;
    case EventMove:{
      SystemAPI::moveEvent(w,state.event.move.x,state.event.move.y);
      }
      break;
    case EventMinimize:{
      SystemAPI::setShowMode( w, Tempest::Window::Minimized);
      SystemAPI::activateEvent(w,false);
      }
      break;
    case EventDeMinimize:{
      Tempest::Window::ShowMode mode=Tempest::Window::FullScreen;
      SystemAPI::sizeEvent( w, state.event.size.w, state.event.size.h );
      SystemAPI::setShowMode( w, mode);
      SystemAPI::activateEvent(w,true);
      }
      break;
    default:
      return false;
      break;
      }
  return true;
  }

int iOSAPI::nextEvent(bool &quit) {
  switch2Fiber(appleContext,mainContext);

  if(!processEvent()){
    for(auto i:wndWx)
      render(i.second);
    }

  quit |= appQuit;
  return 0;
  }

int iOSAPI::nextEvents(bool &quit) {
  // no queue
  switch2Fiber(appleContext,mainContext);

  if(!processEvent()){
    for(auto i:wndWx)
      render(i.second);
    }

  quit |= appQuit;
  return 0;
  }

SystemAPI::Window* iOSAPI::createWindow(int w, int h) {
  return (SystemAPI::Window*)(::createWindow(Tempest::Window::Normal));
  }

SystemAPI::Window *iOSAPI::createWindowMaximized() {
  id w = ::createWindow(Tempest::Window::Maximized);
  return (SystemAPI::Window*)w;
  }

SystemAPI::Window *iOSAPI::createWindowMinimized() {
  return (SystemAPI::Window*)(::createWindow(Tempest::Window::Minimized));
  }

SystemAPI::Window *iOSAPI::createWindowFullScr() {
  id w = ::createWindow(Tempest::Window::FullScreen);
  return (SystemAPI::Window*)w;
  }

Widget *iOSAPI::addOverlay(WindowOverlay *ov) {
  if( wndWx.empty() ){
    delete ov;
    return 0;
    }

  Tempest::Window* w = wndWx.begin()->second;
  SystemAPI::addOverlay(w, ov);
  return ov;
  }

Tempest::Point iOSAPI::windowClientPos(SystemAPI::Window *) {
  return Tempest::Point(0,0);
  }

Tempest::Size iOSAPI::windowClientRect(SystemAPI::Window *w) {
  //NSRect frame = [(id)w frame];
  //frame = [(id)w contentRectForFrameRect:frame];
  //return Size(frame.size.width,frame.size.height);
  return Size(480,800);
  }

void iOSAPI::deleteWindow(SystemAPI::Window *w) {
  id wnd = (id)w;
  wndWx.erase(wnd);
  }

void iOSAPI::show(SystemAPI::Window *w) {
  UIWindow* wx = (UIWindow*)w;
  [ wx makeKeyAndVisible ];
  }

void iOSAPI::setGeometry(SystemAPI::Window *, int , int , int , int ){
  }

void iOSAPI::bind(SystemAPI::Window *i, Tempest::Window *w) {
  wndWx[(id)i] = w;
  }

SystemAPI::CpuInfo iOSAPI::cpuInfoImpl() {
  host_basic_info_data_t hostInfo;
  mach_msg_type_number_t infoCount;

  infoCount = HOST_BASIC_INFO_COUNT;
  host_info( mach_host_self(), HOST_BASIC_INFO, (host_info_t)&hostInfo, &infoCount );


  CpuInfo info;
  memset(&info, 0, sizeof(info));

  info.cpuCount = hostInfo.max_cpus;
  return info;
  }

SystemAPI::File *iOSAPI::fopenImpl(const char *fname, const char *mode) {
  return SystemAPI::fopenImpl( fname, mode );
  }

SystemAPI::File *iOSAPI::fopenImpl(const char16_t *fname, const char *mode) {
  return SystemAPI::fopenImpl( fname, mode );
  }
/*
static CVReturn MyDisplayLinkCallback(CVDisplayLinkRef displayLink,
                                      const CVTimeStamp* now,
                                      const CVTimeStamp* outputTime,
                                      CVOptionFlags flagsIn,
                                      CVOptionFlags* flagsOut,
                                      void* displayLinkContext) {
  //iOSAPI::swapContext();
  return kCVReturnSuccess;
  }*/

void* iOSAPI::initializeOpengl(void* wnd) {
  id window = (id)wnd;

  EAGLContext*     openGLContext = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
  //[openGLContext setView:[window contentView]];
  return openGLContext;
  }

bool iOSAPI::glMakeCurrent(void *ctx) {
  [(id)ctx update];
  [EAGLContext setCurrentContext:(id)ctx];
  return true;
  }

bool iOSAPI::glUpdateContext(void* ctx, void* /*window*/) {
  [(id)ctx update];
  return true;
  }

void iOSAPI::glSwapBuffers(void *ctx) {
  [(id)ctx presentRenderbuffer:GL_RENDERBUFFER];
  }

void iOSAPI::swapContext() {
  std::atomic_thread_fence(std::memory_order_acquire);
  switch2Fiber(mainContext,appleContext);
  }

void iOSAPI::finish() {
  appQuit = true;
  swapContext();
  }

#endif
