#include "androidapi.h"

using namespace Tempest;

#ifdef __ANDROID__

#include "STLConfig.h"

#include <Tempest/Application>
#include <Tempest/Window>
#include <Tempest/Event>
#include <map>
#include <queue>
#include <cassert>

#include <GLES/gl.h>
#include <pthread.h>
#include <android/log.h>
#include <unistd.h>

#include <jni.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

template< class ... Args >
void LOGI( const Args& ... args ){
  __android_log_print( ANDROID_LOG_INFO, "game", args... );
  }

template< class ... Args >
void LOGE( const Args& ... args ){
  __android_log_print( ANDROID_LOG_ERROR, "game", args... );
  }

static int window_w = 0, window_h = 0;
static Tempest::Window * wnd = 0;

static pthread_mutex_t appMutex, imgMutex;
static pthread_t mainThread = 0;
static JavaVM *jvm = 0;

static jclass    libClass = 0;
static jmethodID callMain = 0; 
static jmethodID loadImg  = 0; 
static jobject   assets   = 0;

static void* start( void* ){  
  JNIEnv * env = 0;
  jvm->AttachCurrentThread( &env, NULL);

  env->CallStaticVoidMethod( libClass, callMain );
  return 0;
  } 

struct AEvent {
  enum Type{
    NoEvent,
    SizeEvent,
    KeyEvent,

    MouseDownEvent,
    MouseUpEvent,
    MouseMoveEvent,

    QuitEvent
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

  JNIEXPORT void JNICALL Java_com_android_gl2jni_GL2JNILib_setupAManager(JNIEnv * env, jobject obj, jobject am);

  JNIEXPORT void JNICALL Java_com_android_gl2jni_GL2JNILib_mouseDownEvent(JNIEnv * env, jobject obj,  jint x, jint y);
  JNIEXPORT void JNICALL Java_com_android_gl2jni_GL2JNILib_mouseUpEvent  (JNIEnv * env, jobject obj,  jint x, jint y);
  JNIEXPORT void JNICALL Java_com_android_gl2jni_GL2JNILib_mouseMoveEvent(JNIEnv * env, jobject obj,  jint x, jint y);
  
  JNIEXPORT void JNICALL Java_com_android_gl2jni_GL2JNILib_quit (JNIEnv * env, jobject obj);

  JNIEXPORT void JNICALL Java_com_android_gl2jni_GL2JNILib_resetDevice(JNIEnv * env, jobject obj);
  JNIEXPORT void JNICALL Java_com_android_gl2jni_GL2JNILib_backKeyEvent(JNIEnv * env, jobject obj);
  JNIEXPORT bool JNICALL Java_com_android_gl2jni_GL2JNILib_loadImg( JNIEnv * env, jobject obj,
                                                                    jobject bitmap );

  }

JNIEXPORT void Java_com_android_gl2jni_GL2JNILib_initTempest(JNIEnv * env, jobject obj){
  LOGI("Tempest native init begin");
  pthread_mutex_init( &appMutex, 0 );
  pthread_mutex_init( &imgMutex, 0 );

  env->GetJavaVM(&jvm);

  jclass c = env->FindClass("com/android/gl2jni/GL2JNILib");
  libClass = (jclass)env->NewGlobalRef( (jclass)c );

  callMain = env->GetStaticMethodID(libClass, "runApplication", "()V");
  loadImg  = env->GetStaticMethodID(libClass, "loadImage", "(Ljava/lang/String;)Z");

  LOGI("Tempest native init end");
  }

JNIEXPORT void JNICALL Java_com_android_gl2jni_GL2JNILib_setupAManager( JNIEnv * env,  
                                                                        jobject obj, 
                                                                        jobject am ){
  assets = env->NewGlobalRef(am);
  }

JNIEXPORT void JNICALL Java_com_android_gl2jni_GL2JNILib_init( JNIEnv * env, 
                                                               jobject obj, 
                                                               jint w, jint h) {
  window_w = w;
  window_h = h; 

  AEvent e;
  e.type = AEvent::SizeEvent;

  e.w = w;
  e.h = h;
  
  pthread_mutex_lock( &appMutex );
  events.push(e);
  pthread_mutex_unlock( &appMutex );
  }

JNIEXPORT void JNICALL Java_com_android_gl2jni_GL2JNILib_quit (JNIEnv * env, jobject obj){
  AEvent e;
  e.type = AEvent::QuitEvent;
  
  pthread_mutex_lock( &appMutex );
  events.push(e);
  pthread_mutex_unlock( &appMutex );
  }

JNIEXPORT void JNICALL Java_com_android_gl2jni_GL2JNILib_mouseDownEvent( JNIEnv * env, jobject obj,
                                                                         jint x, jint y) {
  AEvent e;
  e.type = AEvent::MouseDownEvent;

  e.x = x;
  e.y = y;
  
  pthread_mutex_lock( &appMutex );
  events.push(e);
  pthread_mutex_unlock( &appMutex );
  }

JNIEXPORT void JNICALL Java_com_android_gl2jni_GL2JNILib_mouseUpEvent( JNIEnv * env, jobject obj,
                                                                       jint x, jint y) {
  AEvent e;
  e.type = AEvent::MouseUpEvent;

  e.x = x;
  e.y = y;
  
  
  pthread_mutex_lock( &appMutex );
  events.push(e);
  pthread_mutex_unlock( &appMutex );
  }

JNIEXPORT void JNICALL Java_com_android_gl2jni_GL2JNILib_mouseMoveEvent( JNIEnv * env, jobject obj, 
                                                                         jint x, jint y) {
  AEvent e;
  e.type = AEvent::MouseMoveEvent;

  e.x = x;
  e.y = y;  
  
  pthread_mutex_lock( &appMutex );
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
  JNIEnv * env = 0;
  jvm->AttachCurrentThread( &env, NULL);
  env->DeleteGlobalRef(assets);

  pthread_mutex_destroy( &appMutex );
  pthread_mutex_destroy( &imgMutex );
  }

void AndroidAPI::startApplication( ApplicationInitArgs * ) {
  }

void AndroidAPI::endApplication() {
  
  }

std::string AndroidAPI::loadTextImpl( const char* file ){
  JNIEnv * env = 0;
  jvm->AttachCurrentThread( &env, NULL);

  AAssetManager* mgr = AAssetManager_fromJava(env, assets);
  AAsset* asset = AAssetManager_open(mgr, file, AASSET_MODE_UNKNOWN);

  assert( asset );
  /*
  if (NULL == asset) {
    __android_log_print(ANDROID_LOG_ERROR, "Tempest", "_ASSET_NOT_FOUND_");
    return "";
    }*/

  long size = AAsset_getLength(asset);
  std::string str;
  str.resize( size );
  AAsset_read (asset, &str[0], size);
  //__android_log_print(ANDROID_LOG_ERROR, "Tempest", buffer);
  AAsset_close(asset);

  return str;
  }

std::vector<char> AndroidAPI::loadBytesImpl( const char* file ){
  JNIEnv * env = 0;
  jvm->AttachCurrentThread( &env, NULL);

  AAssetManager* mgr = AAssetManager_fromJava(env, assets);
  AAsset* asset = AAssetManager_open(mgr, file, AASSET_MODE_UNKNOWN);

  assert( asset );
  /*
  if (NULL == asset) {
    __android_log_print(ANDROID_LOG_ERROR, "Tempest", "_ASSET_NOT_FOUND_");
    return "";
    }*/

  long size = AAsset_getLength(asset);
  std::vector<char> str;
  str.resize( size );
  AAsset_read(asset, &str[0], size);
  //__android_log_print(ANDROID_LOG_ERROR, "Tempest", buffer);
  AAsset_close(asset);

  return str;
  }

int AndroidAPI::nextEvent(bool &quit) {
  AEvent e;
  e.type = AEvent::NoEvent;

  pthread_mutex_lock( &appMutex );
  if( events.size() ){
    e = events.front();
    events.pop();
    LOGI( "events.size() = %d", events.size() );
    }
  pthread_mutex_unlock( &appMutex );
  
  if( e.type==AEvent::QuitEvent ){
    quit = true;
    }
  else
  if( e.type==AEvent::SizeEvent ){
    wnd->resize(e.w, e.h);
    }
  else
  if( e.type==AEvent::MouseDownEvent ){
    MouseEvent ex( e.x, e.y, Tempest::Event::ButtonLeft );
    wnd->mouseDownEvent(ex);
    }
  else
  if( e.type==AEvent::MouseUpEvent ){
    MouseEvent ex( e.x, e.y, Tempest::Event::ButtonLeft );
    wnd->mouseUpEvent(ex);
    }
  else
  if( e.type==AEvent::MouseMoveEvent ){
    MouseEvent ex( e.x, e.y, Tempest::Event::ButtonNone );
    wnd->mouseDragEvent(ex);

    if( !ex.isAccepted() )
      wnd->mouseMoveEvent(ex);
    }
  else{
    wnd->render();
    sleep(0);
    }
  return 0;
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

Size AndroidAPI::windowClientRect( Window* ){
  return Size(window_w, window_h);
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

static struct TmpImg{
  std::vector<unsigned char> *data;
  int w,h,bpp;
  } tmpImage;

JNIEXPORT bool JNICALL Java_com_android_gl2jni_GL2JNILib_loadImg( JNIEnv * env, jobject obj,
                                                                  jobject bitmap){
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

bool AndroidAPI::loadImageImpl( const char* file,
                                int &w,
                                int &h,
                                int &bpp,
                                std::vector<unsigned char>& out ){
  JNIEnv * env = 0;
  jvm->AttachCurrentThread( &env, NULL);
                                  
  jstring jstr = env->NewStringUTF( file );
  
  pthread_mutex_lock( &imgMutex );
  tmpImage.data = &out;
  bool ok = env->CallStaticBooleanMethod( libClass, loadImg, jstr );
  
  w   = tmpImage.w;
  h   = tmpImage.h;
  bpp = tmpImage.bpp;

  tmpImage.data = 0;
  pthread_mutex_unlock( &imgMutex );
  env->DeleteLocalRef( jstr );

  return ok;
  }

bool AndroidAPI::saveImageImpl( const char* file,
                                int &w,
                                int &h,
                                int &bpp,
                                std::vector<unsigned char>& in ){

  }

#endif
