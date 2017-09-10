#include "androidapi.h"

using namespace Tempest;

#ifdef __ANDROID__

#include <Tempest/Application>
#include <Tempest/Window>
#include <Tempest/Event>
#include <Tempest/Log>
#include <Tempest/Assert>
#include <Tempest/DisplaySettings>
#include <Tempest/JniExtras>

#include <thread>
#include <map>
#include <queue>

#include <EGL/egl.h>
#include <GLES/gl.h>

#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dlfcn.h>
#include <limits.h>

#include <jni.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

#include <android/bitmap.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>

#include <android/keycodes.h>
#include <android/obb.h>

#ifdef __arm__
#include <machine/cpu-features.h>
//extern int android_getCpuCount(void);
#endif

#include <cmath>
#include <locale>

Tempest::signal<> AndroidAPI::onSurfaceDestroyed;

struct Guard {
  Guard( pthread_mutex_t& m ):m(m){
    pthread_mutex_lock(&m);
    }

  ~Guard(){
    pthread_mutex_unlock(&m);
    }

  pthread_mutex_t& m;
  };

static struct Android {
  std::unordered_map<jobject, Tempest::Window*> wndWx;

  JavaVM     *vm =nullptr;
  JNIEnv     *env=nullptr;
  //Jni::Object assets;
  AAssetManager* assets=nullptr;

  Jni::Class  activityClass;
  Jni::Class  surfaceClass;
  Jni::Class  tempestClass;

  Jni::Class  applicationClass;
  Jni::Object applicationObject;

  pthread_mutex_t appMutex, assetsPreloadedMutex;

  int (*mainFunc)(int,char**) = nullptr;
  std::string mainFnArgs;

  bool isPaused=true;

  std::vector<char> internal, external;
  std::string locale;
  float       density;

  enum MainThreadMessage {
    MSG_NONE = 0,
    MSG_WINDOW_SET,
    MSG_SURFACE_RESIZE,
    MSG_START,
    MSG_STOP,
    MSG_TOUCH,
    MSG_PAUSE,
    MSG_RESUME,
    MSG_CHAR,
    MSG_KEYDOWN_EVENT,
    MSG_KEYUP_EVENT,
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

    struct {
      union {
        int x,w;
        void* ptr;
        };
      union {
        int y,h;
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

  void msgClear(){
    Guard g( appMutex );
    (void)g;
    msg.clear();
    }

  size_t msgSize(){
    Guard g( appMutex );
    (void)g;
    return msg.size();
    }

  void waitForQueue(){
    while(true){
      if(msgSize()==0)
        return;
      sleep();
      }
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

  AAsset* open( const std::string& a ){
    //JNIEnv * env = 0;
    //vm->AttachCurrentThread( &env, NULL);

    AAsset* asset = AAssetManager_open(assets, a.c_str(), AASSET_MODE_UNKNOWN);
    return asset;
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

  static void sleep(){
    Application::sleep(10);
    }

  Android(){
    density = 1.5;

    msg.  reserve(256);
    mouse.reserve(8);

    pthread_mutex_init(&appMutex, 0);
    pthread_mutex_init(&assetsPreloadedMutex, 0);
    }

  ~Android(){
    pthread_mutex_destroy(&assetsPreloadedMutex);
    pthread_mutex_destroy(&appMutex);
    }

  SystemAPI::Window *createWindow();
  } android;

using namespace Tempest;

AndroidAPI::AndroidAPI(){
  TranslateKeyPair k[] = {
    { AKEYCODE_DPAD_LEFT,   Event::K_Left   },
    { AKEYCODE_DPAD_RIGHT,  Event::K_Right  },
    { AKEYCODE_DPAD_UP,     Event::K_Up     },
    { AKEYCODE_DPAD_DOWN,   Event::K_Down   },

    //{ AKEYCODE_BACK, Event::K_ESCAPE },
    { AKEYCODE_DEL,  Event::K_Back   },
    { AKEYCODE_HOME, Event::K_Home   },

    { AKEYCODE_0,      Event::K_0  },
    { AKEYCODE_A,      Event::K_A  },
    { AKEYCODE_ENTER,  Event::K_Return },

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

Jni::Object AndroidAPI::surface(jobject activity) {
  while( true ) {
    jobject obj = android.activityClass.callObject(*android.env,activity,"nativeSurface","()Landroid/view/Surface;");
    if( obj!=nullptr ) {
      return Jni::Object(*android.env,obj);
      }
    if( android.env->ExceptionOccurred() )
      return Jni::Object();
    std::this_thread::yield();
    }
  }

Jni::AndroidWindow AndroidAPI::nWindow(void* hwnd) {
  Jni::Object jsurface = AndroidAPI::surface(reinterpret_cast<jobject>(hwnd));
  JNIEnv* env = jenvi();

  Jni::AndroidWindow wnd;
  while(true) {
    wnd = Jni::AndroidWindow(*env,jsurface);
    if( wnd )
      return wnd;
    std::this_thread::yield();
    Application::sleep(10);
    }
  return Jni::AndroidWindow();
  }

const Jni::Class& AndroidAPI::appClass() {
  return android.activityClass;
  }

Jni::Object AndroidAPI::app() {
  return android.applicationObject;
  }

const char *AndroidAPI::internalStorage() {
  mkdir( android.internal.data(), 0770 );
  return android.internal.data();
  }

const char *AndroidAPI::externalStorage() {
  mkdir( android.external.data(), 0770 );
  return android.external.data();
  }

float AndroidAPI::densityDpi(){
  return android.density;
  }

void AndroidAPI::toast( const std::string &s ) {
  Android &a = android;

  jstring str = a.env->NewStringUTF( s.c_str() );
  a.activityClass.callStaticVoid(*a.env,"showToast", "(Ljava/lang/String;)V");
  a.env->DeleteLocalRef(str);
  }

void AndroidAPI::showSoftInput() {
  Android &a = android;
  if( android.wndWx.empty() )
    return;

  jobject w = android.wndWx.begin()->first;
  a.surfaceClass.callVoid(*a.env,w,"showSoftInput", "()V");
  }

void AndroidAPI::hideSoftInput() {
  Android &a = android;
  a.activityClass.callStaticVoid(*a.env,"hideSoftInput", "()V");
  }

void AndroidAPI::toggleSoftInput(){
  Android &a = android;
  a.activityClass.callStaticVoid(*a.env,"toggleSoftInput", "()V");
  }

const std::string &AndroidAPI::iso3Locale() {
  return android.locale;
  }

AndroidAPI::Window* AndroidAPI::createWindow(int /*w*/, int /*h*/) {
  return android.createWindow();
  }

AndroidAPI::Window* AndroidAPI::createWindowMaximized(){
  return android.createWindow();
  }

AndroidAPI::Window* AndroidAPI::createWindowMinimized(){
  return android.createWindow();
  }

AndroidAPI::Window* AndroidAPI::createWindowFullScr(){
  return android.createWindow();
  }

Widget *AndroidAPI::addOverlay(WindowOverlay *ov) {
  if( android.wndWx.empty() ){
    delete ov;
    return 0;
    }

  Tempest::Window* w = android.wndWx.begin()->second;
  SystemAPI::addOverlay(w, ov);
  return ov;
  }

bool AndroidAPI::testDisplaySettings( Window* , const DisplaySettings & s ) {
  return s.bits==16 &&
         s.fullScreen;
  }

bool AndroidAPI::setDisplaySettings( Window* w, const DisplaySettings &s) {
  return testDisplaySettings(w,s);
  }

AndroidAPI::CpuInfo AndroidAPI::cpuInfoImpl(){
  CpuInfo info;
  memset(&info, 0, sizeof(info));

#ifdef __arm__
  info.cpuCount = 1;//android_getCpuCount();
#else
  info.cpuCount = 1;
#endif
  return info;
  }

struct AndroidAPI::DroidFile {
  FILE*   h    =nullptr;
  AAsset* asset=nullptr;
  size_t  size =0;
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
    } else
  if( !wr ) {
    f->asset = android.open(fname);
    if( f->asset )
      f->size = size_t(AAsset_getLength64(f->asset));
    }

  if( !f->h && !f->asset ){
    delete f;
    return 0;
    }

  return reinterpret_cast<SystemAPI::File*>(f);
  }

size_t AndroidAPI::readDataImpl(SystemAPI::File *f, char *dest, size_t count) {
  DroidFile *fn = reinterpret_cast<DroidFile*>(f);
  if( fn->h ){
    return fread(dest, 1, count, fn->h);
    } else {
    int rd = AAsset_read(fn->asset,dest,count);
    if( rd<0 )
      return 0;
    return size_t(rd);
    }
  }

size_t AndroidAPI::peekImpl(SystemAPI::File *f, size_t skip, char *dest, size_t count) {
  DroidFile *fn = reinterpret_cast<DroidFile*>(f);
  if( fn->h ){
    off_t pos = ftell( fn->h );
    fseek( fn->h, skip, SEEK_CUR );
    size_t c = fread( dest, 1, count, fn->h );
    fseek( fn->h , pos, SEEK_SET );
    return c;
    } else {
    size_t pos = size_t(fn->size - AAsset_getRemainingLength64(fn->asset));
    AAsset_seek(fn->asset,skip,SEEK_CUR);
    size_t c   = readDataImpl(f,dest,count);

    AAsset_seek(fn->asset,pos,SEEK_SET);
    return c;
    }
  }

size_t AndroidAPI::writeDataImpl(SystemAPI::File *f, const char *data, size_t count) {
  DroidFile *fn = reinterpret_cast<DroidFile*>(f);
  return fwrite(data, 1, count, fn->h );
  }

void AndroidAPI::flushImpl(SystemAPI::File *f) {
  DroidFile *fn = reinterpret_cast<DroidFile*>(f);
  if( fn->h )
    fflush( fn->h );
  }

size_t AndroidAPI::skipImpl(SystemAPI::File *f, size_t count) {
  DroidFile *fn = reinterpret_cast<DroidFile*>(f);

  if( fn->h ){
    off_t pos = ftell( fn->h );
    fseek( fn->h, off_t(count), SEEK_CUR );

    return ftell(fn->h) - pos;
    } else {
    size_t pos    = size_t(fn->size - AAsset_getRemainingLength64( fn->asset ));
    off_t  newPos = AAsset_seek(fn->asset,count,SEEK_CUR);
    if(newPos<0)
      return 0;

    return newPos-pos;
    }
  }

bool AndroidAPI::eofImpl(SystemAPI::File *f) {
  DroidFile *fn = reinterpret_cast<DroidFile*>(f);
  if( fn->h ){
    return feof( fn->h );
    } else {
    return AAsset_getRemainingLength64(fn->asset)==0;
    }

  return fn->size==0;
  }

size_t AndroidAPI::fsizeImpl( File* f ){
  DroidFile *fn = reinterpret_cast<DroidFile*>(f);
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

  if( fn->asset )
    return fn->size;
  return 0;
  }

void AndroidAPI::fcloseImpl(SystemAPI::File *f) {
  DroidFile *fn = reinterpret_cast<DroidFile*>(f);

  if( fn->h ){
    ::fclose( fn->h );
    }

  if( fn->asset )
    AAsset_close(fn->asset);

  delete fn;
  }

Size AndroidAPI::implScreenSize() {
  return Size();//android.window_w, android.window_h);
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

int AndroidAPI::nextEvents(bool &quit) {
  int r = 0;
  while( !quit ){
    int sz = 0;
    {
      Android::MainThreadMessage msg = android.takeMsg().msg;

      if( android.wndWx.size()==0 && msg!=Android::MSG_RENDER_LOOP_EXIT  ){
        android.sleep();
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
  bool hasWindow = true;
  Android::Message msg = Android::MSG_NONE;

  {
  Guard g(android.appMutex);
  (void)g;
  hasWindow = !android.wndWx.empty();//android.window;

  if( android.msg.size() ){
    msg = android.msg[0];

    if( !hasWindow && msg.msg!=Android::MSG_RENDER_LOOP_EXIT ){
      android.sleep();
      return 0;
      }

    android.msg.erase( android.msg.begin() );
    }
  }

  if( android.wndWx.size()==0 )
    return 0;

  Tempest::Window* wnd = android.wndWx.begin()->second;

  switch( msg.msg ) {
    case Android::MSG_WINDOW_SET:
      if(msg.data.ptr) {
        SystemAPI::activateEvent( wnd, true);
        } else {
        onSurfaceDestroyed();
        SystemAPI::activateEvent( wnd, false);
        }
      break;
    case Android::MSG_SURFACE_RESIZE:
      SystemAPI::sizeEvent(wnd,msg.data.w,msg.data.h);
      break;

    case Android::MSG_RENDER_LOOP_EXIT:
      quit = true;
      break;

    case Android::MSG_CHAR:
    {
       Tempest::KeyEvent e = Tempest::KeyEvent( uint32_t(msg.data1) );

       int wrd[3] = {
         AKEYCODE_DEL,
         AKEYCODE_BACK,
         0
         };

       if( 0 == *std::find( wrd, wrd+2, msg.data1) ){
         Tempest::KeyEvent ed( e.key, e.u16, Event::KeyDown );
         SystemAPI::emitEvent( wnd, ed);

         Tempest::KeyEvent eu( e.key, e.u16, Event::KeyUp );
         SystemAPI::emitEvent( wnd, eu);
         }
    }

    case Android::MSG_KEYDOWN_EVENT:{
      SystemAPI::emitEvent( wnd,
                            makeKeyEvent(msg.data1),
                            makeKeyEvent(msg.data1, true),
                            Event::KeyDown );
      }
      break;

    case Android::MSG_KEYUP_EVENT:{
      Tempest::KeyEvent e = makeKeyEvent(msg.data1);

      Tempest::KeyEvent eu( e.key, e.u16, Event::KeyUp );
      SystemAPI::emitEvent( wnd, eu);
      }
      break;

    case Android::MSG_TOUCH: {
      int id = android.pointerId( msg.data2 );

      if( msg.data1==0 ){
        MouseEvent e( msg.data.x, msg.data.y,
                      MouseEvent::ButtonLeft, 0, id,
                      Event::MouseDown );
        android.pointerPos(id) = Point(msg.data.x, msg.data.y);
        SystemAPI::emitEvent(wnd, e);
        }

      if( msg.data1==1 ){
        MouseEvent e( msg.data.x, msg.data.y,
                      MouseEvent::ButtonLeft, 0, id,
                      Event::MouseUp );
        SystemAPI::emitEvent(wnd, e);
        android.unsetPointer(msg.data2);
        }

      if( msg.data1==2 ){
        MouseEvent e( msg.data.x, msg.data.y,
                      MouseEvent::ButtonLeft, 0, id,
                      Event::MouseMove );
        if( android.pointerPos(id) != Point(msg.data.x, msg.data.y) ){
          android.pointerPos(id) = Point(msg.data.x, msg.data.y);
          SystemAPI::emitEvent(wnd, e);
          }
        }
      }
      break;

    case Android::MSG_CLOSE:  {
      Tempest::CloseEvent e;
      SystemAPI::emitEvent(wnd, e);
      if( !e.isAccepted() ){
        android.pushMsg( Android::MSG_RENDER_LOOP_EXIT );
        }
      }
      break;

    case Android::MSG_START:
      SystemAPI::setShowMode(wnd, Tempest::Window::Maximized);
      break;

    case Android::MSG_STOP:
      SystemAPI::setShowMode(wnd, Tempest::Window::Minimized);
      break;

    case Android::MSG_PAUSE:
      android.isPaused = true;
      break;

    case Android::MSG_RESUME:
      android.isPaused = false;
      break;

    case Android::MSG_NONE:
      if( !android.isPaused && wnd->showMode()!=Tempest::Window::Minimized)
        render(); else
        android.sleep();
      break;

    default:
      break;
    }

  return 0;
  }

Point AndroidAPI::windowClientPos ( Window* ){
  return Point(0,0);
  }

Size AndroidAPI::windowClientRect( Window* hwnd ){
  Jni::AndroidWindow window = AndroidAPI::nWindow(hwnd);
  EGLint w=ANativeWindow_getWidth (window);
  EGLint h=ANativeWindow_getHeight(window);
  return Size(w,h);
  }

void AndroidAPI::deleteWindow( Window *w ) {
  if( w!=nullptr ) {
    jobject ptr = reinterpret_cast<jobject>(w);
    android.activityClass.callStaticVoid(*android.env,"nativeDelWindow","(Lcom/tempest/engine/Activity;)V",ptr);
    android.env->DeleteGlobalRef(ptr);
    android.wndWx.erase(ptr);
    }
  }

void AndroidAPI::show(Window *hwnd) {
  ANativeWindow*   window = AndroidAPI::nWindow(hwnd);
  jobject          ptr    = reinterpret_cast<jobject>(hwnd);
  Tempest::Window* wnd    = nullptr;

  auto i = android.wndWx.find(ptr);

  if( i!= android.wndWx.end() )
    wnd = i->second;

  EGLint w=ANativeWindow_getWidth (window);
  EGLint h=ANativeWindow_getHeight(window);
  if( wnd )
    SystemAPI::sizeEvent(wnd, w, h);
  }

void AndroidAPI::setGeometry( Window */*hw*/, int /*x*/, int /*y*/, int /*w*/, int /*h*/ ) {
  }

void AndroidAPI::bind( Window *hwnd, Tempest::Window *wx ) {
  jobject w = reinterpret_cast<jobject>(hwnd);
  android.wndWx[w] = wx;
  SystemAPI::activateEvent(wx, true);
  }

static void render() {
  Android& e = android;
  for(auto& i:e.wndWx) {
    Tempest::Window& wnd=*i.second;
    if( !wnd.size().isEmpty() )
      if( wnd.showMode()!=Tempest::Window::Minimized && wnd.isActive() )
        wnd.render();
    }
  }

AndroidAPI::Window* Android::createWindow() {
  JNIEnv* e = android.env;
  if( !e )
    return nullptr;

  while( true ) {
    jobject obj = android.activityClass.callStaticObject(*e,"nativeNewWindow","()Lcom/tempest/engine/Activity;");
    if(!obj)
      continue;
    obj = env->NewGlobalRef(obj);
    return reinterpret_cast<AndroidAPI::Window*>(obj);
    }
  }

static void* tempestMainFunc(void*);

static void JNICALL resize( JNIEnv * , jobject, jobject, jint w, jint h ) {
  Android::Message m = Android::MSG_SURFACE_RESIZE;
  m.data.w = w;
  m.data.h = h;
  android.pushMsg(m);
  }

static void JNICALL start(JNIEnv* /*jenv*/, jobject /*obj*/) {
  Log::i("nativeOnStart");
  android.pushMsg(Android::MSG_START);
  }

static void JNICALL stop(JNIEnv* /*jenv*/, jobject /*obj*/) {
  Log::i("nativeOnStop");
  android.pushMsg(Android::MSG_STOP);
  }

static void JNICALL resume(JNIEnv* /*jenv*/, jobject /*obj*/) {
  Log::i("nativeOnResume");
  android.pushMsg( Android::MSG_RESUME );
  }

static void JNICALL pauseA(JNIEnv* /*jenv*/, jobject /*obj*/) {
  Log::i("nativeOnPause");
  android.msg.push_back( Android::MSG_PAUSE );
  }

static void JNICALL setAssets(JNIEnv* jenv, jobject /*obj*/, jobject jassets ) {
  Log::i("setAssets");
  android.assets = AAssetManager_fromJava(jenv, jassets);
  }

static void JNICALL nativeOnTouch( JNIEnv* , jobject ,
                                   jint x, jint y, jint act, jint pid ){
  Android::Message m = Android::MSG_TOUCH;
  m.data.x = x;
  m.data.y = y;
  m.data1  = act;
  m.data2  = pid;

  if( act!=2 || android.msgSize()<64 )
    android.pushMsg(m);
  }

static void JNICALL nativeSetSurface( JNIEnv* /*jenv*/, jobject /*obj*/, jobject surface) {
  Guard g( android.appMutex );
  (void)g;

  Android::Message m = Android::MSG_WINDOW_SET;
  m.data.ptr = surface;
  android.msg.push_back( m );
  }

static void JNICALL onKeyDownEvent(JNIEnv* , jobject, jint key ) {
  Log::i("onKeyDownEvent");

  if( key!=AKEYCODE_BACK ){
    const int K_a = AKEYCODE_A, K_A = 29;
    if( K_a<=key && key<K_a+26 ){
      Android::Message m = Android::MSG_CHAR;
      m.data1 = key+97-K_a;
      android.pushMsg(m);
      } else
    if( K_A<=key && key<K_A+26 ){
      Android::Message m = Android::MSG_CHAR;
      m.data1 = key+65-K_A;
      android.pushMsg(m);
      } else
    if( AKEYCODE_0<=key && key<AKEYCODE_0+10 ){
      Android::Message m = Android::MSG_CHAR;
      m.data1 = key+48-AKEYCODE_0;
      android.pushMsg(m);
      } else {
      Android::Message m = Android::MSG_KEYDOWN_EVENT;
      m.data1 = key;
      android.pushMsg(m);
      }
    } else {
    android.pushMsg( Android::MSG_CLOSE );
    }
  }

static void JNICALL onKeyUpEvent(JNIEnv* , jobject, jint key ) {
  Log::i("onKeyUpEvent");

  if( key!=AKEYCODE_BACK ){
    Android::Message m = Android::MSG_KEYUP_EVENT;
    m.data1 = key;
    android.pushMsg(m);
    } else {
    android.pushMsg( Android::MSG_RENDER_LOOP_EXIT );
    }
  }

static void JNICALL onKeyCharEvent( JNIEnv* env, jobject,
                                    jstring k ) {
  const char* str = env->GetStringUTFChars( k, 0);
  if( str ){
    std::u16string s16 = SystemAPI::toUtf16(str);

    for( size_t i=0; i<s16.size(); ++i ){
      Android::Message m = Android::MSG_CHAR;
      m.data1 = s16[i];
      android.pushMsg(m);
      }
    }

  env->ReleaseStringUTFChars( k, str );
  }

static jint JNICALL nativeCloseEvent( JNIEnv* , jobject ){
  {
    Guard g( android.appMutex );
    (void)g;
    android.msg.push_back( Android::MSG_CLOSE );
    android.msg.push_back( Android::MSG_WAIT  );
  }
  android.waitForQueue();
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

static void JNICALL nativeSetApplication(JNIEnv* env, jobject, jobject app){
  android.applicationObject=Jni::Object(*env,app);
  android.applicationClass =Jni::Class(*env,env->GetObjectClass(app));
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

static void JNICALL setupDpi( JNIEnv* , jobject,
                              jfloat d ){
  android.density = d;
  }

static void JNICALL invokeMain(JNIEnv* env, jobject, jstring ndkLib ){
  android.mainFnArgs = env->GetStringUTFChars( ndkLib, 0);

  void* handle = dlopen(android.mainFnArgs.c_str(), RTLD_NOW);
  if( !handle )
    return;

  void** pptr=reinterpret_cast<void**>(&android.mainFunc);
  *pptr = dlsym(handle, "main");

  tempestMainFunc(nullptr);
  //android.thread_create(&android.mainThread, 0, tempestMainFunc, 0);
  }

extern "C" jint JNICALL JNI_OnLoad(JavaVM *vm, void */*reserved*/){
  Log::i("Tempest JNI_OnLoad");

  android.vm = vm;

  static std::initializer_list<JNINativeMethod> appMethodTable = {
    {"nativeSetApplication", "(Landroid/app/Application;)V",            (void *)nativeSetApplication },
    {"nativeInitLocale",     "(Ljava/lang/String;)V",                   (void *)nativeInitLocale     },
    {"nativeSetupDpi",       "(F)V",                                    (void *)setupDpi             },
    {"invokeMainImpl",       "(Ljava/lang/String;)V",                   (void *)invokeMain           },
    {"nativeSetAssets",      "(Landroid/content/res/AssetManager;)V",   (void *)setAssets            },
    {"nativeSetupStorage",   "(Ljava/lang/String;Ljava/lang/String;)V", (void *)nativeSetupStorage   }
    };

  static std::initializer_list<JNINativeMethod> surfaceMethodTable = {
    {"nativeOnStart",  "()V", (void *) start  },
    {"nativeOnResume", "()V", (void *) resume },
    {"nativeOnPause",  "()V", (void *) pauseA },
    {"nativeOnStop",   "()V", (void *) stop   },

    {"nativeOnTouch",     "(IIII)V", (void *)nativeOnTouch   },

    {"onKeyDownEvent",    "(I)V",                        (void *)onKeyDownEvent     },
    {"onKeyUpEvent",      "(I)V",                        (void *)onKeyUpEvent       },
    {"onKeyCharEvent",    "(Ljava/lang/String;)V",       (void *)onKeyCharEvent     },

    {"nativeCloseEvent",  "()I",                         (void *) nativeCloseEvent  },
    {"nativeSetSurface",  "(Landroid/view/Surface;)V",   (void *) nativeSetSurface  },
    {"nativeOnResize",    "(Landroid/view/Surface;II)V", (void *) resize            }
  };

  JNIEnv* env;
  if( vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK ){
    Log::i("Failed to get the environment");
    return -1;
    }

  android.activityClass = Jni::Class(*env,SystemAPI::androidActivityClass().c_str());
  android.surfaceClass  = Jni::Class(*env,"com/tempest/engine/WindowSurface");
  android.tempestClass  = Jni::Class(*env,"com/tempest/engine/Tempest");

  if( !android.surfaceClass.registerNatives(*env,surfaceMethodTable) |
      !android.tempestClass.registerNatives(*env,appMethodTable) ) {
    Log::i("failed to get ",SystemAPI::androidActivityClass().c_str()," class reference");

    jthrowable err = env->ExceptionOccurred();
    if( err ) {
      env->ExceptionDescribe();
      env->ExceptionClear();
      }
    }

  Log::i("Tempest ~JNI_OnLoad");
  return JNI_VERSION_1_6;
  }

static void* tempestMainFunc(void*){
  Android &a = android;
  a.vm->GetEnv(reinterpret_cast<void**>(&a.env),JNI_VERSION_1_6);

  pthread_setname_np(pthread_self(),"main");
  Log::i("Tempest MainFunc[1]");

  static char* ch[]={
    NULL,
    NULL
    };
  ch[0] = &android.mainFnArgs[0];
  android.mainFunc(1,ch);
  android.msgClear();

  Log::i("~Tempest MainFunc");
  return 0;
  }

#endif
