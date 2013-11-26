#include "texture2d.h"

#include <Tempest/Render>

using namespace Tempest;

Texture2d::Sampler::Sampler(){
  minFilter = Texture2d::FilterType::Linear;
  magFilter = minFilter;
  mipFilter = minFilter;

  uClamp = Texture2d::ClampMode::Repeat;
  vClamp = Texture2d::ClampMode::Repeat;

  anisotropic = true;
  }

Texture2d::Texture2d():data( TextureHolder::ImplManip(0) ),
  w(0), h(0),
  frm(Format::Count) {

  }

Texture2d::Texture2d( AbstractHolderWithLoad< Tempest::Texture2d,
                                              AbstractAPI::Texture >& h )
  :data( h.makeManip() ), w(0), h(0), frm(Format::Count) {

  }

const Texture2d::Sampler& Texture2d::sampler() const {
  return m_sampler;
  }

void Texture2d::setSampler( const Texture2d::Sampler& s ) {
  m_sampler = s;
  }

size_t Texture2d::handle() const {
  return reinterpret_cast<size_t>( data.const_value() );
  }

int Texture2d::width() const {
  return w;
  }

int Texture2d::height() const {
  return h;
  }

Size Texture2d::size() const {
  return Size(w,h);
  }

bool Texture2d::isEmpty() const {
  return w==0||0==h;
  }

AbstractTexture::Format::Type Texture2d::format() const {
  return frm;
  }
