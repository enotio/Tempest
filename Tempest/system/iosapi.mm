#include <Tempest/Platform>
#ifdef __IOS__

#include "iosapi.h"
#import  <UIKit/UIKit.h>
#include <unordered_map>
#include <algorithm>

#include <Tempest/Window>
#include <Tempest/Event>
#include <Tempest/Platform>

#include "thirdparty/utf8cpp/utf8.h"
#include "core/langcodes.h"

#include <OpenGLES/EAGL.h>
#include <OpenGLES/ES2/gl.h>
#include <pthread.h>
#include <mach/machine/thread_status.h>
#include <mach/thread_state.h>

#include <unistd.h>
#include <mach/mach_host.h>
#include <atomic>
#include <sys/utsname.h>

#if TARGET_OS_SIMULATOR
#  define FUNCTION_CALL_ALIGNMENT 16
#  if TARGET_CPU_X86_64
#    define SET_STACK_POINTER "movq %0, %%rsp"
#  elif TARGET_CPU_X86
#    define SET_STACK_POINTER "mov %0, %%esp"
#  endif
#elif TARGET_OS_IOS && !TARGET_OS_SIMULATOR
#  // Valid for both 32 and 64-bit ARM
#  define FUNCTION_CALL_ALIGNMENT 4
#  define SET_STACK_POINTER "mov sp, %0"
#else
#  error "Unknown processor family"
#endif

#define alignDown(val, align) val & ~(align - 1)

using namespace Tempest;

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

volatile           bool appQuit = false;
iOSAPI::Fiber           mainContext, appleContext;
alignas(16) static char appleStack[1*1024*1024]={};
static             void appleMain(void*);

enum MacEvent {
  EventMove = Event::Custom+1,
  EventMinimize,
  EventDeMinimize
  };

struct Touch {
  void*   id;
  CGPoint pos;
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
  volatile id          rootWindow = nullptr;

  std::string          locale;
  std::vector<Touch>   touch;

  size_t addTouch(void* id,const CGPoint& pos){
    for(Touch& t:touch)
      if(t.id==id)
        return -1;

    Touch tx;
    tx.id  = id;
    tx.pos = pos;

    for(size_t i=0;i<touch.size();++i)
      if(touch[i].id==nullptr){
        touch[i] = tx;
        return i;
        }

    touch.push_back(tx);
    return touch.size()-1;
    }

  size_t delTouch(void* id){
    size_t res=-1;
    for(size_t i=0;i<touch.size();++i)
      if(touch[i].id==id){
        touch[i].id = nullptr;
        if(res==size_t(-1))
          res = i;
        }

    while(touch.size() && touch.back().id==nullptr)
      touch.pop_back();

    return res;
    }

  size_t moveTouch(void* id,const CGPoint& pos){
    for(size_t i=0;i<touch.size();++i)
      if(touch[i].id==id){
        if(touch[i].pos.x==pos.x && touch[i].pos.y==pos.y)
          return -1;
        touch[i].pos = pos;
        return i;
        }

    return -1;
    }
  } state;

inline static void createAppleSubContext()  {
  if(_setjmp(mainContext.jmp) == 0) {
    // replace stack
    //static const long kPageSize = sysconf(_SC_PAGESIZE);

    __volatile__ uintptr_t ptr  = reinterpret_cast<uintptr_t>(appleStack);
    __volatile__ uintptr_t base = alignDown(ptr + sizeof(appleStack), FUNCTION_CALL_ALIGNMENT);

    /*
    __asm__ __volatile__("mov lr, %0"
      :
      : "r" (alignDown(0llu, FUNCTION_CALL_ALIGNMENT)));*/

    __asm__ __volatile__(
                SET_STACK_POINTER
                : // no outputs
                : "r" (alignDown(base, FUNCTION_CALL_ALIGNMENT))
            );

    //thread_state_t state;
    //thread_get_state();
    appleMain(nullptr);
    }
  }

inline static void switch2Fiber(iOSAPI::Fiber& fib, iOSAPI::Fiber& prv) {
  if(_setjmp(prv.jmp) == 0)
    _longjmp(fib.jmp, 1);
  }


@interface GLView : UIView<UIKeyInput> {
  CADisplayLink* displayLink;
  }
  @property (nonatomic)         BOOL         closeEventResult;
  @property (nonatomic)         BOOL         canBecomeFirstResponderFlag;
  @property (nonatomic, retain) CAEAGLLayer* egLayer;
@end

@implementation GLView

// We have to implement this method
+ (Class)layerClass {
  return [CAEAGLLayer class];
  }

-(id) initWithFrame: (CGRect) frame {
  if( self = [super initWithFrame:frame] ) {
    self.closeEventResult = NO;
    self.canBecomeFirstResponderFlag = NO;
    self.multipleTouchEnabled = YES;
    self.egLayer = (CAEAGLLayer*) super.layer;
    self.contentScaleFactor = [UIScreen mainScreen].scale;
    self.egLayer.opaque = YES;
    //here we configure the properties of our canvas, most important is the color depth RGBA8 !
    self.egLayer.drawableProperties = [NSDictionary dictionaryWithObjectsAndKeys:
                                    [NSNumber numberWithBool:FALSE], kEAGLDrawablePropertyRetainedBacking,
                                    kEAGLColorFormatRGBA8, kEAGLDrawablePropertyColorFormat,
                                    nil];

    displayLink = [CADisplayLink displayLinkWithTarget:self selector:@selector(drawFrame)];
    //by adding the display link to the run loop our draw method will be called 60 times per second
    [displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
    }
  return self;
  }

- (void)drawFrame {
  iOSAPI::swapContext();
  }

-(void) processEvent:(UIEvent *)event point:(CGPoint) p type:(Event::Type) type id:(size_t) id  {
  (void)event;
  int x=p.x,y=p.y;

  CGRect frame        = [self frame];
  const CGFloat scale = self.contentScaleFactor;
  
  state.eventType = type;

  if(type!=Event::NoEvent){
    new (&state.event.mouse)
      MouseEvent ( x*scale,
                   frame.size.height*scale-y*scale,
                   Event::ButtonLeft,
                   0,
                   id,
                   type
                   );
    iOSAPI::swapContext();
    }
  }

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event {
  for( UITouch *touch in touches ){
    CGPoint point = [touch locationInView:self];

    size_t id = state.addTouch(touch,point);
    if(id<state.touch.size())
      [self processEvent:event point:point type:Event::MouseDown id:id];
    }
  }

- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event {
  for( UITouch *touch in touches ){
    CGPoint point = [touch locationInView:self];

    size_t id = state.moveTouch(touch,point);
    if(id<state.touch.size())
      [self processEvent:event point:point type:Event::MouseMove id:id];
    }
  }

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event {
  for( UITouch *touch in touches ){
    CGPoint point = [touch locationInView:self];

    size_t id = state.delTouch(touch);
    if(id!=size_t(-1))
      [self processEvent:event point:point type:Event::MouseUp id:id];
    }
  }

- (void)insertText:(NSString *)text {
  const char *utf8text = [text UTF8String];

  char16_t txt16[32] = {};
  if( utf8text )
    utf8::unchecked::utf8to16(utf8text, utf8text+std::max<int>(32,strlen(utf8text)), txt16 );

  for(char16_t ch:txt16){
    if(ch=='\0')
      break;

    Event::KeyType kt = Event::K_NoKey;
    if(ch=='\n'){
      kt = Event::K_Return;
      ch = '\0';
      }

    state.eventType = Event::KeyDown;
    new (&state.event.key) Tempest::KeyEvent( kt, ch, state.eventType );
    iOSAPI::swapContext();

    state.eventType = Event::KeyUp;
    new (&state.event.key) Tempest::KeyEvent( kt, ch, state.eventType );
    iOSAPI::swapContext();
    }
  }

- (void)deleteBackward {
  state.eventType = Event::KeyDown;
  new (&state.event.key) Tempest::KeyEvent( Event::K_Back, 0, state.eventType );
  iOSAPI::swapContext();

  state.eventType = Event::KeyUp;
  new (&state.event.key) Tempest::KeyEvent( Event::K_Back, 0, state.eventType );
  iOSAPI::swapContext();
  }

- (BOOL)hasText {
  // Return whether there's any text present
  return YES;
  }

- (BOOL)canBecomeFirstResponder {
  return self.canBecomeFirstResponderFlag;
  }

@end

@interface TempestWindow : UIWindow {
  }
@property (nonatomic)         GLuint renderBuffer, depthBuffer, frameBuffer;
@property (nonatomic, retain) GLView *glView;
@end

@implementation TempestWindow


- (void)layoutSubviews {
  [super layoutSubviews];

  const CGFloat scale = self.contentScaleFactor;
  CGRect frame        = self.rootViewController.view.bounds;
  frame.origin.x      = 0;
  frame.origin.y      = 0;
  [self.glView setFrame: frame];

  state.eventType = Event::Resize;
  state.window    = (void*)self;

  new (&state.event.size) SizeEvent( frame.size.width*scale, frame.size.height*scale );
  iOSAPI::swapContext();
  }

@end

@interface ViewController:UIViewController{}
@end

@implementation ViewController
- (void)viewDidLoad {
  if( [self respondsToSelector:@selector(setNeedsStatusBarAppearanceUpdate)] ) {
    // iOS 7
    [self performSelector:@selector(setNeedsStatusBarAppearanceUpdate)];
    } else {
    // iOS 6
    [[UIApplication sharedApplication] setStatusBarHidden:YES withAnimation:UIStatusBarAnimationSlide];
    }
  }

- (BOOL)prefersStatusBarHidden {
  return YES;
  }

- (BOOL) shouldAutorotate {
  return YES;
  }

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
  (void)interfaceOrientation;
  return YES;
  }

-(UIInterfaceOrientationMask)supportedInterfaceOrientations {
  return  UIInterfaceOrientationMaskAll;
  }
@end

@interface AppDelegate : NSObject <UIApplicationDelegate> {}
  @property (strong, nonatomic) UIWindow *window;
@end
@implementation AppDelegate

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
  (void)application;
  (void)launchOptions;
  
  CGRect frame = [ [ UIScreen mainScreen ] bounds ];
  TempestWindow *window = [ [ TempestWindow alloc ] initWithFrame: frame];
  window.contentScaleFactor = [UIScreen mainScreen].scale;
  window.rootViewController = [ViewController new];
  window.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;

  window.backgroundColor = [ UIColor blackColor ];

  self.window      = window;
  state.rootWindow = window;

  [ window makeKeyAndVisible ]; // possible switch here
  Tempest::iOSAPI::swapContext();
  return YES;
  }

- (UIInterfaceOrientationMask)application:(UIApplication *)application
  supportedInterfaceOrientationsForWindow:(UIWindow *)window {
  (void)application;
  (void)window;
  return UIInterfaceOrientationMaskAll;
  }

- (void)applicationWillResignActive:(UIApplication *)application {
  (void)application;
  }


- (void)applicationDidEnterBackground:(UIApplication *)application {
  (void)application;
  state.eventType = Event::Type(EventMinimize);
  state.window    = nullptr;
  iOSAPI::swapContext();
  }


- (void)applicationWillEnterForeground:(UIApplication *)application {
  (void)application;
  }


- (void)applicationDidBecomeActive:(UIApplication *)application  {
  (void)application;
  state.eventType = Event::Type(EventDeMinimize);
  state.window    = nullptr;
  iOSAPI::swapContext();
  }


- (void)applicationWillTerminate:(UIApplication *)application {
  (void)application;
  }

@end

static id createWindow(Window::ShowMode mode){
  (void)mode;
  TempestWindow* window = (TempestWindow*)state.rootWindow;
  @try{
  window.backgroundColor = [ UIColor blackColor ];
  CGRect frame = window.frame;

  window.glView=[[GLView alloc] initWithFrame:CGRectMake(0, 0, frame.size.width, frame.size.height)];
  [ window.rootViewController.view addSubview: window.glView];
  }
  @catch (NSException * e) {
     NSLog(@"Exception: %@", e);
  }

  return window;
  }

static void render( Tempest::Window* w ){
  if( w->showMode()!=Tempest::Window::Minimized && w->isActive() )
    w->render();
  }

iOSAPI::iOSAPI(){
  NSString *ISO639_1LanguageCode = [[NSLocale currentLocale] objectForKey:NSLocaleLanguageCode];
  if( [[NSLocale preferredLanguages] count]>0 ) {
    ISO639_1LanguageCode = [[NSLocale preferredLanguages] objectAtIndex:0];
    }
  if(ISO639_1LanguageCode!=nil){
    const char *utf8 = [ISO639_1LanguageCode UTF8String];
    if(utf8){
      for(auto& l:lng){
        if(strcmp(l.first,utf8)==0)
          state.locale = l.second;
        }
      }
    }
  if(state.locale.size()==0)
    state.locale="eng";
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
  CGRect rect=[ [ UIScreen mainScreen ] bounds ];
  return Size(rect.size.width,rect.size.height);
  }

static void appleMain(void*){
  state.touch.reserve(16);

  static std::string app="application";
  char * argv[2] = {
    &app[0],nullptr
    };
  UIApplicationMain( 1, argv, nil, NSStringFromClass( [ AppDelegate class ] ) );
  }

void iOSAPI::startApplication(SystemAPI::ApplicationInitArgs *) {
  createAppleSubContext();
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

      //WM_CHAR
      uint32_t key = state.event.key.u16;
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
    case Event::KeyUp:{
      Tempest::KeyEvent k = KeyEvent(state.event.key.key,Event::Type(type));
      SystemAPI::emitEvent( w,k,k,Event::Type(type) );
      }
      break;
    case Event::Resize:{
      const int width=state.event.size.w, height=state.event.size.h;
      SystemAPI::activateEvent(w,true);
      SystemAPI::sizeEvent( w, width,height );
      }
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
      SystemAPI::activateEvent(w,false);
      SystemAPI::setShowMode( w, Tempest::Window::Minimized);
      }
      break;
    case EventDeMinimize:{
      Tempest::Window::ShowMode mode=Tempest::Window::FullScreen;
      SystemAPI::activateEvent(w,true);
      SystemAPI::setShowMode( w, mode);
      SystemAPI::sizeEvent( w, w->w(), w->h() );
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

SystemAPI::Window* iOSAPI::createWindow(int /*w*/, int /*h*/) {
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

Tempest::Point iOSAPI::windowClientPos(SystemAPI::Window* w) {
  TempestWindow* window = (TempestWindow*)w;
  UIView*        view   = window.glView;
  CGRect         frame  = view.frame;
  return Point(frame.origin.x,frame.origin.y);
  }

Tempest::Size iOSAPI::windowClientRect(SystemAPI::Window* w) {
  TempestWindow* window = (TempestWindow*)w;
  UIView*        view   = window.glView;
  CGRect         frame  = view.frame;
  const CGFloat  scale  = view.contentScaleFactor;
  return Size(frame.size.width*scale,frame.size.height*scale);
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
  if(fname && fname[0]=='/')
    return SystemAPI::fopenImpl( fname, mode );

  NSString *dir = [[NSBundle mainBundle] resourcePath];
  std::string full = [dir UTF8String];
  full += "/";
  full += fname;
  return SystemAPI::fopenImpl( full.c_str(), mode );
  }

SystemAPI::File *iOSAPI::fopenImpl(const char16_t *fname, const char *mode) {
  return fopenImpl( SystemAPI::toUtf8(fname).c_str(), mode );
  }

void iOSAPI::createFramebuffers(void* wnd, void* ctx) {
  TempestWindow* window = (TempestWindow*)wnd;
  EAGLContext* openGLContext = (EAGLContext*)ctx;

  const bool useDepthBuffer=true;

  GLint  framebufferWidth=1, framebufferHeight=1;
  GLuint renderBuffer=0, depthBuffer=0;

  id layer=window.glView.egLayer;
  glGenRenderbuffers(1, &renderBuffer);
  glBindRenderbuffer(GL_RENDERBUFFER, renderBuffer);
  [openGLContext renderbufferStorage:GL_RENDERBUFFER fromDrawable:layer];

  glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &framebufferWidth);
  glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &framebufferHeight);

  GLuint framebuffer=0;
  glGenFramebuffers(1, &framebuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, renderBuffer);

  if(useDepthBuffer) {
    //create a depth renderbuffer
    glGenRenderbuffers(1, &depthBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
    //create the storage for the buffer, optimized for depth values, same size as the colorRenderbuffer
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, framebufferWidth, framebufferHeight);

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);
    }
  T_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

  //TODO: cleanup
  window.renderBuffer = renderBuffer;
  window.depthBuffer  = depthBuffer;
  window.frameBuffer  = framebuffer;
  }

void* iOSAPI::initializeOpengl(void* wnd) {
  TempestWindow* window = (TempestWindow*)wnd;

  EAGLContext* openGLContext = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];

  // Make the context the current context.
  if(![EAGLContext setCurrentContext:openGLContext])
    return nullptr;
  createFramebuffers(window,openGLContext);

  return openGLContext;
  }

bool iOSAPI::glMakeCurrent(void *ctx) {
  //[(id)ctx update];
  [EAGLContext setCurrentContext:(id)ctx];
  return true;
  }

bool iOSAPI::glUpdateContext(void* ctx, void* wnd) {
  TempestWindow* window = (TempestWindow*)wnd;
  if(![EAGLContext setCurrentContext:(id)ctx])
    return false;

  if(window.frameBuffer){
    GLuint buffer=window.frameBuffer;
    glDeleteFramebuffers(1, &buffer);
    window.frameBuffer=0;
    }

  if(window.renderBuffer){
    GLuint buffer=window.renderBuffer;
    glDeleteRenderbuffers(1, &buffer);
    window.renderBuffer=0;
    }

  if(window.depthBuffer){
    GLuint buffer=window.depthBuffer;
    glDeleteRenderbuffers(1, &buffer);
    window.depthBuffer=0;
    }

  createFramebuffers(window,ctx);
  return true;
  }

void iOSAPI::glBindZeroFramebuffer(void* wnd) {
  TempestWindow* window = (TempestWindow*)wnd;
  glBindFramebuffer(GL_FRAMEBUFFER, window.frameBuffer);
  }

void iOSAPI::glSwapBuffers(void* wnd, void *ctx) {
  TempestWindow* window = (TempestWindow*)wnd;
  GLuint buffer=window.renderBuffer;
  glBindRenderbuffer(GL_RENDERBUFFER,buffer);
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

static bool isIpadMini() {
  utsname systemInfo={};
  if( uname(&systemInfo)<0 )
    return false; // no idea what is it

  if(   strcmp(systemInfo.machine,"iPad2,5")==0
     || strcmp(systemInfo.machine,"iPad2,6")==0
     || strcmp(systemInfo.machine,"iPad2,7")==0 )
    return true;
  return false;
  }

float iOSAPI::densityDpi() {
  float scale = 1;

  if( [[UIScreen mainScreen] respondsToSelector:@selector(scale)] )
    scale = [[UIScreen mainScreen] scale];
  return scale;

  float dpi;
  if( UI_USER_INTERFACE_IDIOM()==UIUserInterfaceIdiomPad )
    dpi = isIpadMini() ? 163 : 132;
  else if( UI_USER_INTERFACE_IDIOM()==UIUserInterfaceIdiomPhone )
    dpi = 163;
  else
    dpi = 160;

  return int(dpi*scale*scale);
  }

const std::string& iOSAPI::iso3Locale() {
  return state.locale;
  }

void iOSAPI::showSoftInput() {
  if(wndWx.size()==0)
    return;

  id wi = wndWx.begin()->first;

  TempestWindow* window = (TempestWindow*)wi;
  window.glView.canBecomeFirstResponderFlag = YES;
  [window.glView becomeFirstResponder];
  }

void iOSAPI::hideSoftInput() {
  if(wndWx.size()==0)
    return;

  id wi = wndWx.begin()->first;

  TempestWindow* window = (TempestWindow*)wi;
  window.glView.canBecomeFirstResponderFlag = NO;
  [window.glView resignFirstResponder];
  }

void iOSAPI::toggleSoftInput() {
  if(wndWx.size()==0)
    return;

  id wi = wndWx.begin()->first;

  TempestWindow* window = (TempestWindow*)wi;

  if(window.glView.canBecomeFirstResponderFlag==YES)
    hideSoftInput(); else
    showSoftInput();
  }

iOSAPI::InterfaceIdiom iOSAPI::interfaceIdiom() {
  if( UI_USER_INTERFACE_IDIOM()==UIUserInterfaceIdiomPhone )
    return InterfaceIdiomPhone;
  if( UI_USER_INTERFACE_IDIOM()==UIUserInterfaceIdiomPad )
    return InterfaceIdiomPad;
  if( UI_USER_INTERFACE_IDIOM()==UIUserInterfaceIdiomTV )
    return InterfaceIdiomTV;

  return InterfaceIdiomPad; // ???
  }

#endif
