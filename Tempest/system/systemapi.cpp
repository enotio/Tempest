#include "systemapi.h"

#include "system/windowsapi.h"
#include "system/androidapi.h"

#include <Tempest/Window>
#include <cstring>
#include <iostream>
#include <locale>
#include <Tempest/Buffer>
#include <Tempest/File>

#include "thirdparty/utf8cpp/utf8.h"

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
  ImageCodec::installStdCodecs(*this);
  }

Size SystemAPI::screenSize() {
  return instance().implScreenSize();
  }

std::string SystemAPI::loadText(const std::string &file) {
  return std::move( instance().loadText( file.data() ));
  }

std::string SystemAPI::loadText(const std::u16string &file) {
  return std::move( instance().loadText( file.data() ));
  }

std::string SystemAPI::loadText(const char *file) {
  return std::move( instance().loadTextImpl(file) );
  }

std::string SystemAPI::loadText(const char16_t *file) {
  return std::move( instance().loadTextImpl(file) );
  }

std::vector<char> SystemAPI::loadBytes(const char *file) {
  return std::move( instance().loadBytesImpl(file) );
  }

std::vector<char> SystemAPI::loadBytes(const char16_t *file) {
  return std::move( instance().loadBytesImpl(file) );
  }

bool SystemAPI::writeBytes( const char *file,
                            const std::vector<char> &f ) {
  return instance().writeBytesImpl(file, f);
  }

bool SystemAPI::writeBytes( const char16_t *file,
                            const std::vector<char> &f ) {
  return instance().writeBytesImpl(file, f);
  }


SystemAPI::File *SystemAPI::fopen( const char16_t *fname, const char *mode ) {
  return instance().fopenImpl(fname, mode);
  }

SystemAPI::File *SystemAPI::fopen( const char *fname, const char *mode ) {
  return instance().fopenImpl(fname, mode);
  }

size_t SystemAPI::readData(SystemAPI::File *f, char *dest, size_t count) {
  if( f )
    return instance().readDataImpl(f, dest, count);
  return 0;
  }

size_t SystemAPI::writeData(SystemAPI::File *f, const char *src, size_t count) {
  if( f )
    return instance().writeDataImpl(f, src, count);
  return 0;
  }

void SystemAPI::flush(SystemAPI::File *f) {
  if( f )
    instance().flushImpl(f);
  }

size_t SystemAPI::peek(SystemAPI::File *f, size_t skip, char *dest, size_t count) {
  if( f )
    return instance().peekImpl(f, skip, dest, count);

  return 0;
  }

size_t SystemAPI::skip(SystemAPI::File *f, size_t count) {
  if( f )
    return instance().skipImpl(f, count);

  return 0;
  }

bool SystemAPI::eof(SystemAPI::File *f) {
  if( f )
    return instance().eofImpl(f);

  return 0;
  }

void SystemAPI::fclose(SystemAPI::File *file) {
  if( file )
    instance().fcloseImpl(file);
  }

size_t SystemAPI::fsize(SystemAPI::File *f) {
  if( f )
    return instance().fsizeImpl(f);

  return 0;
  }

SystemAPI::File *SystemAPI::fopenImpl( const char *fname, const char *mode ) {
  return (SystemAPI::File*)::fopen(fname, mode);
  }

size_t SystemAPI::readDataImpl(SystemAPI::File *f, char *dest, size_t count) {
  return fread( dest, 1, count, (FILE*)f );
  }

size_t SystemAPI::writeDataImpl(SystemAPI::File *f, const char *data, size_t count) {
  return fwrite(data, 1, count, (FILE*)f );
  }

void SystemAPI::flushImpl(SystemAPI::File *f) {
  fflush( (FILE*)f );
  }

size_t SystemAPI::peekImpl(SystemAPI::File *f, size_t skip, char *dest, size_t count) {
  size_t pos = ftell((FILE*)f);
  fseek( (FILE*)f, skip, SEEK_CUR );
  size_t c = fread( dest, 1, count, (FILE*)f );
  fseek( (FILE*)f,  pos, SEEK_SET );
  return c;
  }

size_t SystemAPI::skipImpl(SystemAPI::File *f, size_t count) {
  size_t pos = ftell((FILE*)f);
  fseek( (FILE*)f, count, SEEK_CUR );

  return ftell((FILE*)f) - pos;
  }

bool SystemAPI::eofImpl(SystemAPI::File *f) {
  return feof((FILE*)f);
  }

SystemAPI::File *SystemAPI::fopenImpl( const char16_t *fname, const char *mode ) {
  return (SystemAPI::File*)::fopen( toUtf8(fname).data(), mode);
  }

void SystemAPI::fcloseImpl(SystemAPI::File *file) {
  ::fclose( (FILE*)file );
  }

size_t SystemAPI::fsizeImpl(SystemAPI::File *f) {
  FILE *file = (FILE*)f;
  size_t pos = ftell(file);

  fseek( file, 0, SEEK_SET );
  size_t s = ftell(file);

  fseek( file, 0, SEEK_END );
  size_t e = ftell(file);

  fseek( file, pos, SEEK_SET );
  return e-s;
  }

std::string SystemAPI::loadTextImpl(const char *file) {
  RFile f(file);
  size_t sz = f.size();

  std::string str;
  str.resize(sz);
  f.readData( &str[0], sz );

  return std::move(str);
  }

std::string SystemAPI::loadTextImpl(const char16_t *file) {
  RFile f(file);
  size_t sz = f.size();

  std::string str;
  str.resize(sz);
  f.readData( &str[0], sz );

  return std::move(str);
  }

std::vector<char> SystemAPI::loadBytesImpl(const char *file) {
  RFile f(file);
  size_t sz = f.size();

  std::vector<char> str;
  str.resize(sz);
  f.readData( &str[0], sz );

  return std::move(str);
  }

std::vector<char> SystemAPI::loadBytesImpl(const char16_t *file) {
  RFile f(file);
  size_t sz = f.size();

  std::vector<char> str;
  str.resize(sz);
  f.readData( &str[0], sz );

  return std::move(str);
  }

bool SystemAPI::loadImage( IDevice &file,
                           ImageCodec::ImgInfo &info,
                           std::vector<unsigned char> &out ) {
  return instance().loadImageImpl( file, info, out );
  }

bool SystemAPI::saveImage( ODevice& file,
                           const ImageCodec::ImgInfo &info,
                           const std::vector<unsigned char> &in ) {
  return instance().saveImageImpl( file, info, in );
  }

struct SystemAPI::GestureDeleter {
  void operator()( AbstractGestureEvent* x ){
    if( x )
      x->owner().deleteGesture(x);
    }
  };

void SystemAPI::emitEventImpl( Tempest::Window *w,
                               Event& e ){
  for( size_t i=w->overlayCount(); i>0; ){
    --i;

    processEvents( &w->overlay( i ), e );
    if( e.isAccepted() )
      return;
    }
  processEvents(w, e );
  }

void SystemAPI::emitEvent( Tempest::Window *w, Event &e ){
  if( e.type()==Event::MouseDown ||
      e.type()==Event::MouseUp   ||
      e.type()==Event::MouseMove ||
      e.type()==Event::MouseWheel ){
    MouseEvent& me = (MouseEvent&)e;

    if( w->pressedC.size() < size_t(me.mouseID+1) )
      w->pressedC.resize(me.mouseID+1);

    std::unique_ptr<AbstractGestureEvent, GestureDeleter> eg;
    eg.reset( w->sendEventToGestureRecognizer(me) );

    if( eg ){
      std::unique_ptr<AbstractGestureEvent, GestureDeleter> e = std::move(eg);
      emitEventImpl( w, *e );
      }

    if( e.type()==Event::MouseDown ){
      w->pressedC[me.mouseID] = 1;
      } else
    if( e.type()==Event::MouseUp ){
      w->pressedC[me.mouseID] = 0;
      }
    }

  if( e.type()==Event::MouseMove ){
    MouseEvent& me = (MouseEvent&)e;
    if( w->pressedC[me.mouseID] ){
      me.setType(Event::MouseDrag);
      emitEventImpl(w, me);

      if( e.isAccepted() )
        return;
      }

    me.setType(Event::MouseMove);
    emitEventImpl(w, me);
    return;
    }

  emitEventImpl(w, e);
  }

void SystemAPI::emitEvent( Tempest::Window *w,
                           const KeyEvent &ebase,
                           const KeyEvent &scut,
                           Event::Type type ) {
  bool scAcepted = false;
  if( type==Event::KeyDown ){
    Tempest::KeyEvent sc( scut.key, scut.u16, Event::Shortcut );
    SystemAPI::emitEvent(w, sc);

    scAcepted = sc.isAccepted();
    }

  if( 1||scAcepted ){
    Tempest::KeyEvent ed( ebase.key, ebase.u16, type );

    if( ebase.key!=Tempest::KeyEvent::K_NoKey )
      SystemAPI::emitEvent( w, ed );
    }
  }

void SystemAPI::processEvents( Widget *w,
                               Event &e ) {
  switch( e.type() ){
    case Event::NoEvent:
    case Event::Resize:
    case Event::Paint:
      break;

    case Event::Gesture:
      w->rootGestureEvent( (AbstractGestureEvent&)e );
      break;

    case Event::MouseDown:
      w->rootMouseDownEvent( (MouseEvent&)e );
      break;
    case Event::MouseUp:
      w->rootMouseUpEvent( (MouseEvent&)e );
      break;
    case Event::MouseMove:
      w->rootMouseMoveEvent( (MouseEvent&)e );
      break;
    case Event::MouseWheel:
      w->rootMouseWheelEvent( (MouseEvent&)e );
      break;
    case Event::MouseDrag:
      w->rootMouseDragEvent( (MouseEvent&)e );
      break;

    case Event::KeyDown:
      w->rootKeyDownEvent( (KeyEvent&)e );
      break;
    case Event::KeyUp:
      w->rootKeyUpEvent( (KeyEvent&)e );
      break;
    case Event::Shortcut:
      w->rootShortcutEvent( (KeyEvent&)e );
      break;

    case Event::Close:
      w->rootCloseEvent( (CloseEvent&)e );
      break;

    case Event::Custom:
      w->customEvent( (CustomEvent&)e );
      break;
    }
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
  w->setShowMode( Tempest::Window::ShowMode(mode) );
  }

void SystemAPI::activateEvent(Tempest::Window *w, bool a) {
  w->isAppActive = a;
  }

SystemAPI::GraphicsContexState SystemAPI::isGraphicsContextAviable( Tempest::Window *) {
  return Aviable;
  }

std::string SystemAPI::toUtf8(const std::u16string &str) {
  using namespace utf8::unchecked;

  std::string r;
  utf16to8( str.begin(), str.end(), std::back_inserter(r) );

  return std::move(r);
  }

std::u16string SystemAPI::toUtf16(const std::string &str) {
  using namespace utf8::unchecked;

  std::u16string r;
  utf8to16( str.begin(), str.end(), std::back_inserter(r) );

  return std::move(r);
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
  WFile fout(file);
  return fout.writeData(f.data(), f.size())==f.size();
  }

bool SystemAPI::writeBytesImpl( const char16_t *file,
                                const std::vector<char> &f ) {
  WFile fout(file);
  return fout.writeData(f.data(), f.size())==f.size();
  }

bool SystemAPI::saveImageImpl( ODevice& file,
                               const ImageCodec::ImgInfo &info,
                               const std::vector<unsigned char> &out) {
  for( size_t i=0; i<codecs.size(); ++i ){
    if( codecs[i]->canSave(info) && codecs[i]->save(file, info, out) ){
      return 1;
      }
    }

  return 0;
  }

bool SystemAPI::loadImageImpl( Tempest::IDevice &imgBytes,
                               ImageCodec::ImgInfo &info,
                               std::vector<unsigned char> &out ) {
  for( size_t i=0; i<codecs.size(); ++i ){
    info = ImageCodec::ImgInfo();

    Tempest::PeekReader reader(imgBytes);
    if( codecs[i]->load(reader, info, out) ){
      reader.commit();
      return 1;
      }
    }

  return 0;
  }

const std::string &SystemAPI::androidActivityClassImpl() {
  static const std::string cls = "com/tempest/TempestActivityBase";
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

void SystemAPI::addOverlay(Tempest::Window *w, WindowOverlay *ov) {
  w->addOverlay(ov);
  }

SystemAPI::Window *SystemAPI::handle( Tempest::Window &owner) {
  return owner.handle();
  }
