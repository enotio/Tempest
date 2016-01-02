#ifdef __APPLE__

#include <Cocoa/Cocoa.h>
#include <unordered_map>
#include <Tempest/Window>
#include "osxapi.h"
#include <OpenGL/gl.h>

using namespace Tempest;

static std::unordered_map<id, Tempest::Window*> wndWx;

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

  id wnd = [[[NSWindow alloc] initWithContentRect:NSMakeRect(0, 0, w, h)
                 styleMask:NSTitledWindowMask
                 backing:NSBackingStoreBuffered
                 defer:NO]
               autorelease];
  [wnd cascadeTopLeftFromPoint:NSMakePoint(20,20)];
  [wnd setTitle:appName];
  [wnd makeKeyAndOrderFront:nil];
  [wnd setStyleMask:[wnd styleMask] | flags];

  return wnd;
  }

static void render( Tempest::Window* w ){
  if( w->showMode()!=Tempest::Window::Minimized && w->isActive() )
    w->render();
  }

OsxAPI::OsxAPI() {
    /*
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
    { XK_Return, Event::K_RetFurn },

    { XK_F1,     Event::K_F1 },
    {   48,      Event::K_0  },
    {   97,      Event::K_A  },

    { 0,         Event::K_NoKey }
    };

  setupKeyTranslate(k);*/
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
  return Size(14440,980);
  }

void OsxAPI::startApplication(SystemAPI::ApplicationInitArgs *) {
  [NSApplication sharedApplication];
  [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
  }

void OsxAPI::endApplication() {
  //NOPE
  }

static bool processEvent(NSEvent* event){
  uint type = [event type];
  id window = [event window];

  auto it = wndWx.begin();//.find(window);
  if(it==wndWx.end()){
    [NSApp sendEvent:event];
    return false;
    }

  switch(type){
    case 0:
      render(it->second);
      [NSApp sendEvent:event];
      return false;
    case NSLeftMouseDown:
      break;

    default:
      [NSApp sendEvent:event];
    }
  return true;
  }

int OsxAPI::nextEvent(bool &quit) {
  NSEvent* event =
  [NSApp nextEventMatchingMask: NSAnyEventMask
                                untilDate:[NSDate distantPast]
                                inMode:NSDefaultRunLoopMode
                                dequeue:YES ];
  processEvent(event);
  quit = false;
  return 0;
  }

int OsxAPI::nextEvents(bool &quit) {
  quit = false;
  bool hasEvents = true;

  while(hasEvents){
    NSEvent* event =
    [NSApp nextEventMatchingMask: NSAnyEventMask
                                  untilDate:[NSDate distantPast]
                                  inMode:NSDefaultRunLoopMode
                                  dequeue:YES ];
    if(!processEvent(event))
      break;
    }

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
  return Size(frame.size.width,frame.size.height);
  }

void OsxAPI::deleteWindow(SystemAPI::Window *w) {
  id wnd = (id)w;
  wndWx.erase(wnd);
  [wnd release];
  }

void OsxAPI::show(SystemAPI::Window *) {

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
  CpuInfo info;
  memset(&info, 0, sizeof(info));

  info.cpuCount = 1;
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

void OsxAPI::glSwapBuffers(void *ctx) {
  [(id)ctx flushBuffer];
  }

#endif
