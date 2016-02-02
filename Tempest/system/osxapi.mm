#include <Tempest/Platform>
#ifdef __OSX__

#define _XOPEN_SOURCE

#include <Cocoa/Cocoa.h>
#include <Carbon/Carbon.h>
#include <unordered_map>
#include <algorithm>

#include <Tempest/Window>
#include <Tempest/Event>

#include "osxapi.h"
#include "appdelegate.h"
#include "thirdparty/utf8cpp/utf8.h"
#include <OpenGL/gl.h>
#include <pthread.h>

#include <ucontext.h>
#include <unistd.h>
#include <atomic>

using namespace Tempest;

static const uint keyTable[26]={
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
  };
static std::unordered_map<id, Tempest::Window*> wndWx;

struct OsxAPI::Fiber  {
  ucontext_t  fib;
  jmp_buf     jmp;
  };

struct OsxAPI::FiberCtx  {
  void(*      fnc)(void*);
  void*       ctx;
  jmp_buf*    cur;
  ucontext_t* prv;
  };

struct OsxAPI::PBox {
  enum {
    sz = (sizeof(void*)+sizeof(int)-1)/sizeof(int)
    };
  int val[2];

  void set(void* v){
    memcpy(val,&v,sizeof(v));
    }
  };

volatile bool appQuit = false;
OsxAPI::Fiber mainContext, appleContext;
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
  OsxAPI::PBox ptr;
  ptr.val[0] = v0;
  ptr.val[1] = v1;

  OsxAPI::FiberCtx* ctx;
  memcpy(&ctx,&ptr.val,sizeof(void*));

  void (*ufnc)(void*) = ctx->fnc;
  void* uctx = ctx->ctx;
  if(_setjmp(*ctx->cur) == 0)  {
    ucontext_t tmp;
    swapcontext(&tmp, ctx->prv);
    }
  ufnc(uctx);
  }

inline static void createFiber(OsxAPI::Fiber& fib, void(*ufnc)(void*), void* uctx, char* stk, size_t ssize)  {
  getcontext(&fib.fib);

  fib.fib.uc_stack.ss_sp   = stk;
  fib.fib.uc_stack.ss_size = ssize;
  fib.fib.uc_link = 0;

  ucontext_t tmp;
  OsxAPI::FiberCtx ctx = {ufnc, uctx, &fib.jmp, &tmp};

  T_ASSERT_X(OsxAPI::PBox::sz<=2,"x86;x64 code");
  OsxAPI::PBox ptr;
  ptr.set(&ctx);

  makecontext(&fib.fib, (void(*)())fiberStartFnc, 2, ptr.val[0], ptr.val[1]);
  swapcontext(&tmp, &fib.fib);
  }

inline static void switch2Fiber(OsxAPI::Fiber& fib, OsxAPI::Fiber& prv) {
  if(_setjmp(prv.jmp) == 0)
    _longjmp(fib.jmp, 1);
  }

static Event::MouseButton toButton( uint type ){
  if( type==NSLeftMouseDown || type==NSLeftMouseUp )
    return Event::ButtonLeft;

  if( type==NSRightMouseDown || type==NSRightMouseUp )
    return Event::ButtonRight;

  if( type==NSOtherMouseDown || type==NSOtherMouseUp )
    return Event::ButtonMid;

  return Event::ButtonNone;
  }

@interface TempestWindow : NSWindow <NSWindowDelegate> {}
  @property (nonatomic) BOOL closeEventResult;
@end

@implementation TempestWindow

-(void) processEvent:(NSEvent *)event {
  uint type = [event type];
  switch(type){
    case NSLeftMouseDown:
    case NSRightMouseDown:
    case NSOtherMouseDown:{
      state.eventType = Event::MouseDown;
      new (&state.event.mouse)
        MouseEvent ( event.locationInWindow.x,
                     event.locationInWindow.y,
                     toButton( type ),
                     0,
                     0,
                     Event::MouseDown
                     );
      OsxAPI::swapContext();
      }
      break;

    case NSLeftMouseUp:
    case NSRightMouseUp:
    case NSOtherMouseUp:{
      state.eventType = Event::MouseUp;
      new (&state.event.mouse)
        MouseEvent( event.locationInWindow.x,
                    event.locationInWindow.y,
                    toButton( type ),
                    0,
                    0,
                    Event::MouseUp
                    );
      OsxAPI::swapContext();
      }
      break;

    case NSLeftMouseDragged:
    case NSRightMouseDragged:
    case NSOtherMouseDragged:
    case NSMouseMoved: {
      state.eventType = Event::MouseMove;
      new (&state.event.mouse)
        MouseEvent( event.locationInWindow.x,
                    event.locationInWindow.y,
                    Event::ButtonNone,
                    0,
                    0,
                    Event::MouseMove  );
      OsxAPI::swapContext();
      }
      break;

    case NSScrollWheel:{
      state.eventType = Event::MouseWheel;
      float ticks = [event deltaY];
      new (&state.event.mouse)
          Tempest::MouseEvent( event.locationInWindow.x,
                               event.locationInWindow.y,
                               Tempest::Event::ButtonNone,
                               ticks>0 ? 100 : -100,
                               0,
                               Event::MouseWheel );
      OsxAPI::swapContext();
      }
      break;

    case NSKeyDown:
    case NSKeyUp: {
      Event::Type eType = type==NSKeyDown ? Event::KeyDown : Event::KeyUp;
      unsigned short key   = [event keyCode];
      NSString* text       = [event characters];
      const char *utf8text = [text UTF8String];

      Tempest::KeyEvent::KeyType kt;
      switch(key){
        case kVK_ANSI_0: kt = KeyEvent::K_0; break;
        case kVK_ANSI_1: kt = KeyEvent::K_1; break;
        case kVK_ANSI_2: kt = KeyEvent::K_2; break;
        case kVK_ANSI_3: kt = KeyEvent::K_3; break;
        case kVK_ANSI_4: kt = KeyEvent::K_4; break;
        case kVK_ANSI_5: kt = KeyEvent::K_5; break;
        case kVK_ANSI_6: kt = KeyEvent::K_6; break;
        case kVK_ANSI_7: kt = KeyEvent::K_7; break;
        case kVK_ANSI_8: kt = KeyEvent::K_8; break;
        case kVK_ANSI_9: kt = KeyEvent::K_9; break;
        case kVK_F1:     kt = KeyEvent::K_F1; break;
        case kVK_F2:     kt = KeyEvent::K_F2; break;
        case kVK_F3:     kt = KeyEvent::K_F3; break;
        case kVK_F4:     kt = KeyEvent::K_F4; break;
        case kVK_F5:     kt = KeyEvent::K_F5; break;
        case kVK_F6:     kt = KeyEvent::K_F6; break;
        case kVK_F7:     kt = KeyEvent::K_F7; break;
        case kVK_F8:     kt = KeyEvent::K_F8; break;
        case kVK_F9:     kt = KeyEvent::K_F9; break;
        case kVK_F10:    kt = KeyEvent::K_F10; break;
        case kVK_F11:    kt = KeyEvent::K_F11; break;
        case kVK_F12:    kt = KeyEvent::K_F12; break;
        case kVK_F13:    kt = KeyEvent::K_F13; break;
        case kVK_F14:    kt = KeyEvent::K_F14; break;
        case kVK_F15:    kt = KeyEvent::K_F15; break;
        case kVK_F16:    kt = KeyEvent::K_F16; break;
        case kVK_F17:    kt = KeyEvent::K_F17; break;
        case kVK_F18:    kt = KeyEvent::K_F18; break;
        case kVK_F19:    kt = KeyEvent::K_F19; break;
        case kVK_F20:    kt = KeyEvent::K_F20; break;
        default: kt = SystemAPI::translateKey(key);
        }

      char16_t txt16[10] = {};
      if(utf8text &&
         key!=kVK_LeftArrow &&
         key!=kVK_RightArrow &&
         key!=kVK_UpArrow &&
         key!=kVK_DownArrow &&
         !(Event::K_F1<=kt && kt<=Event::K_F24) )
        utf8::unchecked::utf8to16(utf8text, utf8text+std::max<int>(10,strlen(utf8text)), txt16 );

      for(const uint& k:keyTable)
        if(k==key)
          kt = Event::KeyType(Event::K_A+(&k-keyTable));

      if(kt!=Event::K_NoKey || txt16[0]!='\0'){
        state.eventType = eType;
        new (&state.event.key) Tempest::KeyEvent( kt, txt16[0], state.eventType );
        OsxAPI::swapContext();
        }
      }
      break;
    }
  }

-(void)mouseDown:(NSEvent *)event {
  [self processEvent:event];
  }

-(void)mouseUp:(NSEvent *)event {
  [self processEvent:event];
  }

-(void)mouseMoved:(NSEvent *)event {
  [self processEvent:event];
  }

-(void)mouseDragged:(NSEvent *)event {
  [self processEvent:event];
  }

-(void)rightMouseDown:(NSEvent *)event {
  [self processEvent:event];
  }

-(void)rightMouseUp:(NSEvent *)event {
  [self processEvent:event];
  }

-(void)rightMouseMoved:(NSEvent *)event {
  [self processEvent:event];
  }

-(void)rightMouseDragged:(NSEvent *)event {
  [self processEvent:event];
  }

-(void)otherMouseDown:(NSEvent *)event {
  [self processEvent:event];
  }

-(void)otherMouseUp:(NSEvent *)event {
  [self processEvent:event];
  }

-(void)otherMouseMoved:(NSEvent *)event {
  [self processEvent:event];
  }

-(void)otherMouseDragged:(NSEvent *)event {
  [self processEvent:event];
  }

-(void) scrollWheel:(NSEvent *)event {
  [self processEvent:event];
  }

-(void) keyDown:(NSEvent *)event {
  [self processEvent:event];
  }

-(void) keyUp:(NSEvent *)event {
  [self processEvent:event];
  }

- (void) flagsChanged: (NSEvent *)event {
  NSUInteger flags = [[NSApp currentEvent] modifierFlags];
  int flg = (flags & NSCommandKeyMask);

  if( (flg&NSControlKeyMask )!=state.ctrl ){
    state.ctrl = (flg&NSCommandKeyMask);

    state.eventType = Tempest::KeyEvent::KeyDown;
    new (&state.event.key) Tempest::KeyEvent( KeyEvent::K_Control, state.eventType );
    OsxAPI::swapContext();

    state.eventType = Tempest::KeyEvent::KeyUp;
    new (&state.event.key) Tempest::KeyEvent( KeyEvent::K_Control, state.eventType );
    OsxAPI::swapContext();
    }

  if( (flg&NSCommandKeyMask )!=state.command ){
    state.command = (flg&NSCommandKeyMask);

    state.eventType = Tempest::KeyEvent::KeyDown;
    new (&state.event.key) Tempest::KeyEvent( KeyEvent::K_Command, state.eventType );
    OsxAPI::swapContext();

    state.eventType = Tempest::KeyEvent::KeyUp;
    new (&state.event.key) Tempest::KeyEvent( KeyEvent::K_Command, state.eventType );
    OsxAPI::swapContext();
    }
  }

@end

@interface WindowController : NSWindowController <NSWindowDelegate> {}
@end

@implementation WindowController

- (BOOL)windowShouldClose:(id)sender {
  TempestWindow* w = (TempestWindow*)sender;
  if(w.closeEventResult==YES)
    return YES;

  state.eventType = Event::Close;
  new (&state.event.close) Tempest::CloseEvent();
  OsxAPI::swapContext();
  return w.closeEventResult;
  }

- (void)windowDidMove:(NSNotification *)notification {
  NSWindow *win = [notification object];

  state.eventType = Event::Type(EventMove);
  new (&state.event.move) Tempest::Point(win.frame.origin.x,win.frame.origin.y);
  OsxAPI::swapContext();
  }

- (void)windowDidResize:(NSNotification *)notification {
  NSWindow* window = [notification object];
  NSRect sz = window.frame;
  sz = [window contentRectForFrameRect:sz];

  state.eventType = Event::Resize;
  state.window    = (void*)window;
  new (&state.event.size) SizeEvent(sz.size.width,sz.size.height);
  OsxAPI::swapContext();
  }

- (void)windowDidMiniaturize:(NSNotification *)notification {
  NSWindow* window = [notification object];
  state.eventType = Event::Type(EventMinimize);
  state.window    = (void*)window;
  OsxAPI::swapContext();
  }

-(void) windowDidDeminiaturize:(NSNotification *)notification {
  NSWindow* window = [notification object];
  NSRect sz = window.frame;
  sz = [window contentRectForFrameRect:sz];

  state.eventType = Event::Type(EventDeMinimize);

  new (&state.event.size) SizeEvent(sz.size.width,sz.size.height);
  state.window    = (void*)window;
  OsxAPI::swapContext();
  }

@end

static id createWindow(int w,int h,unsigned flags,Window::ShowMode mode){
  id menubar     = [[NSMenu new] autorelease];
  id appMenuItem = [[NSMenuItem new] autorelease];

  [menubar addItem:appMenuItem];
  [NSApp setMainMenu:menubar];

  id appMenu      = [[NSMenu new] autorelease];
  id appName      = [[NSProcessInfo processInfo] processName];
  id quitTitle    = [@"Quit " stringByAppendingString:appName];
  id quitMenuItem = [[[NSMenuItem alloc] initWithTitle:quitTitle
                                         action:@selector(stop:)
                                         keyEquivalent:@"q"] autorelease];
  [appMenu addItem:quitMenuItem];
  [appMenuItem setSubmenu:appMenu];

  TempestWindow* wnd = [[TempestWindow alloc] initWithContentRect:
                 NSMakeRect(0, 0, w, h)
                 styleMask:NSTitledWindowMask
                 backing:NSBackingStoreBuffered
                 defer:NO];
  [wnd cascadeTopLeftFromPoint:NSMakePoint(20,20)];
  [wnd setTitle:appName];
  [wnd makeKeyAndOrderFront:nil];
  [wnd setStyleMask:[wnd styleMask] | flags];
  [wnd setAcceptsMouseMovedEvents: YES];
  wnd.closeEventResult=NO;

  switch (mode) {
    case Window::Normal: break;
    case Window::Minimized:
      [wnd miniaturize:wnd];
      break;
    case Window::Maximized:
      [wnd setFrame:[[NSScreen mainScreen] visibleFrame] display:YES];
      break;
    case Window::FullScreen:
      [wnd toggleFullScreen:wnd];
      break;
    }

  id winController = [[WindowController alloc] init];

  [wnd setDelegate:winController];

  //[winController release];
  return wnd;
  }

static void render( Tempest::Window* w ){
  if( w->showMode()!=Tempest::Window::Minimized && w->isActive() )
    w->render();
  }

OsxAPI::OsxAPI(){
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
    };

  setupKeyTranslate(k);
  setFuncKeysCount(20);
  }

OsxAPI::~OsxAPI() {
  }

bool OsxAPI::testDisplaySettings(SystemAPI::Window*, const DisplaySettings &) {
  return true;
  }

bool OsxAPI::setDisplaySettings(SystemAPI::Window *, const DisplaySettings &) {
  return false;
  }

Tempest::Size OsxAPI::implScreenSize() {
  return Size(1440,980);//TODO
  }

static void appleMain(void*){
  [NSAutoreleasePool new];
  [NSApplication sharedApplication];
  [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
  [NSApp activateIgnoringOtherApps:YES];

  id delegate=[AppDelegate new];
  [NSApp setDelegate:delegate];
  [NSApp run];
  }

void OsxAPI::startApplication(SystemAPI::ApplicationInitArgs *) {
  createFiber(appleContext,appleMain,nullptr,appleStack,sizeof(appleStack));
  switch2Fiber(appleContext,mainContext);
  }

void OsxAPI::endApplication() {
  //swapcontext(&main_context2,&appleContext);
  }

bool OsxAPI::processEvent(){
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
      ((TempestWindow*)wi).closeEventResult = ev.isAccepted() ? NO : YES;
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
      uint mask = [wi styleMask];

      Tempest::Window::ShowMode mode=Tempest::Window::Normal;
      if(mask & NSFullScreenWindowMask){
        mode=Tempest::Window::FullScreen;
        } else {
        NSRect visibleFr=[[NSScreen mainScreen] visibleFrame];
        NSRect wFr=[wi frame];
        if(visibleFr.origin.x==wFr.origin.x &&
           visibleFr.origin.y==wFr.origin.y &&
           visibleFr.size.width==wFr.size.width &&
           visibleFr.size.height==wFr.size.height)
          mode=Tempest::Window::Maximized;
        }

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

int OsxAPI::nextEvent(bool &quit) {
  switch2Fiber(appleContext,mainContext);

  if(!processEvent()){
    for(auto i:wndWx)
      render(i.second);
    }

  quit |= appQuit;
  return 0;
  }

int OsxAPI::nextEvents(bool &quit) {
  // no queue
  switch2Fiber(appleContext,mainContext);

  if(!processEvent()){
    for(auto i:wndWx)
      render(i.second);
    }

  quit |= appQuit;
  return 0;
  }

SystemAPI::Window* OsxAPI::createWindow(int w, int h) {
  return (SystemAPI::Window*)(::createWindow(w,h,NSResizableWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask, Tempest::Window::Normal));
  }

SystemAPI::Window *OsxAPI::createWindowMaximized() {
  id w = ::createWindow(800,600,NSResizableWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask, Tempest::Window::Maximized);
  return (SystemAPI::Window*)w;
  }

SystemAPI::Window *OsxAPI::createWindowMinimized() {
  return (SystemAPI::Window*)(::createWindow(800,600,NSResizableWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask, Tempest::Window::Minimized));
  }

SystemAPI::Window *OsxAPI::createWindowFullScr() {
  id w = ::createWindow(800,600,NSResizableWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask, Tempest::Window::FullScreen);
  return (SystemAPI::Window*)w;
  }

Widget *OsxAPI::addOverlay(WindowOverlay *ov) {
  if( wndWx.empty() ){
    delete ov;
    return 0;
    }

  Tempest::Window* w = wndWx.begin()->second;
  SystemAPI::addOverlay(w, ov);
  return ov;
  }

Tempest::Point OsxAPI::windowClientPos(SystemAPI::Window *w) {
  NSRect frame = [(id)w frame];
  return Tempest::Point(frame.origin.x,frame.origin.y);
  }

Tempest::Size OsxAPI::windowClientRect(SystemAPI::Window *w) {
  NSRect frame = [(id)w frame];
  frame = [(id)w contentRectForFrameRect:frame];
  return Size(frame.size.width,frame.size.height);
  }

void OsxAPI::deleteWindow(SystemAPI::Window *w) {
  id wnd = (id)w;
  wndWx.erase(wnd);
  }

void OsxAPI::show(SystemAPI::Window *w) {
  NSWindow* wx = (NSWindow*)w;
  if( !wx.miniaturized )
    [(id)w orderFrontRegardless];
  }

void OsxAPI::setGeometry(SystemAPI::Window *wx, int x, int y, int w, int h){
  NSRect frame = [(id)wx frame];
  if(frame.origin.x==x && frame.origin.y==y &&
     frame.size.width==w && frame.size.height==h )
    return;
  frame.origin.x    = x;
  frame.origin.y    = y;
  frame.size.width  = w;
  frame.size.height = h;
  frame = [(id)wx frameRectForContentRect:frame];

  //state.eventType = Event::Type(EventWindowGeometry);
  //new (&state.event.setGeometry) Tempest::Rect(x,y,w,h);

  NSWindow* wnd = (NSWindow*)wx;
  //[wnd setFrame: frame display:YES];
  //OsxAPI::swapContext();
  }

void OsxAPI::bind(SystemAPI::Window *i, Tempest::Window *w) {
  wndWx[(id)i] = w;
  }

SystemAPI::CpuInfo OsxAPI::cpuInfoImpl() {
  host_basic_info_data_t hostInfo;
  mach_msg_type_number_t infoCount;

  infoCount = HOST_BASIC_INFO_COUNT;
  host_info( mach_host_self(), HOST_BASIC_INFO, (host_info_t)&hostInfo, &infoCount );


  CpuInfo info;
  memset(&info, 0, sizeof(info));

  info.cpuCount = hostInfo.max_cpus;
  return info;
  }

SystemAPI::File *OsxAPI::fopenImpl(const char *fname, const char *mode) {
  return SystemAPI::fopenImpl( fname, mode );
  }

SystemAPI::File *OsxAPI::fopenImpl(const char16_t *fname, const char *mode) {
  return SystemAPI::fopenImpl( fname, mode );
  }
/*
static CVReturn MyDisplayLinkCallback(CVDisplayLinkRef displayLink,
                                      const CVTimeStamp* now,
                                      const CVTimeStamp* outputTime,
                                      CVOptionFlags flagsIn,
                                      CVOptionFlags* flagsOut,
                                      void* displayLinkContext) {
  //OsxAPI::swapContext();
  return kCVReturnSuccess;
  }*/

void* OsxAPI::initializeOpengl(void* wnd) {
  id window = (id)wnd;

  static NSOpenGLPixelFormatAttribute glAttributes[] = {
    NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersionLegacy,
    NSOpenGLPFAColorSize,     24,
    NSOpenGLPFAAlphaSize,     8,
    NSOpenGLPFADepthSize,     32,
    NSOpenGLPFAAccelerated,
    NSOpenGLPFADoubleBuffer,
    0
    };

  NSOpenGLPixelFormat* pixelFormat   = [[NSOpenGLPixelFormat alloc] initWithAttributes:glAttributes];
  NSOpenGLContext*     openGLContext = [[NSOpenGLContext alloc]initWithFormat:pixelFormat shareContext:nil];
  [openGLContext setView:[window contentView]];

  /*
  //TODO: CoreVide support

  CVDisplayLinkRef displayLink;
  // Create a display link capable of being used with all active displays
  CVDisplayLinkCreateWithActiveCGDisplays(&displayLink);

  // Set the renderer output callback function
  CVDisplayLinkSetOutputCallback(displayLink, &MyDisplayLinkCallback, nullptr);

  // Set the display link for the current renderer
  CGLContextObj cglContext = [openGLContext CGLContextObj];
  CGLPixelFormatObj cglPixelFormat = [pixelFormat CGLPixelFormatObj];
  CVDisplayLinkSetCurrentCGDisplayFromOpenGLContext(displayLink, cglContext, cglPixelFormat);

  // Activate the display link
  CVDisplayLinkStart(displayLink);
  */

  return openGLContext;
  }

bool OsxAPI::glMakeCurrent(void *ctx) {
  [(id)ctx update];
  [(id)ctx makeCurrentContext];
  return true;
  }

bool OsxAPI::glUpdateContext(void* ctx, void* /*window*/) {
  [(id)ctx update];
  return true;
  }

void OsxAPI::glSwapBuffers(void *ctx) {
  [(id)ctx flushBuffer];
  }

void OsxAPI::swapContext() {
  std::atomic_thread_fence(std::memory_order_acquire);
  switch2Fiber(mainContext,appleContext);
  }

void OsxAPI::finish() {
  appQuit = true;
  swapContext();
  }

#endif
