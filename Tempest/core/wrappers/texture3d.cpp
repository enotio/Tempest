#include "texture3d.h"

#include <Tempest/Render>

using namespace Tempest;

Texture3d::Sampler::Sampler(){
  minFilter = Texture2d::FilterType::Linear;
  magFilter = minFilter;
  mipFilter = minFilter;

  uClamp = Texture2d::ClampMode::Repeat;
  vClamp = Texture2d::ClampMode::Repeat;
  wClamp = Texture2d::ClampMode::ClampToEdge;
  }

Texture3d::Texture3d():data( VolumeHolder::ImplManip(0) ),
  sx(0), sy(0), sz(0),
  frm(Format::Count) {
  }

Texture3d::Texture3d( AbstractHolder< Tempest::Texture3d,
                                      AbstractAPI::Texture >& h )
  :data( h.makeManip() ), sx(0), sy(0), sz(0), frm(Format::Count) {

  }

const Texture3d::Sampler& Texture3d::sampler() const {
  return m_sampler;
  }

void Texture3d::setSampler( const Texture3d::Sampler& s ) {
  m_sampler = s;
  }

size_t Texture3d::handle() const {
  return reinterpret_cast<size_t>( data.const_value() );
  }

int Texture3d::width() const {
  return sx;
  }

int Texture3d::height() const {
  return sy;
  }

int Texture3d::sizeX() const {
  return sx;
  }

int Texture3d::sizeY() const {
  return sy;
  }

int Texture3d::sizeZ() const {
  return sz;
  }

bool Texture3d::isEmpty() const {
  return sx==0||0==sy||sz==0;
  }

AbstractTexture::Format::Type Texture3d::format() const {
  return frm;
  }
