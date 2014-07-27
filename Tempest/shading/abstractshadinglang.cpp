#include "abstractshadinglang.h"

#include <Tempest/ShaderProgram>

#include <Tempest/SystemAPI>
#include <Tempest/Device>
#include <Tempest/Matrix4x4>
#include <Tempest/Texture2d>
#include <Tempest/Texture3d>

using namespace Tempest;

GraphicsSubsystem::ProgramObject *AbstractShadingLang::get(const Tempest::ShaderProgram &s) {
  return s.data.const_value();
  }

AbstractAPI::Texture* AbstractShadingLang::get( const Tempest::Texture2d & t ){
  return t.data.const_value();
  }

AbstractAPI::Texture *AbstractShadingLang::get(const Texture3d &t) {
  return t.data.const_value();
  }

const std::shared_ptr<std::vector<AbstractShadingLang::UBO>>&
  AbstractShadingLang::inputOf(const Tempest::ShaderProgram &s) {
  return s.ubo;
  }

template< class T >
static inline void align(void*& ptr){
  uintptr_t i = uintptr_t(ptr);
  ptr = ((i+alignof(T)-1)/alignof(T))*alignof(T);
  }

template< class T >
static inline void align(size_t& ptr){
  ptr = ((ptr+alignof(T)-1)/alignof(T))*alignof(T);
  }

template< class T, class X >
static inline void inc(X& ptr, int count=1){
  ptr = (X)(((uintptr_t(ptr)+alignof(T)-1)/alignof(T))*alignof(T));
  ptr += sizeof(T)*count;
  }

void AbstractShadingLang::assignUniformBuffer( UBO& ux,
                                               const char *ubo,
                                               const UniformDeclaration &u ) {
  size_t bufsz=0;
  for( int type:u.type ){
    if( type<Decl::count ){
      align<float>(bufsz);
      bufsz += Decl::elementSize(Decl::ComponentType(type));
      }
    else if( type==Decl::Texture2d ){
      inc<void*>(bufsz);
      inc<Texture2d::Sampler>(bufsz);
      }
    else if( type==Decl::Texture3d ){
      inc<void*>(bufsz);
      inc<Texture3d::Sampler>(bufsz);
      }
    else if( type==Decl::Matrix4x4 ){
      inc<Matrix4x4>(bufsz);
      }
    }
  ux.id.resize(u.type.size());
  ux.names = u.name;
  ux.desc  = u.type;
  ux.data .resize(bufsz);
  ux.fields.resize(ux.desc.size());

  bufsz = 0;
  int i=0;
  char *data = &ux.data[0];
  for( int type:ux.desc ){
    const char* addr = ubo+bufsz;    
    ux.fields[i] = data;
    ++i;

    if( type<Decl::count ){
      size_t vsz = Decl::elementSize(Decl::ComponentType(type));
      align<float>(bufsz);
      memcpy(data, addr, vsz);

      bufsz += vsz;
      data  += vsz;
      } else {
      void* t=0;
      switch (type) {
        case Decl::Texture2d:
          t = (void*)get(*(Texture2d*)addr);
          (*(void**)data) = t;
          inc<void*>(data);
          inc<Texture2d>(bufsz);
          *(Texture2d::Sampler*)data = ((Texture2d*)addr)->sampler();
          inc<Texture2d::Sampler>(data);
          break;
        case Decl::Texture3d:
          t = (void*)get(*(Texture3d*)addr);
          (*(void**)data) = t;
          inc<void*>(data);
          inc<Texture3d>(bufsz);
          *(Texture3d::Sampler*)data = ((Texture3d*)addr)->sampler();
          inc<Texture3d::Sampler>(data);
          break;
        case Decl::Matrix4x4:
          *(Tempest::Matrix4x4*)data = *(Tempest::Matrix4x4*)addr;
          inc<Matrix4x4>(data);
          inc<Matrix4x4>(bufsz);
          break;
        }
      }
    }
  }

AbstractShadingLang::UiShaderOpt::UiShaderOpt() {
  hasTexture = true;
  vertex     = Tempest::Decl::float2;
  texcoord   = Tempest::Decl::float2;
  color      = Tempest::Decl::float4;
  }
