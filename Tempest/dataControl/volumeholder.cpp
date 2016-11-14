#include "volumeholder.h"

#include <Tempest/Texture2d>
#include <Tempest/Texture3d>

#include <Tempest/Device>
#include <Tempest/Pixmap>
#include <Tempest/Assert>

#include <map>

#include <stdexcept>
#include <algorithm>

using namespace Tempest;

/// \cond HIDDEN_SYMBOLS
struct VolumeHolder::Data {
  //std::map< AbstractAPI::Texture*, std::string > textures;

  struct DynTexture{
    int x, y, z;
    bool mip;
    TextureUsage usage;
    AbstractTexture::Format::Type format;
    };

  std::map< AbstractAPI::Texture*, DynTexture   > dynamic_textures;

  size_t count;
  };
/// \endcond

VolumeHolder::VolumeHolder( Device& d):BaseType(d) {
  data = new Data();
  data->count = 0;
  }

VolumeHolder::~VolumeHolder(){
  T_WARNING( data->count==0 );
  delete data;
  }

Texture3d VolumeHolder::load( int x, int y, int z,
                              const void *data,
                              AbstractTexture::Format::Type f,
                              TextureUsage u ) {
  Tempest::Texture3d obj( *this );
  x = std::max(x,0);
  y = std::max(y,0);
  z = std::max(z,0);

  if( !(x==0 || y==0 || z==0) )
    createObject( obj.data.value(), x, y, z, false, (const char*)data, f, u ); else
    obj.data.value() = 0;
  obj.sx   = x;
  obj.sy   = y;
  obj.sz   = z;

  obj.frm = f;

  return obj;
  }

Texture3d VolumeHolder::create( int x, int y, int z,
                                AbstractTexture::Format::Type f,
                                TextureUsage u) {
  return load(x,y,z, 0, f,u);
  }

void VolumeHolder::createObject( AbstractAPI::Texture *&t,
                                 int x, int y, int z, bool mips,
                                 const char* d,
                                 AbstractTexture::Format::Type f,
                                 TextureUsage u ) {
  Data::DynTexture px;
  px.x = x;
  px.y = y;
  px.z = z;

  px.mip = mips;

  px.format = f;
  px.usage  = u;

  t = device().createTexture3d( x, y, z, mips, d, f, u );

  if( !t )
    return;

  ++data->count;

  if( hasCPUStorage() )
    data->dynamic_textures[t] = px;
  }

void VolumeHolder::deleteObject( AbstractAPI::Texture* t ){
  if( hasCPUStorage() ){
    if( data->dynamic_textures.find(t) != data->dynamic_textures.end() )
      data->dynamic_textures.erase(t);
    }

  --data->count;
  device().deleteTexture(t);
  }

void VolumeHolder::reset( AbstractAPI::Texture* t ){
  if( hasCPUStorage() ){
    --data->count;
    device().deleteTexture(t);
    }
  }

AbstractAPI::Texture* VolumeHolder::restore( AbstractAPI::Texture* t ){
  if( !hasCPUStorage() ){
    return t;
    }

  if( data->dynamic_textures.find(t)!=data->dynamic_textures.end() ){
    Data::DynTexture d = data->dynamic_textures[t];
    data->dynamic_textures.erase(t);

    createObject( t, d.x, d.y, d.z, d.mip, 0, d.format, d.usage );
    return t;
    }

  return 0;
  }

AbstractAPI::Texture* VolumeHolder::copy( AbstractAPI::Texture*  ){
#ifndef __ANDROID__
  throw std::runtime_error("VolumeHolder::copy not implemented yet");
#endif
  return 0;
  }

void VolumeHolder::setTextureFlag( AbstractAPI::Texture *t,
                                   AbstractAPI::TextureFlag f,
                                   bool v) {
  device().setTextureFlag(t,f,v);
  }

bool VolumeHolder::hasCPUStorage() {
  return !device().hasManagedStorge();
  }
