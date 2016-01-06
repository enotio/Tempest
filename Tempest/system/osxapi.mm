#ifdef __APPLE__

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

using namespace Tempest;

static std::unordered_map<id, Tempest::Window*> wndWx;

volatile bool appQuit = false;
ucontext_t    mainContext1, mainContext2, appleContext;
static char   appleStack[1*1024*1024];

static struct State{
  union Ev{
    ~Ev(){}

    Event      noEvent;
    SizeEvent  size;
    MouseEvent mouse;
    KeyEvent   key;
    } event = {Event()};

  volatile Event::Type eventType=Event::NoEvent;
  volatile void*       window   =nullptr;
  } state;

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
      state.eventType = eType;
      unsigned short key = [event keyCode];
      NSString* text = [event characters];
      const char *utf8text=[text UTF8String];

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
        default: kt = SystemAPI::translateKey(key);
        }
      new (&state.event.key) Tempest::KeyEvent( kt,state.eventType );
      OsxAPI::swapContext();

      if(type==NSKeyUp && utf8text!=nullptr){
        char16_t txt16[10] = {};
        utf8::unchecked::utf8to16(utf8text, utf8text+std::max<int>(10,strlen(utf8text)), txt16 );

        if(txt16[0]){
          state.eventType = eType;
          new (&state.event.key) Tempest::KeyEvent( txt16[0],Event::KeyDown );
          OsxAPI::swapContext();

          state.eventType = eType;
          new (&state.event.key) Tempest::KeyEvent( txt16[0],Event::KeyUp );
          OsxAPI::swapContext();
          }
        }
      [text release];
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

@end

@interface WindowController : NSWindowController <NSWindowDelegate> {}
@end

@implementation WindowController

- (void)windowDidResize:(NSNotification *)notification {
  NSWindow* window = [notification object];
  CGSize sz = window.frame.size;

  state.eventType = Event::Resize;
  state.window    = (void*)window;
  new (&state.event.size) SizeEvent(sz.width,sz.height);
  OsxAPI::swapContext();
  }

@end

static id createWindow(int w,int h,unsigned flags){
  id menubar     = [[NSMenu new] autorelease];
  id appMenuItem = [[NSMenuItem new] autorelease];

  [menubar addItem:appMenuItem];
  [NSApp setMainMenu:menubar];

  id appMenu      = [[NSMenu new] autorelease];
  id appName      = [[NSProcessInfo processInfo] processName];
  id quitTitle    = [@"Quit " stringByAppendingString:appName];
  id quitMenuItem = [[[NSMenuItem alloc] initWithTitle:quitTitle

  action:@selector(terminate:) keyEquivalent:@"q"] autorelease];
  [appMenu addItem:quitMenuItem];
  [appMenuItem setSubmenu:appMenu];

  id wnd = [[TempestWindow alloc] initWithContentRect:NSMakeRect(0, 0, w, h)
                 styleMask:NSTitledWindowMask
                 backing:NSBackingStoreBuffered
                 defer:NO];
  [wnd cascadeTopLeftFromPoint:NSMakePoint(20,20)];
  [wnd setTitle:appName];
  [wnd makeKeyAndOrderFront:nil];
  [wnd setStyleMask:[wnd styleMask] | flags];
  [wnd setAcceptsMouseMovedEvents: YES];

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
    { kVK_ForwardDelete, Event::K_Back   },
    { kVK_Delete,        Event::K_Delete },
    //{ kVK_Insert,        Event::K_Insert },
    { kVK_Home,          Event::K_Home   },
    { kVK_End,           Event::K_End    },
    //{ kVK_Pause,         Event::K_Pause  },
    { kVK_Return,        Event::K_Return },

    { kVK_F1,     Event::K_F1 },
    //{ kVK_ANSI_0, Event::K_0  },
    //{ kVK_ANSI_A, Event::K_A  },

    { 0,          Event::K_NoKey }
    };

  setupKeyTranslate(k);
  setFuncKeysCount(24);
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
  return Size(1440,980);
  }

static void appleMain(){
  [NSAutoreleasePool new];
  [NSApplication sharedApplication];
  [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
  [NSApp activateIgnoringOtherApps:YES];

  id delegate=[AppDelegate new];
  [NSApp setDelegate:delegate];
  [NSApp run];
  }

void OsxAPI::startApplication(SystemAPI::ApplicationInitArgs *) {
  getcontext(&appleContext);
  appleContext.uc_link          = nullptr;//&main_context1;
  appleContext.uc_stack.ss_sp   = appleStack;
  appleContext.uc_stack.ss_size = sizeof(appleStack);

  T_WARNING(mainContext1.uc_link==nullptr);
  mainContext1.uc_link=&appleContext;

  makecontext(&appleContext, appleMain, 0);
  getcontext(&mainContext1);

  swapcontext(&mainContext2,&appleContext);
  }

void OsxAPI::endApplication() {
  //swapcontext(&main_context2,&appleContext);
  }

static bool processEvent(){
  if(wndWx.size()==0)
    return false;

  Tempest::Window* w = wndWx.begin()->second;
  auto type = state.eventType;
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
                     e.type()
                     );
      SystemAPI::emitEvent( w, ex );
      }
      break;
    case Event::KeyDown:
    case Event::KeyUp:
      SystemAPI::emitEvent( w, state.event.key );
      break;
    case Event::Resize:
      SystemAPI::sizeEvent( w, state.event.size.w, state.event.size.h );
      break;
    default:
      return false;
      break;
      }
  return true;
  }

int OsxAPI::nextEvent(bool &quit) {
  swapcontext(&mainContext2,&appleContext);

  if(!processEvent()){
    for(auto i:wndWx)
      render(i.second);
    }

  quit = appQuit;
  return 0;
  }

int OsxAPI::nextEvents(bool &quit) {
  swapcontext(&mainContext2,&appleContext);

  //bool hasEvents = true;
  if(!processEvent()){
    for(auto i:wndWx)
      render(i.second);
    }

  quit = appQuit;
  return 0;
  }

SystemAPI::Window* OsxAPI::createWindow(int w, int h) {
  return (SystemAPI::Window*)(::createWindow(w,h,NSResizableWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask));
  }

SystemAPI::Window *OsxAPI::createWindowMaximized() {
  return (SystemAPI::Window*)(::createWindow(800,600,NSResizableWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask));
  }

SystemAPI::Window *OsxAPI::createWindowMinimized() {
  return (SystemAPI::Window*)(::createWindow(800,600,NSResizableWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask));
  }

SystemAPI::Window *OsxAPI::createWindowFullScr() {
  return (SystemAPI::Window*)(::createWindow(800,600,NSResizableWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask));
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
  return Point(frame.origin.x,frame.origin.y);
  }

Tempest::Size OsxAPI::windowClientRect(SystemAPI::Window *w) {
  NSRect frame = [(id)w frame];
  frame = [(id)w contentRectForFrameRect:frame];
  return Size(frame.size.width,frame.size.height);
  }

void OsxAPI::deleteWindow(SystemAPI::Window *w) {
  id wnd = (id)w;
  wndWx.erase(wnd);
  //[wnd release];
  }

void OsxAPI::show(SystemAPI::Window *w) {
  [(id)w orderFrontRegardless];
  }

void OsxAPI::setGeometry(SystemAPI::Window *wx, int x, int y, int w, int h){
  NSRect frame = [(id)wx frame];
  frame.origin.x    = x;
  frame.origin.y    = y;
  frame.size.width  = w;
  frame.size.height = h;
  //[(id)wx setFrame: frame];
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
  return openGLContext;
  }

bool OsxAPI::glMakeCurrent(void *ctx) {
  [(id)ctx makeCurrentContext];
  return true;
  }

bool OsxAPI::glUpdateContext(void *ctx, void *window) {
  [(id)ctx update];
  }

void OsxAPI::glSwapBuffers(void *ctx) {
  [(id)ctx flushBuffer];
  }

void OsxAPI::swapContext() {
  swapcontext(&appleContext,&mainContext2);
  }

void OsxAPI::finish() {
  appQuit = true;
  swapcontext(&appleContext,&mainContext2);
  }

#endif
