#include "textureholder.h"

#include <Tempest/Texture2d>
#include <Tempest/Texture3d>

#include <Tempest/Device>
#include <Tempest/Pixmap>
#include <Tempest/Assert>

#include <map>

#include <stdexcept>

using namespace Tempest;

struct TextureHolder::Data {
  //std::map< AbstractAPI::Texture*, std::string > textures;

  struct DynTexture{
    int w, h;
    bool mip;
    TextureUsage usage;
    AbstractTexture::Format::Type format;
    AbstractAPI::Texture* owner;
    };

  struct PixmapTexture{
    Pixmap pm;
    bool mip, compress;
    AbstractAPI::Texture* owner;
    };

  std::vector<DynTexture   > dynamic_textures;
  std::vector<PixmapTexture> pixmap_textures;

  void removeTex( AbstractAPI::Texture* t ){
    for( size_t i=0; i<dynamic_textures.size(); ++i )
      if( dynamic_textures[i].owner==t ){
        dynamic_textures[i] = dynamic_textures.back();
        dynamic_textures.pop_back();
        return;
        }

    for( size_t i=0; i<pixmap_textures.size(); ++i )
      if( pixmap_textures[i].owner==t ){
        pixmap_textures[i] = pixmap_textures.back();
        pixmap_textures.pop_back();
        return;
        }
    }

  size_t count;
  };

TextureHolder::TextureHolder( Device& d):BaseType(d) {
  data = new Data();
  data->count = 0;
  }

TextureHolder::~TextureHolder(){
  T_WARNING( data->count==0 );
  delete data;
  }

Tempest::Texture2d TextureHolder::create( int w, int h,
                                          AbstractTexture::Format::Type f,
                                          TextureUsage u ){
  Tempest::Texture2d obj( *this );
  w = std::max(w,0);
  h = std::max(h,0);

  if( !(w==0 || h==0) )
    createObject( obj.data.value(), w, h, 0, f, u ); else
    obj.data.value() = 0;

  if( obj.data.const_value() ){
    obj.w   = w;
    obj.h   = h;
    } else {
    obj.w   = 0;
    obj.h   = 0;
    }

  obj.frm = f;  

  return obj;
  }

Texture2d TextureHolder::create( const Pixmap &p, bool mips, bool compress ) {
  Tempest::Texture2d obj( *this );

  if( !(p.width()==0 || p.height()==0) )
    createObject( obj.data.value(), p, mips, compress ); else
    obj.data.value() = 0;

  obj.w = p.width();
  obj.h = p.height();

  if( p.hasAlpha() )
    obj.frm = Texture2d::Format::RGBA; else
    obj.frm = Texture2d::Format::RGB;

  return obj;
  }

Texture2d TextureHolder::create(Size wh,
                                AbstractTexture::Format::Type format,
                                TextureUsage u) {
  return create( wh.w, wh.h, format, u );
  }

Texture2d TextureHolder::load(const std::u16string &fname) {
  Pixmap p;
  p.load(fname);
  return create(p, true);
  }

Tempest::Texture2d TextureHolder::load( const std::string & fname ){
  Pixmap p;
  p.load(fname);
  return create(p, true);
  }

Tempest::Texture2d TextureHolder::load( const char* fname ){
  Pixmap p;
  p.load(fname);
  return create(p, true);
  }

Texture2d TextureHolder::load( IDevice &file) {
  Pixmap p;
  p.load(file);
  return create(p, true);
  }

void TextureHolder::createObject( AbstractAPI::Texture*& t,
                                  IDevice &file ){
  Pixmap p;
  p.load(file);
  createObject( t, p, true, true );
  }

void TextureHolder::createObject( AbstractAPI::Texture*& t,
                                  int w, int h, bool mips,
                                  AbstractTexture::Format::Type f,
                                  TextureUsage u ){
  t = device().createTexture( w, h, mips, f, u );

  if( !t )
    return;

  ++data->count;
  Data::DynTexture d;
  d.w      = w;
  d.h      = h;
  d.mip    = mips;
  d.usage  = u;
  d.format = f;
  d.owner  = t;

  if( hasCPUStorage() )
    data->dynamic_textures.push_back(d);

  setTextureFlag(t, AbstractAPI::TF_Inialized, false );
  }

void TextureHolder::createObject( AbstractAPI::Texture *&t,
                                  const Pixmap &p,
                                  bool mips,
                                  bool compress ){
  Data::PixmapTexture px;
  px.pm = p;
  px.mip = mips;
  px.compress = compress;

  t = device().createTexture( p, mips, compress );

  if( !t )
    return;

  ++data->count;
  px.owner = t;
  //data->pixmap_textures[t] = px;
  if( hasCPUStorage() )
    data->pixmap_textures.push_back(px);
  }

void TextureHolder::recreateObject( AbstractAPI::Texture *&t,
                                    AbstractAPI::Texture * old,
                                    const Pixmap &p,
                                    bool mips,
                                    bool compress ) {
  if( hasCPUStorage() ){
    data->removeTex(old);
    }

  Data::PixmapTexture px;
  px.pm = p;
  px.mip = mips;
  px.compress = compress;

  t = device().recreateTexture( p, mips, compress, old );
  px.owner = t;
  //data->pixmap_textures[t] = px;
  if( hasCPUStorage() )
    data->pixmap_textures.push_back(px);
  }

void TextureHolder::deleteObject( AbstractAPI::Texture* t ){
  if( hasCPUStorage() ){
    data->removeTex(t);
    }

  --data->count;
  device().deleteTexture(t);
  }

void TextureHolder::reset( AbstractAPI::Texture* t ){
  if( hasCPUStorage() ){
    --data->count;
    device().deleteTexture(t);
    }
  }

AbstractAPI::Texture* TextureHolder::restore( AbstractAPI::Texture* t ){
  if( !hasCPUStorage() ){
    return t;
    }

  for( size_t i=0; i<data->dynamic_textures.size(); ++i )
    if( data->dynamic_textures[i].owner==t ){
      Data::DynTexture d = data->dynamic_textures[i];
      data->dynamic_textures[i] = data->dynamic_textures.back();
      data->dynamic_textures.pop_back();

      createObject( t, d.w, d.h, d.mip, d.format, d.usage );
      return t;
      }

  for( size_t i=0; i<data->pixmap_textures.size(); ++i )
    if( data->pixmap_textures[i].owner==t ){
      Data::PixmapTexture d = data->pixmap_textures[i];

      data->pixmap_textures[i] = data->pixmap_textures.back();
      data->pixmap_textures.pop_back();

      createObject( t, d.pm, d.mip, d.compress );
      return t;
      }

  return 0;
  }

AbstractAPI::Texture* TextureHolder::copy( AbstractAPI::Texture*  ){
#ifndef __ANDROID__
  throw std::runtime_error("TextureHolder::copy not implemented yet");
#endif
  return 0;
  }

void TextureHolder::setTextureFlag( AbstractAPI::Texture *t,
                                    AbstractAPI::TextureFlag f,
                                    bool v) {
  device().setTextureFlag(t,f,v);
  }

bool TextureHolder::hasCPUStorage() {
  return !device().hasManagedStorge();
  }

Pixmap TextureHolder::pixmapOf( AbstractAPI::Texture *t ) {
  for( size_t i=0; i<data->pixmap_textures.size(); ++i )
    if( data->pixmap_textures[i].owner==t )
      return data->pixmap_textures[i].pm;

  return Pixmap();
  }

void TextureHolder::onMipmapsAdded(GraphicsSubsystem::Texture *tg) {
  for( size_t i=0; i<data->pixmap_textures.size(); ++i )
    if( data->pixmap_textures[i].owner==tg ){
      data->pixmap_textures[i].mip = true;
      return;
      }

  for( size_t i=0; i<data->dynamic_textures.size(); ++i )
    if( data->dynamic_textures[i].owner==tg ){
      data->dynamic_textures[i].mip = 1;
      return;
      }
  }
