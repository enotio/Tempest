#include "androidapi.h"

using namespace Tempest;

#ifdef __ANDROID__

#include <Tempest/Application>
#include <Tempest/Window>
#include <Tempest/Event>
#include <map>
#include <queue>
#include <cassert>

#include <EGL/egl.h>
#include <GLES/gl.h>

#include <pthread.h>
#include <android/log.h>
#include <unistd.h>

#include <jni.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <android_native_app_glue.h>
#include <android/bitmap.h>

struct android_app;

struct AEngine{
  //bool       isContextAviable;
  EGLDisplay display;
  EGLSurface surface;
  EGLContext context;
  int window_w, window_h;
  Tempest::Window * wnd;

  bool forceToResize;

  AEngine(){
    display = EGL_NO_DISPLAY;
    surface = 0;
    context = 0;

    window_w = 0;
    window_h = 0;
    wnd           = 0;
    forceToResize = 0;
    }
  };

template< class ... Args >
void LOGI( const Args& ... args ){
  __android_log_print( ANDROID_LOG_INFO, "game", args... );
  }

template< class ... Args >
void LOGE( const Args& ... args ){
  __android_log_print( ANDROID_LOG_ERROR, "game", args... );
  }

static android_app* android = 0;

static int32_t handle_input( android_app* app, AInputEvent* event);
static void    handle_cmd  ( android_app* app, int32_t cmd );

static void initDisplay( android_app* app );
static void termDisplay( android_app* app , bool killContext = false );
static void      render( android_app* app );

static int preloadNextEvent( bool & q, android_app* state ){
  int ident;
  int events;
  struct android_poll_source* source;

  if ((ident=ALooper_pollOnce( 0, NULL, &events,
                                 (void**)&source)) >= 0) {
    if( source ) {
      source->process(state, source);
      }

    if( state->destroyRequested != 0 ) {
    //if( !nv_app_status_running(android) ){
      q = true;
      return 0;
      }
    }

  return 0;
  }

void Tempest_android_main( android_app* s,
                           int (*main)(int, char**),
                           int w,
                           int h ){
  AEngine e;
  e.window_w = w;
  e.window_h = h;

  android = s;
  android->userData     = &e;
  android->onAppCmd     = handle_cmd;
  android->onInputEvent = handle_input;  

  bool quit = false;
  while( !quit && (e.context==EGL_NO_CONTEXT) ){
    preloadNextEvent(quit, android);
    if( quit ){
      return;
      }
    }

  if( !quit ){
    char args[] = "TempestAndroidApp";
    ((AndroidAPI&)SystemAPI::instance()).android = android;
    LOGI("Tempest: start main");
    main(1, (char**)&args);
    LOGI("Tempest: main finished");

    termDisplay(android, true);
    }

  e.wnd = 0;


  JNIEnv * env = 0;
  ((android_app*)android)->activity->vm->AttachCurrentThread( &env, NULL);
  jclass libClass = env->GetObjectClass(((android_app*)android)->activity->clazz);
  jmethodID finish = env->GetStaticMethodID(libClass, "finishAll", "()V");

  env->CallStaticVoidMethod( libClass, finish );

  android->activity->vm->DetachCurrentThread();

  ANativeActivity_finish(android->activity);
  }

using namespace Tempest;

AndroidAPI::AndroidAPI(){

  }

AndroidAPI::~AndroidAPI(){

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

std::string AndroidAPI::loadTextImpl( const char* file ){
  //JNIEnv * env = 0;

  //android->activity->assetManager;
  //jvm->AttachCurrentThread( &env, NULL);

  AAssetManager* mgr = ((android_app*)android)->activity->assetManager;//AAssetManager_fromJava(env, assets);
  AAsset* asset = AAssetManager_open(mgr, file, AASSET_MODE_UNKNOWN);

  assert( asset );

  long size = AAsset_getLength(asset);
  std::string str;
  str.resize( size );
  AAsset_read (asset, &str[0], size);
  //__android_log_print(ANDROID_LOG_ERROR, "Tempest", buffer);
  AAsset_close(asset);

  return str;
  }

std::vector<char> AndroidAPI::loadBytesImpl( const char* file ){
  AAssetManager* mgr = ((android_app*)android)->activity->assetManager;
  AAsset* asset = AAssetManager_open(mgr, file, AASSET_MODE_UNKNOWN);

  assert( asset );

  long size = AAsset_getLength(asset);
  std::vector<char> str;
  str.resize( size );
  AAsset_read(asset, &str[0], size);
  //__android_log_print(ANDROID_LOG_ERROR, "Tempest", buffer);
  AAsset_close(asset);

  return str;
  }

int AndroidAPI::nextEvent(bool &quit) {
  int ident;
  int events;
  struct android_poll_source* source;

  if ((ident=ALooper_pollOnce( 0, NULL, &events,
                                 (void**)&source)) >= 0) {

    // Process this event.
    if (source != NULL) {
      source->process(((android_app*)android), source);
      }

    // Check if we are exiting.
    if( ((android_app*)android)->destroyRequested != 0 ){
    //if( !nv_app_status_running((android_app*)android) ){
      //termDisplay( ((android_app*)android) );
      quit = true;
      return 0;
      }
    } else {
    if( ((android_app*)android)->destroyRequested==0 )
    //if( !nv_app_status_running((android_app*)android) )
      render(((android_app*)android));
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
  AEngine *e = (AEngine*)(((android_app*)android)->userData);
  return Size( e->window_w, e->window_h );
  }

void AndroidAPI::deleteWindow( Window *w ) {
  AEngine *e = (AEngine*)(((android_app*)android)->userData);
  e->wnd = 0;
  }

void AndroidAPI::show(Window *) {
  AEngine *e = (AEngine*)(((android_app*)android)->userData);

  if( e->wnd && e->display!=EGL_NO_DISPLAY ){
    glViewport(0,0,e->window_w, e->window_h);
    //e->forceToResize = true;
    e->wnd->resize( e->window_w, e->window_h );
    }
  }

void AndroidAPI::setGeometry( Window *hw, int x, int y, int w, int h ) {
  }

void AndroidAPI::bind( Window *w, Tempest::Window *wx ) {
  AEngine *e = (AEngine*)(((android_app*)android)->userData);
  e->wnd = wx;
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

bool AndroidAPI::loadImageImpl( const char* file,
                                int &w,
                                int &h,
                                int &bpp,
                                std::vector<unsigned char>& out ){
  LOGI("load img : %s", file);
  std::vector<char> imgBytes = loadBytesImpl(file);
  if( loadS3TCImpl(imgBytes,w,h,bpp,out) )
    return true;

  if( loadPngImpl(imgBytes,w,h,bpp,out) )
    return true;
  //LOGI("load img end  : %s", file);

  JNIEnv * env = 0;
  ((android_app*)android)->activity->vm->AttachCurrentThread( &env, NULL);

  jclass libClass = env->GetObjectClass(((android_app*)android)->activity->clazz);

  jmethodID loadImg = env->GetStaticMethodID(libClass, "loadImage", "(Ljava/lang/String;)Z");

  jstring jstr = env->NewStringUTF( file );

  //pthread_mutex_lock( &imgMutex );
  tmpImage.data = &out;
  bool ok = env->CallStaticBooleanMethod( libClass, loadImg, jstr );

  w   = tmpImage.w;
  h   = tmpImage.h;
  bpp = tmpImage.bpp;

  tmpImage.data = 0;
  //pthread_mutex_unlock( &imgMutex );
  env->DeleteLocalRef( jstr );

  return ok;
  }

bool AndroidAPI::saveImageImpl( const char* file,
                                int &w,
                                int &h,
                                int &bpp,
                                std::vector<unsigned char>& in ){

  }

bool AndroidAPI::isGraphicsContextAviable( Tempest::Window *w ) {
  return 1;//isContextAviable;
  }

static int32_t handle_input( android_app* a, AInputEvent* event) {
  AEngine *e = (AEngine*)a->userData;

  if( !e->wnd )
    return 0;

  if( AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION ){
    int x = AMotionEvent_getX(event, 0),
        y = AMotionEvent_getY(event, 0);
    int action = AMotionEvent_getAction(event) & AMOTION_EVENT_ACTION_MASK;

    if( action==AMOTION_EVENT_ACTION_DOWN ){
      MouseEvent ex( x, y, Tempest::Event::ButtonLeft );
      e->wnd->mouseDownEvent(ex);
      }
    else
    if( action==AMOTION_EVENT_ACTION_UP ){
      MouseEvent ex( x, y, Tempest::Event::ButtonLeft );
      e->wnd->mouseUpEvent(ex);
      }
    else
    if( action==AMOTION_EVENT_ACTION_MOVE ){
      MouseEvent ex( x, y, Tempest::Event::ButtonNone );
      e->wnd->mouseDragEvent(ex);

      if( !ex.isAccepted() )
        e->wnd->mouseMoveEvent(ex);
      }
    return 1;
    }

  if( AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION ){
    int x = AMotionEvent_getX(event, 0),
        y = AMotionEvent_getY(event, 0);

    MouseEvent ex( x, y, Tempest::Event::ButtonNone );
    e->wnd->mouseDragEvent(ex);

    if( !ex.isAccepted() )
      e->wnd->mouseMoveEvent(ex);
    return 1;
    }

  return 0;
  }

static void handle_cmd( android_app* app, int32_t cmd ) {
  AEngine* e = (AEngine*)app->userData;

  switch (cmd) {
    case APP_CMD_SAVE_STATE:
      //engine->app->savedState = malloc( sizeof(saved_state) );
      //*((saved_state*)engine->app->savedState) = engine->state;
      //engine->app->savedStateSize = sizeof(saved_state);
      break;

    case APP_CMD_INIT_WINDOW:
      if( app->window != NULL ){
        initDisplay(app);
        render(app);
        }
      break;

    case APP_CMD_TERM_WINDOW:
      termDisplay(app);
      break;

    case APP_CMD_WINDOW_RESIZED:
    case APP_CMD_CONFIG_CHANGED:
      e->window_w = 0;
      e->window_h = 0;
      termDisplay(app);
      initDisplay(app);
      break;

    case APP_CMD_GAINED_FOCUS:
      break;

    case APP_CMD_LOST_FOCUS:
      // render(app); //don't do it
      break;
      }
    }

static void initDisplay(android_app *app){
  AEngine* e = (AEngine*)app->userData;

  const EGLint attribs[] = {
    EGL_SURFACE_TYPE,
    EGL_WINDOW_BIT,
    EGL_BLUE_SIZE, 4,
    EGL_GREEN_SIZE, 4,
    EGL_RED_SIZE, 4,
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

  ANativeWindow_setBuffersGeometry( app->window, 0, 0, format);

  ini_surface = eglCreateWindowSurface( ini_display, config, app->window, NULL);

  if( e->context==EGL_NO_CONTEXT ){
    const EGLint attrib_list[] = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE};
    ini_context = eglCreateContext(ini_display, config, NULL, attrib_list);
    } else {
    ini_context = e->context;
    }

  if (eglMakeCurrent(ini_display, ini_surface, ini_surface, ini_context) == EGL_FALSE) {
    LOGE("Unable to eglMakeCurrent");
    return;
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

  //return true;
  }

static void termDisplay( android_app * a, bool killContext ) {
  AEngine* e = (AEngine*)a->userData;
  //isContextAviable = 0;

  if( e->display != EGL_NO_DISPLAY ) {
    eglMakeCurrent( e->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT );

    if( e->context != EGL_NO_CONTEXT && killContext ){
      eglDestroyContext( e->display, e->context );
      }

    if( e->surface != EGL_NO_SURFACE ){
      eglDestroySurface( e->display, e->surface );
      }

    if( killContext )
      eglTerminate(e->display);
    }

  e->display = EGL_NO_DISPLAY;
  if( killContext )
    e->context = EGL_NO_CONTEXT;
  e->surface = EGL_NO_SURFACE;
  //android->activity->vm->DetachCurrentThread();
  }


static void render( android_app * a ) {
  AEngine* e = (AEngine*)a->userData;

  if( e && e->display!= EGL_NO_DISPLAY && e->wnd ){
    int w = e->window_w,
        h = e->window_h;

    eglQuerySurface(e->display, e->surface, EGL_WIDTH,  &w);
    eglQuerySurface(e->display, e->surface, EGL_HEIGHT, &h);

    if( e->forceToResize ||
      (e->window_w!=w || e->window_h!=h) ){
      e->window_w = w;
      e->window_h = h;

      if( e->forceToResize  ){
        SizeEvent s(w,h);
        e->wnd->resizeEvent(s);
        e->wnd->resize(w,h);
        e->forceToResize = 0;
        } else {
        e->wnd->resize( w,h );
        }
      }

    e->wnd->render();
    }
  }
#endif
