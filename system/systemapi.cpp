#include "systemapi.h"

#include "system/windowsapi.h"
#include "system/androidapi.h"

#include <Tempest/Window>
#include <cstring>
#include <iostream>
#include <locale>
#include <fstream>


using namespace Tempest;

SystemAPI &SystemAPI::instance() {
#ifdef __WIN32__
  static WindowsAPI api;
#endif

#ifdef __ANDROID__
  static AndroidAPI api;
#endif

  return api;
  }

SystemAPI::SystemAPI(){
  buffer.reserve( 4*1024*1024 );
  ImageCodec::installStdCodecs(*this);
  }

Size SystemAPI::screenSize() {
  return instance().implScreenSize();
  }

std::string SystemAPI::loadText(const std::string &file) {
  return std::move( instance().loadText( file.data() ));
  }

std::string SystemAPI::loadText(const std::wstring &file) {
  return std::move( instance().loadText( file.data() ));
  }

std::string SystemAPI::loadText(const char *file) {
  return std::move( instance().loadTextImpl(file) );
  }

std::string SystemAPI::loadText(const wchar_t *file) {
  return std::move( instance().loadTextImpl(file) );
  }

std::vector<char> SystemAPI::loadBytes(const char *file) {
  return std::move( instance().loadBytesImpl(file) );
  }

std::vector<char> SystemAPI::loadBytes(const wchar_t *file) {
  return std::move( instance().loadBytesImpl(file) );
  }

bool SystemAPI::writeBytes(const char *file, const std::vector<char> &f) {
  return instance().writeBytesImpl(file, f);
  }

bool SystemAPI::writeBytes(const wchar_t *file, const std::vector<char> &f) {
  return instance().writeBytesImpl(file, f);
  }

bool SystemAPI::loadImage(const wchar_t *file,
                           ImageCodec::ImgInfo &info,
                           std::vector<unsigned char> &out) {
  return instance().loadImageImpl( file, info, out );
  }

bool SystemAPI::loadImage(const char *file,
                           ImageCodec::ImgInfo &info,
                           std::vector<unsigned char> &out ) {
  return instance().loadImageImpl( file, info, out );
  }

bool SystemAPI::saveImage(const wchar_t *file,
                           ImageCodec::ImgInfo &info,
                           std::vector<unsigned char> &in ) {
  return instance().saveImageImpl( file, info, in );
  }

bool SystemAPI::saveImage(const char *file,
                           ImageCodec::ImgInfo &info,
                           std::vector<unsigned char> &in ) {
  return instance().saveImageImpl( file, info, in );
  }

void SystemAPI::emitEvent( Tempest::Window *w,
                           KeyEvent& e,
                           Event::Type type ){
  if( type==Event::KeyDown ){
    processEvents(w, e, type);
    }
  if( type==Event::KeyUp ){
    processEvents(w, e, type);
    }
  if( type==Event::Shortcut ){
    processEvents(w, e, type);
    }
  }

void SystemAPI::emitEvent( Tempest::Window *w,
                           CloseEvent &e,
                           Event::Type type) {
  processEvents(w, e, type);
  }

struct SystemAPI::GestureDeleter {
  void operator()( AbstractGestureEvent* x ){
    if( x )
      x->owner().deleteGesture(x);
    }
  };

void SystemAPI::emitEvent( Tempest::Window *w, MouseEvent &e, Event::Type type ){
  if( w->pressedC.size() < size_t(e.mouseID+1) )
    w->pressedC.resize(e.mouseID+1);

  std::unique_ptr<AbstractGestureEvent, GestureDeleter> eg;
  eg.reset( w->sendEventToGestureRecognizer(e) );

  if( eg ){
    std::unique_ptr<AbstractGestureEvent, GestureDeleter> e = std::move(eg);
    processEvents(w, *e, type);
    }

  if( type==Event::MouseDown ){
    w->pressedC[e.mouseID] = 1;
    processEvents(w, e, type);
    }

  if( type==Event::MouseUp ){
    w->pressedC[e.mouseID] = 0;
    processEvents(w, e, type);
    }

  if( type==Event::MouseMove ){
    if( w->pressedC[e.mouseID] ){
      processEvents(w, e, Event::MouseDrag);

      if( e.isAccepted() )
        return;
      }

    processEvents(w, e, type);
    }

  if( type==Event::MouseWheel ){
    processEvents(w, e, type);
    }
  }

void SystemAPI::processEvents( Widget *w,
                               AbstractGestureEvent &e,
                               Event::Type /*type*/ ) {
  e.setType( Event::Gesture );
  w->rootGestureEvent(e);
  }


void SystemAPI::processEvents(Widget *w, MouseEvent &e, Event::Type type) {
  e.setType( type );

  if( type==Event::MouseDown )
    return w->rootMouseDownEvent(e);

  if( type==Event::MouseUp )
    return w->rootMouseUpEvent(e);

  if( type==Event::MouseMove )
    return w->rootMouseMoveEvent(e);

  if( type==Event::MouseWheel )
    return w->rootMouseWheelEvent(e);

  if( type==Event::MouseDrag)
    return w->rootMouseDragEvent(e);
  }

void SystemAPI::processEvents(Widget *w, KeyEvent &e, Event::Type type) {
  e.setType( type );

  if( type==Event::KeyDown )
    return w->rootKeyDownEvent(e);

  if( type==Event::KeyUp )
    return w->rootKeyUpEvent(e);

  if( type==Event::Shortcut )
    return w->rootShortcutEvent(e);
  }

void SystemAPI::processEvents(Widget *w, CloseEvent &e, Event::Type type) {
  if( type==Event::Close )
    return w->rootCloseEvent(e);
  }

void SystemAPI::moveEvent( Tempest::Window *w, int cX, int cY) {
  w->setPosition(cX, cY);
  }

void SystemAPI::sizeEvent( Tempest::Window *w, int cW, int cH ) {
  if( w->winW==cW &&
      w->winH==cH )
    return;

  w->winW = cW;
  w->winH = cH;

  if( w->isAppActive ){
    if( cW * cH ){
      w->resize( cW, cH );
      w->resizeIntent = false;
      }
    } else {
    w->resizeIntent = true;
    }
  }

void SystemAPI::setShowMode(Tempest::Window *w, int mode) {
  w->smode = Tempest::Window::ShowMode(mode);
  }

void SystemAPI::activateEvent(Tempest::Window *w, bool a) {
  w->isAppActive = a;
  }

SystemAPI::GraphicsContexState SystemAPI::isGraphicsContextAviable( Tempest::Window *) {
  return Aviable;
  }

std::string SystemAPI::toUtf8(const std::wstring &str) {
  std::string r;
  r.assign( str.begin(), str.end() );

  return r;
  }

std::wstring SystemAPI::toWstring(const std::string &str) {
  std::wstring r;
  r.assign( str.begin(), str.end() );

  return r;
  }

const std::string &SystemAPI::androidActivityClass() {
  return instance().androidActivityClassImpl();
  }

Event::KeyType SystemAPI::translateKey(uint64_t scancode) {
  KeyInf &k = instance().ki;

  for( size_t i=0; i<k.keys.size(); ++i )
    if( k.keys[i].src==scancode )
      return k.keys[i].result;

  for( size_t i=0; i<k.k0.size(); ++i )
    if( k.k0[i].src <= scancode &&
                       scancode <=k.k0[i].src+9 ){
      auto dx = ( scancode-k.k0[i].src );
      return Event::KeyType( k.k0[i].result + dx );
      }

  auto literalsCount = (Event::K_Z - Event::K_A);
  for( size_t i=0; i<k.a.size(); ++i )
    if( k.a[i].src <= scancode &&
                      scancode <= k.a[i].src+literalsCount ){
      auto dx = ( scancode-k.a[i].src );
      return Event::KeyType( k.a[i].result + dx );
      }

  for( size_t i=0; i<k.f1.size(); ++i )
    if( k.f1[i].src <= scancode &&
                       scancode <= k.f1[i].src+k.fkeysCount ){
      auto dx = ( scancode-k.f1[i].src );
      return Event::KeyType( k.f1[i].result + dx );
      }

  return Event::K_NoKey;
  }

void SystemAPI::installImageCodec( ImageCodec *codec ) {
  codecs.emplace_back( codec );
  }

bool SystemAPI::writeBytesImpl( const char *file,
                                const std::vector<char> &f ) {
  std::ofstream fout(file, std::ios::binary);
  if( fout )
    fout.write( f.data(), f.size() );
  return bool(fout);
  }

bool SystemAPI::writeBytesImpl( const wchar_t *file,
                                const std::vector<char> &f ) {
  return 0;
  }

bool SystemAPI::loadImageImpl( const char *file,
                               ImageCodec::ImgInfo &info,
                               std::vector<unsigned char> &out ) {
  bool ok = false;
  {
    Detail::Guard guard(byteBuffer);
    (void)guard;

    buffer = std::move( SystemAPI::loadBytes(file) );
    return loadImageImpl(buffer, info, out );
  }

  return ok;
  }

bool SystemAPI::saveImageImpl( const char *file,
                               ImageCodec::ImgInfo &info,
                               std::vector<unsigned char> &out) {
  for( size_t i=0; i<codecs.size(); ++i ){
    if( codecs[i]->canSave(info) && codecs[i]->save(file, info, out) ){
      return 1;
      }
    }

  return 0;
  }

bool SystemAPI::saveImageImpl( const wchar_t *file,
                               ImageCodec::ImgInfo &info,
                               std::vector<unsigned char> &out) {
  for( size_t i=0; i<codecs.size(); ++i ){
    if( codecs[i]->save(file, info, out) ){
      return 1;
      }
    }

  return 0;
  }

bool SystemAPI::loadImageImpl( const wchar_t *file,
                               ImageCodec::ImgInfo &info,
                               std::vector<unsigned char> &out) {
  bool ok = false;
  {
    Detail::Guard guard(byteBuffer);
    (void)guard;

    buffer = std::move( SystemAPI::loadBytes(file) );
    return loadImageImpl(buffer, info, out );
  }

  return ok;
  }

bool SystemAPI::loadImageImpl(const std::vector<char> &imgBytes,
                               ImageCodec::ImgInfo &info,
                               std::vector<unsigned char> &out ) {
  for( size_t i=0; i<codecs.size(); ++i ){
    info = ImageCodec::ImgInfo();
    if( codecs[i]->load(imgBytes, info, out) ){
      return 1;
      }
    }

  return 0;
  }

const std::string &SystemAPI::androidActivityClassImpl() {
  static const std::string cls = "com/tempest/engine/TempestActivity";
  return cls;
  }

void SystemAPI::setupKeyTranslate( const SystemAPI::TranslateKeyPair k[] ) {
  ki.keys.clear();
  ki.a. clear();
  ki.k0.clear();
  ki.f1.clear();

  for( size_t i=0; k[i].result!=Event::K_NoKey; ++i ){
    if( k[i].result==Event::K_A )
      ki.a.push_back(k[i]); else
    if( k[i].result==Event::K_0 )
      ki.k0.push_back(k[i]); else
    if( k[i].result==Event::K_F1 )
      ki.f1.push_back(k[i]); else
      ki.keys.push_back( k[i] );
    }

#ifndef __ANDROID__
  ki.keys.shrink_to_fit();
  ki.a. shrink_to_fit();
  ki.k0.shrink_to_fit();
  ki.f1.shrink_to_fit();
#endif
  }

void SystemAPI::setFuncKeysCount(int c) {
  ki.fkeysCount = c;
  }

size_t SystemAPI::imageCodecCount() const {
  return codecs.size();
  }

ImageCodec &SystemAPI::imageCodec(size_t id) {
  return *codecs[id];
  }

SystemAPI::CpuInfo SystemAPI::cpuInfo() {
  return instance().cpuInfoImpl();
  }
