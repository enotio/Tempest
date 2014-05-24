#include "localtexturesholder.h"

#include <cstring>
#include <Tempest/Pixmap>
#include <Tempest/Device>

using namespace Tempest;

LocalTexturesHolder::LocalTexturesHolder( Tempest::Device &d ):Tempest::TextureHolder(d) {
  dynTextures.reserve(128);

  needToRestore = false;
  maxReserved = -1;
  pcollect    = false;
  }

LocalTexturesHolder::~LocalTexturesHolder() {
  reset();
  }

void LocalTexturesHolder::setMaxCollectIterations(int c) {
  nonFreed.maxColletIterations = c;
  }

int LocalTexturesHolder::maxCollectIterations() const {
  return nonFreed.maxColletIterations;
  }

void LocalTexturesHolder::setMaxReservedCount(int s) {
  maxReserved = s;
  }

int LocalTexturesHolder::maxReservedCount() const {
  return maxReserved;
  }

void LocalTexturesHolder::pauseCollect(bool p) {
  pcollect = p;
  }

bool LocalTexturesHolder::isCollectPaused() const {
  return pcollect;
  }

void LocalTexturesHolder::reset() {
  nonFreed.reset(*this, &LocalTexturesHolder::deleteObject );
  TextureHolder::BaseType::reset();
  }

bool LocalTexturesHolder::restore() {
  for( size_t i=0; i<dynTextures.size(); ++i )
    dynTextures[i].data.restoreIntent = true;

  bool ok = TextureHolder::BaseType::restore();
  needToRestore = false;
  return ok;
  }

void LocalTexturesHolder::presentEvent() {
  if( pcollect )
    return;

  nonFreed.collect( *this,
                    &LocalTexturesHolder::deleteObject,
                    &LocalTexturesHolder::deleteCond );
  }

void LocalTexturesHolder::deleteObject( LocalTexturesHolder::NonFreed &obj ) {
  Tempest::TextureHolder::deleteObject( obj.data.handle );
  }

bool LocalTexturesHolder::deleteCond( LocalTexturesHolder::NonFreed&  ){
  return true;
  }

void LocalTexturesHolder::onMipmapsAdded(GraphicsSubsystem::Texture *tg) {
  TextureHolder::onMipmapsAdded(tg);

  for( NonFreed& t : dynTextures )
    if( t.data.handle==tg )
      t.data.mip = true;

  /*
  for( NonFreed& t : nonFreed.data )
    if( t.data.handle==tg )
      t.data.mip = true;
  */
  }

void LocalTexturesHolder::createObject(Tempest::AbstractAPI::Texture *&t,
                                        int w, int h, bool mips,
                                        Tempest::AbstractTexture::Format::Type f,
                                        Tempest::TextureUsage u ) {
  if( needToRestore ){
    AbstractAPI::Texture* old = t;
    Tempest::TextureHolder::createObject(t,w,h,mips,f,u);

    for( size_t i=0; i<dynTextures.size(); ++i )
      if( dynTextures[i].data.handle==old &&
          dynTextures[i].data.restoreIntent ){
        dynTextures[i].data.handle = t;
        dynTextures[i].data.restoreIntent = false;
        return;
        }
    return;
    }

  NonFreedData d;
  d.w        = w;
  d.h        = h;
  d.mip      = mips;
  d.compress = false;
  d.usage    = u;
  d.format   = f;
  d.dynamic  = true;
  d.restoreIntent = true;

  NonFreed x = nonFreed.find(d, *this, &LocalTexturesHolder::validAs);

  if( x.data.handle ){
    dynTextures.push_back(x);

    t = dynTextures.back().data.handle;
    setTextureFlag(t, AbstractAPI::TF_Inialized, false );
    return;
    }

  Tempest::TextureHolder::createObject(t,w,h,mips,f,u);

  if( !t )
    return;

  d.handle = t;

  NonFreed nf;
  nf.data = d;
  nf.collectIteration = 0;
  nf.userPtr = 0;

  dynTextures.push_back( nf );
  }

void LocalTexturesHolder::createObject( AbstractAPI::Texture *&t,
                                        const Pixmap &p,
                                        bool mips,
                                        bool compress ) {
  NonFreedData d;
  d.w        = p.width();
  d.h        = p.height();
  d.mip      = mips ? 1:0;
  d.compress = compress;
  d.usage    = Tempest::TU_Dynamic;
  d.format   = p.hasAlpha() ? Tempest::AbstractTexture::Format::RGBA : Tempest::AbstractTexture::Format::RGB;
  d.dynamic  = false;
  d.restoreIntent = false;

  NonFreed x = nonFreed.find(d, *this, &LocalTexturesHolder::validAs);
  if( x.data.handle ){
    dynTextures.push_back( x );

    AbstractAPI::Texture * old = x.data.handle;
    Tempest::TextureHolder::recreateObject(t, old, p, mips, compress);
    return;
    }

  Tempest::TextureHolder::createObject(t,p,mips,compress);

  if( !t )
    return;

  d.handle = t;

  NonFreed nf;
  nf.data = d;
  nf.collectIteration = 0;
  nf.userPtr = 0;

  dynTextures.push_back( nf );
  }

void LocalTexturesHolder::deleteObject( Tempest::AbstractAPI::Texture *t ) {
  for( size_t i=0; i<dynTextures.size(); ++i )
    if( dynTextures[i].data.handle==t ){
      dynTextures[i].collectIteration = 0;

      if( maxReserved>=0 && int(nonFreed.size())>=maxReserved ){
        TextureHolder::deleteObject(t);
        } else {
        nonFreed.push( dynTextures[i] );
        setTextureFlag( dynTextures[i].data.handle, AbstractAPI::TF_Inialized, false );
        }

      dynTextures[i] = dynTextures.back();
      dynTextures.pop_back();
      return;
      }

  Tempest::TextureHolder::deleteObject(t);
  }

bool LocalTexturesHolder::validAs( const LocalTexturesHolder::NonFreedData &x,
                                   const LocalTexturesHolder::NonFreedData &d ) {
  return x==d;
  }
