#include "androidapi.h"

using namespace Tempest;

#ifdef __ANDROID__

#include <Tempest/Application>
#include <Tempest/Window>
#include <Tempest/Event>
#include <map>
#include <queue>
#include <Tempest/Assert>
#include <Tempest/DisplaySettings>

#include <EGL/egl.h>
#include <GLES/gl.h>

#include <pthread.h>
#include <android/log.h>
#include <unistd.h>
#include <sys/stat.h>

#include <jni.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

#include <android/bitmap.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>

#include <android/keycodes.h>


#include <cmath>
#include <locale>

static struct Android{
  EGLDisplay display;
  EGLSurface surface;
  EGLContext context;

  int window_w, window_h;
  Tempest::Window * wnd;

  JavaVM  *vm;
  JNIEnv  *env;
  jobject assets;
  jclass  tempestClass;

  ANativeWindow   *window;
  pthread_mutex_t appMutex, waitMutex;

  pthread_t mainThread;

  bool forceToResize;
  bool isWndAviable;
  bool isPaused;

  std::vector<char> internal, external;
  std::string locale;

  enum MainThreadMessage {
    MSG_NONE = 0,
    MSG_WINDOW_SET,
    MSG_SURFACE_RESIZE,
    MSG_TOUCH,
    MSG_PAUSE,
    MSG_RESUME,
    MSG_KEY_EVENT,
    MSG_CLOSE,
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

  struct MousePointer{
    int   nativeID;
    bool  valid;
    Point pos;
    };
  std::vector<MousePointer> mouse;
  bool closeRqAccepted;

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

    env     = 0;

    closeRqAccepted = 0;

    window_w = 0;
    window_h = 0;
    wnd           = 0;
    forceToResize = 0;

    isWndAviable = false;
    isPaused     = true;

    mainThread = 0;
    msg.  reserve(32);
    mouse.reserve(8);
    }

  bool initialize();
  void destroy(bool killContext);

  void waitForQueue();
  } android;

template< class ... Args >
void LOGI( const Args& ... args ){
  __android_log_print( ANDROID_LOG_INFO, "game", args... );
  }

template< class ... Args >
void LOGE( const Args& ... args ){
  __android_log_print( ANDROID_LOG_ERROR, "game", args... );
  }

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

AndroidAPI::Window *AndroidAPI::createWindow(int w, int h) {
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

bool AndroidAPI::testDisplaySettings( const DisplaySettings & s ) {
  return s.width ==android.window_w  &&
         s.height==android.window_h &&
         s.bits==16 &&
         s.fullScreen;
  }

bool AndroidAPI::setDisplaySettings(const DisplaySettings &s) {
  return testDisplaySettings(s);
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

std::string AndroidAPI::loadTextImpl(const char *file ){
  return loadAssetImpl<std::string>( file );
  }

std::string AndroidAPI::loadTextImpl( const wchar_t* file ){
  return loadAssetImpl<std::string>( toUtf8(file).c_str() );
  }

std::vector<char> AndroidAPI::loadBytesImpl( const char* file ){
  return loadAssetImpl<std::vector<char>>(file);
  }

std::vector<char> AndroidAPI::loadBytesImpl(const wchar_t *file) {
  return loadBytesImpl( toUtf8(file).c_str() );
  }

template< class T >
T AndroidAPI::loadAssetImpl( const char* file ){
  Android &a = android;

  JNIEnv * env = 0;
  a.vm->AttachCurrentThread( &env, NULL);

  AAssetManager* mgr = AAssetManager_fromJava(env, a.assets);
  AAsset* asset = AAssetManager_open(mgr, file, AASSET_MODE_UNKNOWN);

  T_ASSERT( asset );

  long size = AAsset_getLength(asset);
  T str;
  str.resize( size );
  AAsset_read (asset, &str[0], size);
  //__android_log_print(ANDROID_LOG_ERROR, "Tempest", buffer);
  AAsset_close(asset);

  return str;
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
    pthread_mutex_lock(&android.appMutex);
    size_t s = android.msg.size();
    pthread_mutex_unlock(&android.appMutex);

    if( s==0 )
      return;
    }
  }

int AndroidAPI::nextEvents(bool &quit) {
  int r = 0;
  while( !quit ){
    pthread_mutex_lock(&android.appMutex);
    Android::MainThreadMessage msg = Android::MSG_NONE;

    if( android.msg.size() )
      msg = android.msg[0].msg;

    if( android.window==0 && msg!=Android::MSG_RENDER_LOOP_EXIT  ){
      pthread_mutex_unlock(&android.appMutex);
      return 1;
      }

    int sz = android.msg.size();
    pthread_mutex_unlock(&android.appMutex);

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
  pthread_mutex_lock(&android.appMutex);  
  hasWindow = android.window;

  Android::Message msg = Android::MSG_NONE;
  if( android.msg.size() ){
    msg = android.msg[0];

    if( !hasWindow && msg.msg!=Android::MSG_RENDER_LOOP_EXIT ){
      pthread_mutex_unlock(&android.appMutex);
      return 0;
      }

    android.msg.erase( android.msg.begin() );
    }

  pthread_mutex_unlock(&android.appMutex);

  switch( msg.msg ) {
    case Android::MSG_SURFACE_RESIZE:
      break;

    case Android::MSG_RENDER_LOOP_EXIT:
      quit = true;
      break;

    case Android::MSG_KEY_EVENT:{
      Tempest::KeyEvent sce = makeKeyEvent(msg.data1, true);
      SystemAPI::emitEvent(android.wnd, sce, Event::Shortcut);

      if( !sce.isAccepted() ){
        Tempest::KeyEvent e =  makeKeyEvent(msg.data1);
        if( e.key!=Tempest::KeyEvent::K_NoKey ){
          SystemAPI::emitEvent(android.wnd, e, Event::KeyDown);
          SystemAPI::emitEvent(android.wnd, e, Event::KeyUp);
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
        SystemAPI::emitEvent(android.wnd, e, Event::MouseDown);
        }

      if( msg.data1==1 ){
        MouseEvent e( msg.data.x, msg.data.y,
                      MouseEvent::ButtonLeft, 0, id,
                      Event::MouseUp );
        SystemAPI::emitEvent(android.wnd, e, Event::MouseUp);
        android.unsetPointer(msg.data2);
        }

      if( msg.data1==2 ){
        MouseEvent e( msg.data.x, msg.data.y,
                      MouseEvent::ButtonLeft, 0, id,
                      Event::MouseMove );
        if( android.pointerPos(id) != Point(msg.data.x, msg.data.y) ){
          android.pointerPos(id) = Point(msg.data.x, msg.data.y);
          SystemAPI::emitEvent(android.wnd, e, Event::MouseMove);
          }
        }
      }
      break;

    case Android::MSG_CLOSE:  {
      Tempest::CloseEvent e;
      SystemAPI::emitEvent(android.wnd, e, Event::Close );
      android.closeRqAccepted = e.isAccepted();
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

void AndroidAPI::setGeometry( Window *hw, int x, int y, int w, int h ) {
  }

void AndroidAPI::bind( Window *w, Tempest::Window *wx ) {
  android.wnd = wx;
  }

static struct TmpImg{
  std::vector<unsigned char> *data;
  int w,h,bpp;
  } tmpImage;

extern "C"
JNIEXPORT bool JNICALL Java_com_native_1call_TempestActivity_loadImg(
        JNIEnv * env, jobject obj,
        jobject bitmap){
  if( bitmap==0 ){
    LOGE("bad bitmap!");
    return false;
    }

  AndroidBitmapInfo  info;
  AndroidBitmap_getInfo(env, bitmap, &info);

  tmpImage.w = info.width;
  tmpImage.h = info.height;

  unsigned char *pixels;

  if(info.format == ANDROID_BITMAP_FORMAT_RGBA_8888) {
    tmpImage.bpp = 4;
    tmpImage.data->resize( tmpImage.w*tmpImage.h*tmpImage.bpp );

    AndroidBitmap_lockPixels(env, bitmap, reinterpret_cast<void **>(&pixels));
    memcpy( &(*tmpImage.data)[0], pixels, tmpImage.data->size() );
    AndroidBitmap_unlockPixels(env, bitmap);

    return true;
    }

  if(info.format == ANDROID_BITMAP_FORMAT_RGB_565) {
    tmpImage.bpp = 3;
    tmpImage.data->resize( tmpImage.w*tmpImage.h*tmpImage.bpp );

    AndroidBitmap_lockPixels(env, bitmap, reinterpret_cast<void **>(&pixels));

    for( size_t i=0, r=0; i<tmpImage.data->size(); i+=3, r+=2 ){
      uint16_t p = *(uint16_t*)(&pixels[r]);
      unsigned char* v = &(*tmpImage.data)[i];

      //WORD red_mask = 0x7C00;
      //WORD green_mask = 0x3E0;
      //WORD blue_mask = 0x1F;

      unsigned char red_value   = (p & 0x7C00) >> 10;
      unsigned char green_value = (p & 0x3E0)  >> 5;
      unsigned char blue_value  = (p & 0x1F);

      v[0] = red_value   << 3;
      v[1] = green_value << 3;
      v[2] = blue_value  << 3;
      }

    AndroidBitmap_unlockPixels(env, bitmap);
    return true;
    }

  if(info.format != ANDROID_BITMAP_FORMAT_RGBA_8888) {
    LOGE("Bitmap format is not RGBA_8888!");
    return false;
    }

  return true;
  }

bool AndroidAPI::loadImageImpl( const wchar_t *file,
                                int &w,
                                int &h,
                                int &bpp,
                                std::vector<unsigned char>& out ){
  const std::string u8 = toUtf8(file);

  LOGI("load img : %s", u8.c_str());
  std::vector<char> imgBytes = loadBytesImpl(file);
  if( loadS3TCImpl(imgBytes,w,h,bpp,out) )
    return true;

  if( loadPngImpl(imgBytes,w,h,bpp,out) )
    return true;
  //LOGI("load img end  : %s", file);
/*
  JNIEnv * env = 0;
  ((android_app*)android)->activity->vm->AttachCurrentThread( &env, NULL);

  jclass libClass = env->GetObjectClass(((android_app*)android)->activity->clazz);

  jmethodID loadImg = env->GetStaticMethodID(libClass, "loadImage", "(Ljava/lang/String;)Z");

  jstring jstr = env->NewStringUTF( u8.c_str() );

  //pthread_mutex_lock( &imgMutex );
  tmpImage.data = &out;
  bool ok = env->CallStaticBooleanMethod( libClass, loadImg, jstr );

  w   = tmpImage.w;
  h   = tmpImage.h;
  bpp = tmpImage.bpp;

  tmpImage.data = 0;
  //pthread_mutex_unlock( &imgMutex );
  env->DeleteLocalRef( jstr );

  return ok;*/
  return false;
  }

bool AndroidAPI::saveImageImpl( const wchar_t* file,
                                int &w,
                                int &h,
                                int &bpp,
                                std::vector<unsigned char>& in ){

  }

bool AndroidAPI::isGraphicsContextAviable( Tempest::Window * ) {
  return android.window;
  }

static void render() {
  Android& e = android;

  if( e.display!= EGL_NO_DISPLAY && e.wnd ){
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

  const EGLint attribs[] = {
    EGL_SURFACE_TYPE,
    EGL_WINDOW_BIT,
    EGL_BLUE_SIZE, 5,
    EGL_GREEN_SIZE, 6,
    EGL_RED_SIZE, 5,
    EGL_DEPTH_SIZE,   16,
    EGL_STENCIL_SIZE, 0,
    EGL_NONE
  };

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
    LOGE("Unable to eglMakeCurrent");
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

  return true;
  }

void Android::destroy( bool killContext ) {
  LOGI("Destroying context");

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

static void resize( JNIEnv * , jobject , jint w, jint h ) {
  pthread_mutex_lock( &android.waitMutex );

  Android::Message m = Android::MSG_SURFACE_RESIZE;
  m.data.w = w;
  m.data.h = h;

  pthread_mutex_lock( &android.appMutex );
  android.msg.push_back( m );
  pthread_mutex_unlock( &android.appMutex );

  android.waitForQueue();
  pthread_mutex_unlock( &android.waitMutex );
  }

static void JNICALL start(JNIEnv* jenv, jobject obj) {
  LOGI("nativeOnStart");
  }

static void JNICALL stop(JNIEnv* jenv, jobject obj) {
  LOGI("nativeOnStop");
  }

static void JNICALL resume(JNIEnv* jenv, jobject obj) {
  LOGI("nativeOnResume");
  pthread_mutex_lock( &android.waitMutex );

  pthread_mutex_lock( &android.appMutex );
  android.msg.push_back( Android::MSG_RESUME );
  android.isPaused = false;
  pthread_mutex_unlock( &android.appMutex );

  android.waitForQueue();
  pthread_mutex_unlock( &android.waitMutex );
  }

static void JNICALL pauseA(JNIEnv* jenv, jobject obj) {
  LOGI("nativeOnPause");
  pthread_mutex_lock( &android.waitMutex );
  pthread_mutex_lock( &android.appMutex );
  android.msg.push_back( Android::MSG_PAUSE );

  android.isPaused = true;
  pthread_mutex_unlock( &android.appMutex );
  android.waitForQueue();
  pthread_mutex_unlock( &android.waitMutex );
  }

static void JNICALL setAssets(JNIEnv* jenv, jobject obj, jobject assets ) {
  LOGI("setAssets");
  android.assets = jenv->NewGlobalRef(assets);
  }

static void JNICALL nativeOnTouch( JNIEnv* , jobject ,
                                   jint x, jint y, jint act, jint pid ){
  pthread_mutex_lock(&android.waitMutex);

  pthread_mutex_lock(&android.appMutex);
  Android::Message m = Android::MSG_TOUCH;
  m.data.x = x;
  m.data.y = y;
  m.data1  = act;
  m.data2  = pid;

  android.msg.push_back(m);

  pthread_mutex_unlock(&android.appMutex);
  //android.waitForQueue();
  pthread_mutex_unlock( &android.waitMutex );
  }

static void JNICALL nativeSetSurface( JNIEnv* jenv, jobject obj, jobject surface) {
  if( surface ) {
    android.window = ANativeWindow_fromSurface(jenv, surface);
    LOGI("Got window %p", android.window);

    pthread_mutex_lock(&android.appMutex);
    android.msg.push_back( Android::MSG_WINDOW_SET );
    pthread_mutex_unlock(&android.appMutex);

    if( android.mainThread==0 )
      pthread_create(&android.mainThread, 0, tempestMainFunc, 0);
    } else {
    LOGI("Releasing window");
    pthread_mutex_lock(&android.appMutex);
    ANativeWindow_release(android.window);
    android.window = 0;
    pthread_mutex_unlock(&android.appMutex);
    }

  return;
  }

static void JNICALL onCreate(JNIEnv* , jobject ) {
  LOGI("nativeOnCreate");

  pthread_mutex_init(&android.waitMutex, 0);
  pthread_mutex_init(&android.appMutex, 0);

  android.mainThread = 0;
  }

static void JNICALL onDestroy(JNIEnv* jenv, jobject obj) {
  pthread_mutex_lock( &android.appMutex );
  android.msg.clear();
  android.msg.push_back( Android::MSG_RENDER_LOOP_EXIT );
  pthread_mutex_unlock( &android.appMutex );

  pthread_join(android.mainThread,0);
  android.mainThread = 0;

  pthread_mutex_destroy(&android.appMutex);
  pthread_mutex_destroy(&android.waitMutex);
  android.msg.clear();

  LOGI("nativeOnDestroy");
  }

static void JNICALL onKeyEvent(JNIEnv* , jobject, jint key ) {
  LOGI("onKeyEvent");

  pthread_mutex_lock( &android.waitMutex );
  pthread_mutex_lock( &android.appMutex );

  if( key!=AKEYCODE_BACK ){
    android.msg.push_back( Android::MSG_KEY_EVENT );
    android.msg.back().data1 = key;
    } else {
    android.msg.push_back( Android::MSG_RENDER_LOOP_EXIT );
    }

  pthread_mutex_unlock( &android.appMutex );
  //android.waitForQueue();
  pthread_mutex_unlock( &android.waitMutex );
  }

static jint JNICALL nativeCloseEvent( JNIEnv* , jobject ){
  pthread_mutex_lock( &android.waitMutex );
  pthread_mutex_lock( &android.appMutex );

  android.msg.push_back( Android::MSG_CLOSE );

  pthread_mutex_unlock( &android.appMutex );
  android.waitForQueue();
  pthread_mutex_unlock( &android.waitMutex );

  return android.closeRqAccepted? 1:0;
/*
  pthread_mutex_lock( &android.waitMutex );
  pthread_mutex_lock( &android.appMutex );

  if( android.closeRqAccepted ){
    android.msg.push_back( Android::MSG_RENDER_LOOP_EXIT );
    }

  pthread_mutex_unlock( &android.appMutex );
  android.waitForQueue();
  pthread_mutex_unlock( &android.waitMutex );*/
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

jint JNI_OnLoad(JavaVM *vm, void */*reserved*/){
  static JNINativeMethod methodTable[] = {
    {"nativeOnCreate",  "()V", (void *) onCreate  },
    {"nativeOnDestroy", "()V", (void *) onDestroy },

    {"nativeOnStart",  "()V", (void *) start  },
    {"nativeOnResume", "()V", (void *) resume },
    {"nativeOnPause",  "()V", (void *) pauseA },
    {"nativeOnStop",   "()V", (void *) stop   },
    {"nativeInitLocale",   "(Ljava/lang/String;)V", (void *)nativeInitLocale },
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
    LOGE("Failed to get the environment");
    return -1;
    }

  android.tempestClass = env->FindClass( SystemAPI::androidActivityClass().c_str() );
  android.tempestClass = (jclass)env->NewGlobalRef(android.tempestClass);

  if (!android.tempestClass) {
    LOGE( "failed to get %s class reference", SystemAPI::androidActivityClass().c_str() );
    return -1;
    }

  env->RegisterNatives( android.tempestClass,
                        methodTable,
                        sizeof(methodTable) / sizeof(methodTable[0]) );

  LOGI("Tempest JNI_OnLoad");
  return JNI_VERSION_1_6;
  }

static void* tempestMainFunc(void*){
  LOGI("Tempest MainFunc");
  Android &a = android;

  a.vm->AttachCurrentThread( &a.env, NULL);

  jclass clazz = android.tempestClass;
  jmethodID invokeMain = a.env->GetStaticMethodID( clazz, "invokeMain", "()V");

  android.initialize();
  a.env->CallStaticVoidMethod(clazz, invokeMain);

  LOGI("~Tempest MainFunc");
  android.destroy(true);

  a.vm->DetachCurrentThread();

  pthread_exit(0);
  return 0;
  }

#endif
