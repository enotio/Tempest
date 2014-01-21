#include "androidapi.h"

using namespace Tempest;

#ifdef __ANDROID__

#include <Tempest/Application>
#include <Tempest/Window>
#include <Tempest/Event>
#include <Tempest/Log>
#include <map>
#include <queue>
#include <Tempest/Assert>
#include <Tempest/DisplaySettings>

#include <EGL/egl.h>
#include <GLES/gl.h>

#include <pthread.h>
#include <unistd.h>
#include <sys/stat.h>

#include <jni.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

#include <android/bitmap.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>

#include <android/keycodes.h>
#include <machine/cpu-features.h>

#include <cmath>
#include <locale>

struct Guard {
  Guard( pthread_mutex_t& m ):m(m){
    pthread_mutex_lock(&m);
    }

  ~Guard(){
    pthread_mutex_unlock(&m);
    }

  pthread_mutex_t& m;
  };

static struct Android{
  EGLDisplay display;
  EGLSurface surface;
  EGLContext context;

  int window_w, window_h;
  Tempest::Window * volatile wnd;

  JavaVM  *vm;
  JNIEnv  *env;
  jobject assets;
  jclass  tempestClass;

  ANativeWindow   * volatile window;
  pthread_mutex_t appMutex, waitMutex, assetsPreloadedMutex;
  volatile AndroidAPI::GraphicsContexState graphicsState;

  pthread_t mainThread;

  bool forceToResize;
  bool isWndAviable;
  bool isPaused;

  std::vector<char> internal, external;
  std::string locale;
  int densityDpi;

  enum MainThreadMessage {
    MSG_NONE = 0,
    MSG_WINDOW_SET,
    MSG_SURFACE_RESIZE,
    MSG_TOUCH,
    MSG_PAUSE,
    MSG_RESUME,
    MSG_KEY_EVENT,
    MSG_CLOSE,
    MSG_WAIT,
    MSG_RENDER_LOOP_EXIT
    };

  struct Message{
    Message( MainThreadMessage m ):msg(m){
      data.x = 0;
      data.y = 0;
      data1  = 0;
      data2  = 0;
      }

    union{
      struct {
        int x,y;
        };

      struct {
        int w,h;
        };
      } data;

    int data1, data2;
    MainThreadMessage msg;
    };

  std::vector<Message> msg;

  void pushMsg( const Message& m ){
    Guard g( appMutex );
    (void)g;
    msg.push_back(m);
    }

  size_t msgSize(){
    Guard g( appMutex );
    (void)g;
    return msg.size();
    }

  Message takeMsg(){
    Guard g( appMutex );
    (void)g;
    if(msg.size())
      return msg[0];

    return MSG_NONE;
    }

  struct MousePointer{
    int   nativeID;
    bool  valid;
    Point pos;
    };
  std::vector<MousePointer> mouse;

  std::unordered_map< std::string, std::unique_ptr< std::vector<char>> > asset_files;

  std::vector<char>& preloadAsset( const std::string& ass ){
    Guard guard(assetsPreloadedMutex);
    (void)guard;

    auto i = asset_files.find(ass);
    if( i!=asset_files.end() ){
      return *i->second;
      }

    JNIEnv * env = 0;
    vm->AttachCurrentThread( &env, NULL);

    AAssetManager* mgr = AAssetManager_fromJava(env, assets);
    AAsset* asset = AAssetManager_open(mgr, ass.c_str(), AASSET_MODE_UNKNOWN);

    if( !asset )
      Log(Log::Error) << "not found: \"" << ass <<'\"';

    T_ASSERT( asset );

    std::unique_ptr< std::vector<char>> vec(new std::vector<char>(AAsset_getLength(asset)));
    AAsset_read (asset, &((*vec)[0]), vec->size() );
    AAsset_close(asset);

    asset_files[ass] = std::move(vec);
    return *asset_files[ass];
    }

  int pointerId( int nid ){
    for( size_t i=0; i<mouse.size(); ++i )
      if( mouse[i].nativeID==nid && mouse[i].valid ){
        return i;
        }

    MousePointer p;
    p.nativeID = nid;
    p.valid    = true;
    mouse.push_back(p);
    return mouse.size()-1;
    }

  Point& pointerPos( int id ){
    return mouse[id].pos;
    }

  void unsetPointer( int nid ){
    for( size_t i=0; i<mouse.size(); ++i )
      if( mouse[i].nativeID==nid ){
        mouse[i].valid = false;
        }

    while( mouse.size() && !mouse.back().valid )
      mouse.pop_back();
    }

  Android(){
    display = EGL_NO_DISPLAY;
    surface = 0;
    context = 0;

    env        = 0;
    densityDpi = 480;

    graphicsState = SystemAPI::NotAviable;
    window     = 0;
    window_w = 0;
    window_h = 0;
    wnd           = 0;
    forceToResize = 0;

    isWndAviable = false;
    isPaused     = true;

    mainThread = 0;
    msg.  reserve(32);
    mouse.reserve(8);

    pthread_mutex_init(&waitMutex, 0);
    pthread_mutex_init(&appMutex, 0);
    pthread_mutex_init(&assetsPreloadedMutex, 0);
    }

  ~Android(){
    pthread_mutex_destroy(&assetsPreloadedMutex);
    pthread_mutex_destroy(&appMutex);
    pthread_mutex_destroy(&waitMutex);
    }

  bool initialize();
  void destroy(bool killContext);

  void waitForQueue();
  } android;

struct SyncMethod{
  SyncMethod():g(android.waitMutex){

    }

  ~SyncMethod(){
    android.waitForQueue();
    }

  Guard g;
  };

using namespace Tempest;

AndroidAPI::AndroidAPI(){
  TranslateKeyPair k[] = {
    { AKEYCODE_DPAD_LEFT,   Event::K_Left   },
    { AKEYCODE_DPAD_RIGHT,  Event::K_Right  },
    { AKEYCODE_DPAD_UP,     Event::K_Up     },
    { AKEYCODE_DPAD_DOWN,   Event::K_Down   },

    //{ AKEYCODE_BACK, Event::K_ESCAPE },
    { AKEYCODE_BACK, Event::K_Back   },
    { AKEYCODE_DEL,  Event::K_Delete },
    { AKEYCODE_HOME, Event::K_Home   },

    { AKEYCODE_0,      Event::K_0  },
    { AKEYCODE_A,      Event::K_A  },

    { 0,         Event::K_NoKey }
    };

  setupKeyTranslate(k);
  setFuncKeysCount(0);
  }

AndroidAPI::~AndroidAPI(){

  }

JavaVM *AndroidAPI::jvm() {
  return android.vm;
  }

JNIEnv *AndroidAPI::jenvi() {
  return android.env;
  }

jclass AndroidAPI::appClass() {
  return android.tempestClass;
  }

const char *AndroidAPI::internalStorage() {
  mkdir( android.internal.data(), 0770 );
  return android.internal.data();
  }

const char *AndroidAPI::externalStorage() {
  mkdir( android.external.data(), 0770 );
  return android.external.data();
  }

int AndroidAPI::densityDpi(){
  return android.densityDpi;
  }

void AndroidAPI::toast( const std::string &s ) {
  Android &a = android;

  jclass clazz    = a.tempestClass;
  jmethodID toast = a.env->GetStaticMethodID( clazz, "showToast", "(Ljava/lang/String;)V");

  jstring str = a.env->NewStringUTF( s.c_str() );
  a.env->CallStaticVoidMethod(clazz, toast, str);

  a.env->DeleteLocalRef(str);
  }

const std::string &AndroidAPI::iso3Locale() {
  return android.locale;
  }

AndroidAPI::Window *AndroidAPI::createWindow(int /*w*/, int /*h*/) {
  return 0;
  }

AndroidAPI::Window* AndroidAPI::createWindowMaximized(){
  return 0;
  }

AndroidAPI::Window* AndroidAPI::createWindowMinimized(){
  return 0;
  }

AndroidAPI::Window* AndroidAPI::createWindowFullScr(){
  return 0;
  }

Widget *AndroidAPI::addOverlay(WindowOverlay *ov) {
  if( android.wnd==0 ){
    delete ov;
    return 0;
    }

  SystemAPI::addOverlay(android.wnd, ov);
  return ov;
  }

bool AndroidAPI::testDisplaySettings( const DisplaySettings & s ) {
  return s.width ==android.window_w  &&
         s.height==android.window_h &&
         s.bits==16 &&
         s.fullScreen;
  }

bool AndroidAPI::setDisplaySettings(const DisplaySettings &s) {
  return testDisplaySettings(s);
  }

AndroidAPI::CpuInfo AndroidAPI::cpuInfoImpl(){
  CpuInfo info;
  memset(&info, 0, sizeof(info));

  info.cpuCount = 1;//android_getCpuCount();
  return info;
  }

struct AndroidAPI::DroidFile{
  FILE* h;
  char * assets_ptr;
  char * pos;
  size_t size;
  };

AndroidAPI::File *AndroidAPI::fopenImpl( const char16_t *fname, const char *mode ) {
  return fopenImpl( toUtf8(fname).data(), mode );
  }

AndroidAPI::File *AndroidAPI::fopenImpl( const char *fname, const char *mode ) {
  bool wr = 0;
  for( int i=0; mode[i]; ++i ){
    if( mode[i]=='w' )
      wr = 1;
    }

  DroidFile* f = new DroidFile();
  if( fname[0]=='/'||fname[0]=='.' ){
    f->h = ::fopen(fname, mode);
    f->assets_ptr = 0;
    } else
  if( !wr ) {
    f->h = 0;
    std::vector<char>& vec = android.preloadAsset(fname);
    f->size       = vec.size();
    f->assets_ptr = &vec[0];
    f->pos        = f->assets_ptr;
    }

  if( !f->h && !f->assets_ptr ){
    delete f;
    return 0;
    }

  return (SystemAPI::File*)f;
  }

size_t AndroidAPI::readDataImpl(SystemAPI::File *f, char *dest, size_t count) {
  DroidFile *fn = (DroidFile*)f;
  if( fn->h ){
    return fread(dest, 1, count, fn->h);
    } else {
    size_t c = std::min(fn->size, count);
    memcpy(dest, fn->pos, c);
    fn->pos  += c;
    fn->size -= c;
    return c;
    }
  }

size_t AndroidAPI::peekImpl(SystemAPI::File *f, size_t skip, char *dest, size_t count) {
  DroidFile *fn = (DroidFile*)f;
  if( fn->h ){
    size_t pos = ftell( fn->h );
    fseek( fn->h, skip, SEEK_CUR );
    size_t c = fread( dest, 1, count, fn->h );
    fseek( fn->h , pos, SEEK_SET );
    return c;
    } else {
    if( skip>=fn->size )
      return 0;

    size_t c = std::min(fn->size-skip, count);
    memcpy(dest, fn->pos+skip, c);
    return c;
    }
  }

size_t AndroidAPI::writeDataImpl(SystemAPI::File *f, const char *data, size_t count) {
  DroidFile *fn = (DroidFile*)f;
  return fwrite(data, 1, count, fn->h );
  }

void AndroidAPI::flushImpl(SystemAPI::File *f) {
  DroidFile *fn = (DroidFile*)f;
  if( fn->h )
    fflush( fn->h );
  }

size_t AndroidAPI::skipImpl(SystemAPI::File *f, size_t count) {
  DroidFile *fn = (DroidFile*)f;

  if( fn->h ){
    size_t pos = ftell( fn->h );
    fseek( fn->h, count, SEEK_CUR );

    return ftell( fn->h ) - pos;
    } else {
    size_t c = std::min(fn->size, count);

    fn->pos  += c;
    fn->size -= c;
    return c;
    }
  }

bool AndroidAPI::eofImpl(SystemAPI::File *f) {
  DroidFile *fn = (DroidFile*)f;
  if( fn->h ){
    return feof( fn->h );
    }

  return fn->size==0;
  }

size_t AndroidAPI::fsizeImpl( File* f ){
  DroidFile *fn = (DroidFile*)f;
  if( fn->h ){
    FILE *file = fn->h;
    size_t pos = ftell(file);

    fseek( file, 0, SEEK_SET );
    size_t s = ftell(file);

    fseek( file, 0, SEEK_END );
    size_t e = ftell(file);

    fseek( file, pos, SEEK_SET );
    return e-s;
    }

  return fn->size + size_t(fn->pos - fn->assets_ptr);
  }

void AndroidAPI::fcloseImpl(SystemAPI::File *f) {
  DroidFile *fn = (DroidFile*)f;

  if( fn->h ){
    ::fclose( fn->h );
    }

  delete fn;
  }

Size AndroidAPI::implScreenSize() {
  return Size(android.window_w, android.window_h);
  }

bool AndroidAPI::startRender( Window * ) {
  return 1;
  }

bool AndroidAPI::present(SystemAPI::Window *) {
  return 0;
  }

void AndroidAPI::startApplication( ApplicationInitArgs * ) {
  }

void AndroidAPI::endApplication() {
  }

static Tempest::KeyEvent makeKeyEvent( int32_t k, bool scut = false ){
  Tempest::KeyEvent::KeyType e = SystemAPI::translateKey(k);
  if( !scut ){
    if( Event::K_0<=e && e<= Event::K_9 )
      e = Tempest::KeyEvent::K_NoKey;

    if( Event::K_A<=e && e<= Event::K_Z )
      e = Tempest::KeyEvent::K_NoKey;
    }
  return Tempest::KeyEvent( e );
  }

static void render();

void Android::waitForQueue() {
  while( mainThread && android.window ){
    size_t s = android.msgSize();
    if( s==0 )
      return;
    }
  }

int AndroidAPI::nextEvents(bool &quit) {
  int r = 0;
  while( !quit ){
    int sz = 0;
    {
      Android::MainThreadMessage msg = android.takeMsg().msg;

      if( android.window==0 && msg!=Android::MSG_RENDER_LOOP_EXIT  ){
        return 1;
        }

      sz = msg!=Android::MSG_NONE;
      }

    r = nextEvent(quit);
    if( sz==0 )
      return r;
    }

  return r;
  }

int AndroidAPI::nextEvent(bool &quit) {  
  /*
  static const int ACTION_DOWN = 0,
                   ACTION_UP   = 1,
                   ACTION_MOVE = 2;
  */

  bool hasWindow = true;
  Android::Message msg = Android::MSG_NONE;
  {
  Guard g(android.appMutex);
  (void)g;
  hasWindow = android.window;

  if( android.msg.size() ){
    msg = android.msg[0];

    if( !hasWindow && msg.msg!=Android::MSG_RENDER_LOOP_EXIT ){
      return 0;
      }

    android.msg.erase( android.msg.begin() );
    } else {
    if( !android.isPaused && hasWindow )
      render();
    return 0;
    }
  }

  switch( msg.msg ) {
    case Android::MSG_SURFACE_RESIZE:
      break;

    case Android::MSG_RENDER_LOOP_EXIT:
      quit = true;
      break;

    case Android::MSG_KEY_EVENT:{
      Tempest::KeyEvent sce = makeKeyEvent(msg.data1, true);

      Tempest::KeyEvent sc( sce.key, sce.u16, Event::Shortcut );
      SystemAPI::emitEvent( android.wnd, sc );

      if( !sc.isAccepted() ){
        Tempest::KeyEvent e =  makeKeyEvent(msg.data1);
        if( e.key!=Tempest::KeyEvent::K_NoKey ){
          Tempest::KeyEvent ed( e.key, e.u16, Event::KeyDown );
          SystemAPI::emitEvent( android.wnd, ed);

          Tempest::KeyEvent eu( e.key, e.u16, Event::KeyUp );
          SystemAPI::emitEvent( android.wnd, eu);
          }
        }
      }
      break;

    case Android::MSG_TOUCH: {
      int id = android.pointerId( msg.data2 );

      if( msg.data1==0 ){
        MouseEvent e( msg.data.x, msg.data.y,
                      MouseEvent::ButtonLeft, 0, id,
                      Event::MouseDown );
        android.pointerPos(id) = Point(msg.data.x, msg.data.y);
        SystemAPI::emitEvent(android.wnd, e);
        }

      if( msg.data1==1 ){
        MouseEvent e( msg.data.x, msg.data.y,
                      MouseEvent::ButtonLeft, 0, id,
                      Event::MouseUp );
        SystemAPI::emitEvent(android.wnd, e);
        android.unsetPointer(msg.data2);
        }

      if( msg.data1==2 ){
        MouseEvent e( msg.data.x, msg.data.y,
                      MouseEvent::ButtonLeft, 0, id,
                      Event::MouseMove );
        if( android.pointerPos(id) != Point(msg.data.x, msg.data.y) ){
          android.pointerPos(id) = Point(msg.data.x, msg.data.y);
          SystemAPI::emitEvent(android.wnd, e);
          }
        }
      }
      break;

    case Android::MSG_CLOSE:  {
      Tempest::CloseEvent e;
      SystemAPI::emitEvent(android.wnd, e);
      if( !e.isAccepted() ){
        android.pushMsg( Android::MSG_RENDER_LOOP_EXIT );
        }
      }
      break;

    case Android::MSG_PAUSE:
      android.destroy(false);
      break;

    case Android::MSG_RESUME:
      android.initialize();
      break;

    case Android::MSG_NONE:
      if( !android.isPaused && hasWindow )
        render();
      break;

    default:
      break;
    }

  return 0;
  }

Point AndroidAPI::windowClientPos ( Window* ){
  return Point(0,0);
  }

Size AndroidAPI::windowClientRect( Window* ){
  Android &a = android;
  return Size(a.window_w, a.window_h);
  }

void AndroidAPI::deleteWindow( Window */*w*/ ) {
  android.wnd = 0;
  }

void AndroidAPI::show(Window *) {
  if( android.wnd && android.display!=EGL_NO_DISPLAY ){
    glViewport( 0, 0, android.window_w, android.window_h );
    if( android.wnd )
      SystemAPI::sizeEvent( android.wnd, android.window_w, android.window_h );
    android.isWndAviable = true;
    }
  }

void AndroidAPI::setGeometry( Window */*hw*/, int /*x*/, int /*y*/, int /*w*/, int /*h*/ ) {
  }

void AndroidAPI::bind( Window *, Tempest::Window *wx ) {
  android.wnd = wx;
  SystemAPI::activateEvent( android.wnd, android.window!=0);
  }

AndroidAPI::GraphicsContexState AndroidAPI::isGraphicsContextAviable( Tempest::Window * ){
  return android.graphicsState;
  //return android.window;
  }

static void render() {
  Android& e = android;

  if( e.display!= EGL_NO_DISPLAY && e.wnd &&
      android.graphicsState != SystemAPI::DestroyedByAndroid){
    int w = e.window_w,
        h = e.window_h;

    eglQuerySurface(e.display, e.surface, EGL_WIDTH,  &w);
    eglQuerySurface(e.display, e.surface, EGL_HEIGHT, &h);

    if( e.forceToResize ||
        (e.window_w!=w || e.window_h!=h) ){
      e.window_w = w;
      e.window_h = h;

      if( e.forceToResize  ){
        SizeEvent s(w,h);
        e.wnd->resizeEvent(s);
        e.wnd->resize(w,h);
        e.forceToResize = 0;
        } else {
        e.wnd->resize( w,h );
        }
      }

    e.wnd->render();
    }
  }

bool Android::initialize() {
  Android* e = this;

  if( display != EGL_NO_DISPLAY )
    return true;

  static const EGLint attribs16[] = {
    EGL_SURFACE_TYPE,
    EGL_WINDOW_BIT,
    EGL_BLUE_SIZE,    5,
    EGL_GREEN_SIZE,   6,
    EGL_RED_SIZE,     5,
    EGL_ALPHA_SIZE,   0,
    EGL_DEPTH_SIZE,   16,
    EGL_STENCIL_SIZE, 0,
    EGL_NONE
  };

  /*
  static const EGLint attribs32[] = {
    EGL_SURFACE_TYPE,
    EGL_WINDOW_BIT,
    EGL_BLUE_SIZE,    8,
    EGL_GREEN_SIZE,   8,
    EGL_RED_SIZE,     8,
    EGL_ALPHA_SIZE,   8,
    EGL_DEPTH_SIZE,   16,
    EGL_STENCIL_SIZE, 0,
    EGL_NONE
  };*/

  const EGLint* attribs = attribs16;

  EGLint w, h, format;
  EGLint numConfigs;
  EGLConfig  config;
  EGLSurface ini_surface;
  EGLContext ini_context;

  EGLDisplay ini_display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

  eglInitialize(ini_display, 0, 0);
  eglChooseConfig(ini_display, attribs, &config, 1, &numConfigs);
  eglGetConfigAttrib(ini_display, config, EGL_NATIVE_VISUAL_ID, &format);

  ANativeWindow_setBuffersGeometry( window, 0, 0, format);

  ini_surface = eglCreateWindowSurface( ini_display, config, window, NULL);

  if( e->context==EGL_NO_CONTEXT ){
    const EGLint attrib_list[] = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE};
    ini_context = eglCreateContext(ini_display, config, NULL, attrib_list);
    } else {
    ini_context = e->context;
    }

  if( eglMakeCurrent(ini_display, ini_surface, ini_surface, ini_context) == EGL_FALSE ){
    Log(Log::Error) << "Unable to eglMakeCurrent";
    return false;
    }

  eglQuerySurface(ini_display, ini_surface, EGL_WIDTH,  &w);
  eglQuerySurface(ini_display, ini_surface, EGL_HEIGHT, &h);

  e->display = ini_display;
  e->context = ini_context;
  e->surface = ini_surface;

  e->window_w = w;
  e->window_h = h;

  if( e->wnd && e->display != EGL_NO_DISPLAY ){
    e->forceToResize = true;
    }

  glViewport(0, 0, w, h);
  graphicsState = SystemAPI::Aviable;

  return true;
  }

void Android::destroy( bool killContext ) {
  Log(Log::Info) << "Destroying context";

  if( graphicsState == SystemAPI::DestroyedByAndroid )
    return;

  graphicsState = SystemAPI::NotAviable;

  if( display != EGL_NO_DISPLAY ) {
    eglMakeCurrent( display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT );

    if( context != EGL_NO_CONTEXT && killContext ){
      eglDestroyContext( display, context );
      }

    if( surface != EGL_NO_SURFACE ){
      eglDestroySurface( display, surface );
      }

    if( killContext )
      eglTerminate(display);
    }

  display = EGL_NO_DISPLAY;
  if( killContext )
    context = EGL_NO_CONTEXT;
  surface = EGL_NO_SURFACE;
  }

static void* tempestMainFunc(void*);

static void resize( JNIEnv * , jobject, jobject, jint w, jint h ) {
  SyncMethod wm;
  (void)wm;

  Android::Message m = Android::MSG_SURFACE_RESIZE;
  m.data.w = w;
  m.data.h = h;
  android.pushMsg(m);
  }

static void JNICALL start(JNIEnv* /*jenv*/, jobject /*obj*/) {
  Log(Log::Info) << "nativeOnStart";
  }

static void JNICALL stop(JNIEnv* /*jenv*/, jobject /*obj*/) {
  Log(Log::Info) << "nativeOnStop";
  }

static void JNICALL resume(JNIEnv* /*jenv*/, jobject /*obj*/) {
  Log(Log::Info) << "nativeOnResume";
  SyncMethod wm;
  (void)wm;

  android.pushMsg( Android::MSG_RESUME );
  android.isPaused = false;
  }

static void JNICALL pauseA(JNIEnv* /*jenv*/, jobject /*obj*/) {
  Log(Log::Info) << "nativeOnPause";
  SyncMethod wm;
  (void)wm;

  android.msg.push_back( Android::MSG_PAUSE );
  android.isPaused = true;
  }

static void JNICALL setAssets(JNIEnv* jenv, jobject /*obj*/, jobject assets ) {
  Log(Log::Info) << "setAssets";
  android.assets = jenv->NewGlobalRef(assets);
  }

static void JNICALL nativeOnTouch( JNIEnv* , jobject ,
                                   jint x, jint y, jint act, jint pid ){
  Android::Message m = Android::MSG_TOUCH;
  m.data.x = x;
  m.data.y = y;
  m.data1  = act;
  m.data2  = pid;

  android.pushMsg(m);
  }

static void JNICALL nativeSetSurface( JNIEnv* jenv, jobject /*obj*/, jobject surface) {
  if( surface ) {
    android.window = ANativeWindow_fromSurface(jenv, surface);
    android.graphicsState = SystemAPI::Aviable;
    Log(Log::Info) << "Got window " << android.window;

    {
      Guard g( android.appMutex );
      (void)g;
      android.msg.push_back( Android::MSG_WINDOW_SET );
      if( android.wnd )
        SystemAPI::activateEvent( android.wnd, android.window!=0);
      }

    if( android.mainThread==0 )
      pthread_create(&android.mainThread, 0, tempestMainFunc, 0);    
    } else {
    Log(Log::Info) << "Releasing window";
    {
      Guard g( android.appMutex );
      (void)g;

      ANativeWindow* wx = android.window;
      ANativeWindow_release(wx);
      android.graphicsState = SystemAPI::DestroyedByAndroid;
      android.window = 0;
      if( android.wnd )
        SystemAPI::activateEvent( android.wnd, android.window!=0);
      }
    }

  return;
  }

static void JNICALL onCreate(JNIEnv* , jobject ) {
  Log(Log::Info) << "nativeOnCreate";
  android.mainThread = 0;
  }

static void JNICALL onDestroy(JNIEnv* /*jenv*/, jobject /*obj*/) {
  {
  Guard g( android.appMutex );
  (void)g;
  android.msg.clear();
  android.msg.push_back( Android::MSG_RENDER_LOOP_EXIT );
  }

  pthread_join(android.mainThread,0);

  android.msg.clear();

  Log(Log::Info) << "nativeOnDestroy";
  }

static void JNICALL onKeyEvent(JNIEnv* , jobject, jint key ) {
  Log(Log::Info) << "onKeyEvent";

  Guard w( android.waitMutex );
  (void)w;

  if( key!=AKEYCODE_BACK ){
    Android::Message m = Android::MSG_KEY_EVENT;
    m.data1 = key;
    android.pushMsg(m);
    } else {
    android.pushMsg( Android::MSG_RENDER_LOOP_EXIT );
    }
  }

static jint JNICALL nativeCloseEvent( JNIEnv* , jobject ){
  android.pushMsg( Android::MSG_CLOSE );
  return 1;
  }

static void JNICALL nativeSetupStorage( JNIEnv* , jobject,
                                        jstring internal, jstring external ) {
  JNIEnv * env = 0;
  android.vm->AttachCurrentThread( &env, NULL);

  android.internal.clear();
  android.external.clear();

  const char* str = 0;

  str = env->GetStringUTFChars( internal, 0);
  if( str ){
    for( int i=0; str[i]; ++i )
      android.internal.push_back(str[i]);
    android.internal.push_back('/');
    android.internal.push_back(0);
    }
  env->ReleaseStringUTFChars( internal, str );

  str = env->GetStringUTFChars( external, 0);
  if( str ){
    for( int i=0; str[i]; ++i )
      android.external.push_back(str[i]);
    android.external.push_back('/');
    android.external.push_back(0);
    }
  env->ReleaseStringUTFChars( external, str );
  }


static void JNICALL nativeInitLocale( JNIEnv* , jobject,
                                      jstring loc ){
  JNIEnv * env = 0;
  android.vm->AttachCurrentThread( &env, NULL);

  const char* str = env->GetStringUTFChars( loc, 0);
  if( str ){
    android.locale = str;
    } else {
    android.locale = "eng";
    }

  env->ReleaseStringUTFChars( loc, str );
  }

static void setupDpi( JNIEnv* , jobject,
                      jint d ){
  android.densityDpi = d;
  }

jint JNI_OnLoad(JavaVM *vm, void */*reserved*/){
  Log(Log::Info) << "Tempest JNI_OnLoad";

  static JNINativeMethod methodTable[] = {
    {"nativeOnCreate",  "()V", (void *) onCreate  },
    {"nativeOnDestroy", "()V", (void *) onDestroy },

    {"nativeOnStart",  "()V", (void *) start  },
    {"nativeOnResume", "()V", (void *) resume },
    {"nativeOnPause",  "()V", (void *) pauseA },
    {"nativeOnStop",   "()V", (void *) stop   },

    {"nativeInitLocale", "(Ljava/lang/String;)V", (void *)nativeInitLocale },
    {"nativeSetupDpi",   "(I)V",                  (void *)setupDpi         },

    {"nativeOnTouch",  "(IIII)V", (void *)nativeOnTouch   },
    {"onKeyEvent",     "(I)V",   (void *)onKeyEvent      },
    {"nativeCloseEvent",  "()I", (void *) nativeCloseEvent  },
    {"nativeSetSurface", "(Landroid/view/Surface;)V",   (void *) nativeSetSurface   },
    {"nativeOnResize",   "(Landroid/view/Surface;II)V", (void *) resize             },
    {"nativeSetAssets",  "(Landroid/content/res/AssetManager;)V",   (void *) setAssets          },

    {"nativeSetupStorage",  "(Ljava/lang/String;Ljava/lang/String;)V",  (void *)nativeSetupStorage }
  };

  android.vm = vm;

  JNIEnv* env;
  if( vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK ){
    Log(Log::Error) << "Failed to get the environment";
    return -1;
    }

  android.tempestClass = env->FindClass( SystemAPI::androidActivityClass().c_str() );
  android.tempestClass = (jclass)env->NewGlobalRef(android.tempestClass);

  if (!android.tempestClass) {
    Log(Log::Error) << "failed to get " << SystemAPI::androidActivityClass().c_str() << " class reference";
    return -1;
    }

  env->RegisterNatives( android.tempestClass,
                        methodTable,
                        sizeof(methodTable) / sizeof(methodTable[0]) );

  Log(Log::Info) << "Tempest ~JNI_OnLoad";
  return JNI_VERSION_1_6;
  }

static void* tempestMainFunc(void*){
  Log(Log::Info) << "Tempest MainFunc";
  Android &a = android;

  a.vm->AttachCurrentThread( &a.env, NULL);

  jclass clazz = android.tempestClass;
  jmethodID invokeMain = a.env->GetStaticMethodID( clazz, "invokeMain", "()V");

  android.initialize();
  Log(Log::Info) << "Tempest MainFunc[1]";
  a.env->CallStaticVoidMethod(clazz, invokeMain);

  Log(Log::Info) << "~Tempest MainFunc";
  android.destroy(true);

  a.vm->DetachCurrentThread();

  android.mainThread = 0;
  pthread_exit(0);
  return 0;
  }

#endif
