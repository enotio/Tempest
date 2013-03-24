#include "localtexturesholder.h"

#include <cstring>
#include <Tempest/Pixmap>

using namespace Tempest;

LocalTexturesHolder::LocalTexturesHolder( Tempest::Device &d ):Tempest::TextureHolder(d) {
  nonFreed   .reserve(128);
  dynTextures.reserve(128);

  needToRestore = false;
  }

LocalTexturesHolder::~LocalTexturesHolder() {
  reset();
  }

void LocalTexturesHolder::reset() {
  for( size_t i=0; i<nonFreed.size(); ++i )
    deleteObject( nonFreed[i] );

  nonFreed.clear();
  //dynTextures.clear();
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
  collect( nonFreed );
  }

void LocalTexturesHolder::collect(std::vector<NonFreed> &nonFreed) {
  for( size_t i=0; i<nonFreed.size(); ){
    ++nonFreed[i].collectIteration;

    if( nonFreed[i].collectIteration > 3 ){
      deleteObject( nonFreed[i] );
      nonFreed[i] = nonFreed.back();
      nonFreed.pop_back();
      } else {
      ++i;
      }

    }
  }

void LocalTexturesHolder::deleteObject(LocalTexturesHolder::NonFreed &obj) {
  Tempest::TextureHolder::deleteObject( obj.data.handle );
  }

void LocalTexturesHolder::createObject( Tempest::AbstractAPI::Texture *&t,
                                        int w, int h, int mip,
                                        Tempest::AbstractTexture::Format::Type f,
                                        Tempest::TextureUsage u ) {
  if( needToRestore ){
    AbstractAPI::Texture* old = t;
    Tempest::TextureHolder::createObject(t,w,h,mip,f,u);

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
  d.w       = w;
  d.h       = h;
  d.mip     = mip;
  d.usage   = u;
  d.format  = f;
  d.dynamic = true;
  d.restoreIntent = true;

  for( size_t i=0; i<nonFreed.size(); ++i ){
    d.handle = nonFreed[i].data.handle;

    NonFreedData& x = nonFreed[i].data;
    if( x.cmp(d) ){
      dynTextures.push_back( nonFreed[i] );
      nonFreed[i] = nonFreed.back();
      nonFreed.pop_back();

      t = dynTextures.back().data.handle;
      return;
      }
    }

  Tempest::TextureHolder::createObject(t,w,h,mip,f,u);

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
                                        bool mips ) {
  NonFreedData d;
  d.w       = p.width();
  d.h       = p.height();
  d.mip     = mips ? 1:0;
  d.usage   = Tempest::TU_Dynamic;
  d.format  = p.hasAlpha() ? Tempest::AbstractTexture::Format::RGBA : Tempest::AbstractTexture::Format::RGB;
  d.dynamic = false;
  d.restoreIntent = false;

  for( size_t i=0; i<nonFreed.size(); ++i ){
    d.handle = nonFreed[i].data.handle;

    NonFreedData& x = nonFreed[i].data;
    if( memcmp( &x, &d, sizeof(d) )==0 ){
      dynTextures.push_back( nonFreed[i] );
      nonFreed[i] = nonFreed.back();
      nonFreed.pop_back();

      AbstractAPI::Texture * old = dynTextures.back().data.handle;

      Tempest::TextureHolder::recreateObject(t, old, p, mips);
      return;
      }
    }

  Tempest::TextureHolder::createObject(t,p,mips);

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
      nonFreed.push_back( dynTextures[i] );

      dynTextures[i] = dynTextures.back();
      dynTextures.pop_back();

      return;
      }

  Tempest::TextureHolder::deleteObject(t);
  }
