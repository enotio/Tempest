#include "textureholder.h"

#include <Tempest/Texture2d>
#include <Tempest/Device>
#include <Tempest/Pixmap>

#include <map>

#include <stdexcept>

using namespace Tempest;

struct TextureHolder::Data {
  //std::map< AbstractAPI::Texture*, std::string > textures;

  struct DynTexture{
    int w, h, mip;
    TextureUsage usage;
    AbstractTexture::Format::Type format;
    };

  struct PixmapTexture{
    Pixmap pm;
    bool mip, compress;
    AbstractAPI::Texture* owner;
    };

  std::map< AbstractAPI::Texture*, DynTexture   > dynamic_textures;
  std::vector<PixmapTexture> pixmap_textures;
  };

TextureHolder::TextureHolder( Device& d):BaseType(d) {
  data = new Data();
  }

TextureHolder::~TextureHolder(){
  delete data;
  }

TextureHolder::TextureHolder( const TextureHolder& h):BaseType( h.device() ) {
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
  obj.w   = w;
  obj.h   = h;
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

Texture2d TextureHolder::load(const std::wstring &fname) {
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

void TextureHolder::createObject( AbstractAPI::Texture*& t,
                                  const std::wstring &fname ){
  Pixmap p;
  p.load(fname);
  createObject( t, p, true, true );
  }

void TextureHolder::createObject( AbstractAPI::Texture*& t,
                                  int w, int h, bool mips,
                                  AbstractTexture::Format::Type f,
                                  TextureUsage u ){
  t = device().createTexture( w, h, mips, f, u );

  if( !t )
    return;

  Data::DynTexture d;
  d.w      = w;
  d.h      = h;
  d.mip    = mips;
  d.usage  = u;
  d.format = f;

  if( hasCPUStorage() )
    data->dynamic_textures[t] = d;

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
    if( data->dynamic_textures.find(old) != data->dynamic_textures.end() )
      data->dynamic_textures.erase(old);

    for( size_t i=0; i<data->pixmap_textures.size(); ++i )
      if( data->pixmap_textures[i].owner==old ){
        data->pixmap_textures[i] = data->pixmap_textures.back();
        data->pixmap_textures.pop_back();
        i = -1;
        }
    }

  Data::PixmapTexture px;
  px.pm = p;
  px.mip = mips;
  px.compress = compress;

  t = device().recreateTexture( old, p, mips, compress );
  px.owner = t;
  //data->pixmap_textures[t] = px;
  if( hasCPUStorage() )
    data->pixmap_textures.push_back(px);
  }

void TextureHolder::deleteObject( AbstractAPI::Texture* t ){
  if( hasCPUStorage() ){
    if( data->dynamic_textures.find(t) != data->dynamic_textures.end() )
      data->dynamic_textures.erase(t);

    for( size_t i=0; i<data->pixmap_textures.size(); ++i )
      if( data->pixmap_textures[i].owner==t ){
        data->pixmap_textures[i] = data->pixmap_textures.back();
        data->pixmap_textures.pop_back();
        i = -1;
        }
    }

  device().deleteTexture(t);
  }

void TextureHolder::reset( AbstractAPI::Texture* t ){
  if( hasCPUStorage() )
    device().deleteTexture(t);
  }

AbstractAPI::Texture* TextureHolder::restore( AbstractAPI::Texture* t ){
  if( !hasCPUStorage() ){
    return t;
    }

  if( data->dynamic_textures.find(t)!=data->dynamic_textures.end() ){
    Data::DynTexture d = data->dynamic_textures[t];
    data->dynamic_textures.erase(t);

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
