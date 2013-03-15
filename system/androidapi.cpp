#include "androidapi.h"

using namespace Tempest;

#ifdef __ANDROID__

#include "STLConfig.h"

#include <Tempest/Application>
#include <Tempest/Window>
#include <Tempest/Event>
#include <map>
#include <queue>

#include <GLES/gl.h>
#include <pthread.h>
#include <android/log.h>
#include <unistd.h>

#include <dlfcn.h>
#include <jni.h>

template< class ... Args >
void LOGI( Args& ... args ){
  __android_log_print( ANDROID_LOG_INFO, "game", args... );
  }

template< class ... Args >
void LOGE( Args& ... args ){
  __android_log_print( ANDROID_LOG_ERROR, "game", args... );
  }

static int window_w = 0, window_h = 0;
static Tempest::Window * wnd = 0;

static pthread_mutex_t appMutex;
static pthread_t mainThread = 0;
static JavaVM *jvm = 0;

static jclass    libClass = 0;
static jmethodID callMain = 0; 
static jmethodID loadImg  = 0; 

static void* start( void* ){  
  JNIEnv * env = 0;
  jvm->AttachCurrentThread( &env, NULL);

  env->CallStaticVoidMethod( libClass, callMain );
  return 0;
  } 
/*
struct Api{
  Api(){
    }

  ~Api(){
    pthread_join( mainThread, NULL );
    pthread_mutex_destroy( &appMutex );
    }

  };

Api& api(){
  static Api a;
  return a;
  }*/

struct AEvent {
  enum Type{
    NoEvent,
    SizeEvent,
    KeyEvent,

    MouseDownEvent,
    MouseUpEvent,
    MouseMoveEvent
    } type;

  int w,h, x,y;
  };

static std::queue<AEvent> events;

static AEvent peekMessage(){
  AEvent e;
  e.type = AEvent::NoEvent;

  pthread_mutex_lock( &appMutex );

  if( events.size() ){
    e = events.back();
    events.pop();
    }

  pthread_mutex_unlock( &appMutex );

  return e;
  }

#include <android/bitmap.h>

extern "C" {
  JNIEXPORT void JNICALL Java_com_android_gl2jni_GL2JNILib_initTempest(JNIEnv * env, jobject obj);
  JNIEXPORT void JNICALL Java_com_android_gl2jni_GL2JNILib_init(JNIEnv * env, jobject obj,  jint width, jint height);
  JNIEXPORT void JNICALL Java_com_android_gl2jni_GL2JNILib_run (JNIEnv * env, jobject obj);

  JNIEXPORT void JNICALL Java_com_android_gl2jni_GL2JNILib_mouseDownEvent(JNIEnv * env, jobject obj,  jint x, jint y);
  JNIEXPORT void JNICALL Java_com_android_gl2jni_GL2JNILib_mouseUpEvent  (JNIEnv * env, jobject obj,  jint x, jint y);
  JNIEXPORT void JNICALL Java_com_android_gl2jni_GL2JNILib_mouseMoveEvent(JNIEnv * env, jobject obj,  jint x, jint y);
  
  JNIEXPORT void JNICALL Java_com_android_gl2jni_GL2JNILib_resetDevice(JNIEnv * env, jobject obj);
  JNIEXPORT void JNICALL Java_com_android_gl2jni_GL2JNILib_backKeyEvent(JNIEnv * env, jobject obj);
  JNIEXPORT int  JNICALL Java_com_android_gl2jni_GL2JNILib_loadImg( JNIEnv * env, jobject obj,
                                                                    jstring str, jobject bitmap );

  }

JNIEXPORT int JNICALL Java_com_android_gl2jni_GL2JNILib_loadImg( JNIEnv * env, jobject obj,
                                                                 jstring str, jobject bitmap){
  AndroidBitmapInfo  info;
  uint32_t          *pixels;

  AndroidBitmap_getInfo(env, bitmap, &info);

  if(info.format != ANDROID_BITMAP_FORMAT_RGBA_8888) {
    LOGE("Bitmap format is not RGBA_8888!");
    return 0;
    }

  AndroidBitmap_lockPixels(env, bitmap, reinterpret_cast<void **>(&pixels));
  const char *nativeString = env->GetStringUTFChars( str, 0);

  int x = 0;
  /*
  int x = application().addTexture( nativeString, pixels,
    info.width, info.height );
    */

  env->ReleaseStringUTFChars( str, nativeString);
  AndroidBitmap_unlockPixels(env, bitmap);

  return x;
  }

void loadImage( JNIEnv * env, const char* file ){
  jstring jstr = env->NewStringUTF( file );
  env->CallStaticVoidMethod( libClass, loadImg, jstr );
  env->DeleteLocalRef( jstr );
  }

JNIEXPORT void Java_com_android_gl2jni_GL2JNILib_initTempest(JNIEnv * env, jobject obj){
  LOGI("Tempest native init begin");
  pthread_mutex_init( &appMutex, 0 );

  env->GetJavaVM(&jvm);

  jclass c = env->FindClass("com/android/gl2jni/GL2JNILib");
  libClass = (jclass)env->NewGlobalRef( (jclass)c );

  callMain = env->GetStaticMethodID(libClass, "runApplication", "()V");
  loadImg  = env->GetStaticMethodID(libClass, "loadImage", "(Ljava/lang/String;)V");

  LOGI("Tempest native init end");

  //pthread_create( &mainThread, NULL, start, NULL );
  }

JNIEXPORT void JNICALL Java_com_android_gl2jni_GL2JNILib_init( JNIEnv * env, 
                                                               jobject obj, 
                                                               jint w, jint h) {
  window_w = w;
  window_h = h; 

  pthread_mutex_lock( &appMutex );
  AEvent e;
  e.type = AEvent::SizeEvent;

  e.w = w;
  e.h = h;

  events.push(e);
  pthread_mutex_unlock( &appMutex );
  }

JNIEXPORT void JNICALL Java_com_android_gl2jni_GL2JNILib_mouseDownEvent( JNIEnv * env, jobject obj,
                                                                         jint x, jint y) {
  pthread_mutex_lock( &appMutex );
  AEvent e;
  e.type = AEvent::MouseDownEvent;

  e.x = x;
  e.y = y;

  events.push(e);
  pthread_mutex_unlock( &appMutex );
  }

JNIEXPORT void JNICALL Java_com_android_gl2jni_GL2JNILib_mouseUpEvent( JNIEnv * env, jobject obj,
                                                                       jint x, jint y) {
  
  pthread_mutex_lock( &appMutex );
  AEvent e;
  e.type = AEvent::MouseUpEvent;

  e.x = x;
  e.y = y;

  events.push(e);
  pthread_mutex_unlock( &appMutex );
  }

JNIEXPORT void JNICALL Java_com_android_gl2jni_GL2JNILib_mouseMoveEvent( JNIEnv * env, jobject obj, 
                                                                         jint x, jint y) {
  
  pthread_mutex_lock( &appMutex );
  AEvent e;
  e.type = AEvent::MouseMoveEvent;

  e.x = x;
  e.y = y;

  events.push(e);
  pthread_mutex_unlock( &appMutex );
  }

JNIEXPORT void JNICALL Java_com_android_gl2jni_GL2JNILib_resetDevice(JNIEnv * g_env, jobject obj) {  
  //application().resetDevice();
  }

JNIEXPORT void JNICALL Java_com_android_gl2jni_GL2JNILib_run(JNIEnv * g_env, jobject obj) {
  start(0);
  }

JNIEXPORT void JNICALL Java_com_android_gl2jni_GL2JNILib_backKeyEvent(JNIEnv * g_env, jobject obj) {
  //application().backKeyEvent();
  }

AndroidAPI::AndroidAPI() {
  }

AndroidAPI::~AndroidAPI() {
  }

void AndroidAPI::startApplication( ApplicationInitArgs * ) {
 
  }

void AndroidAPI::endApplication() {
 
  }

int AndroidAPI::nextEvent(bool &quit) {
  AEvent e;
  e.type = AEvent::NoEvent;

  pthread_mutex_lock( &appMutex );
  if( events.size() ){
    e = events.front();
    events.pop();
    }
  pthread_mutex_unlock( &appMutex );

  if( e.type==AEvent::MouseDownEvent ){
    MouseEvent ex( e.x, e.y, Tempest::Event::ButtonLeft );
    wnd->mouseDownEvent(ex);
    }
  else
  if( e.type==AEvent::MouseUpEvent ){
    MouseEvent ex( e.x, e.y, Tempest::Event::ButtonLeft );
    wnd->mouseDragEvent(ex);
    }
  else
  if( e.type==AEvent::MouseMoveEvent ){
    MouseEvent ex( e.x, e.y, Tempest::Event::ButtonNone );
    wnd->mouseMoveEvent(ex);

    if( !ex.isAccepted() )
      wnd->mouseUpEvent(ex);
    }

  wnd->render();
  sleep(0);
  return 0;
  }

AndroidAPI::Window *AndroidAPI::createWindow(int w, int h) {
  return 0;
  }

void AndroidAPI::deleteWindow( Window *w ) {
  }

void AndroidAPI::show(Window *) {
  glViewport(0,0,window_w, window_h);
  wnd->resize(window_w, window_h);
  }

void AndroidAPI::setGeometry( Window *hw, int x, int y, int w, int h ) {
  }

void AndroidAPI::bind( Window *w, Tempest::Window *wx ) {
  wnd = wx;
  }

#endif
