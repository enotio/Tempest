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

void AbstractShadingLang::assignUniformBuffer( UBO& ux,
                                               const char *ubo,
                                               const UniformDeclaration &u ) {
  size_t bufsz=0;
  for( int type:u.type ){
    if( type<Decl::count )
      bufsz += Decl::elementSize(Decl::ComponentType(type));
    else if( type==Decl::Texture2d )
      bufsz += sizeof(void*) + sizeof(Texture2d::Sampler);
    else if( type==Decl::Texture3d )
      bufsz += sizeof(void*) + sizeof(Texture3d::Sampler);
    else if( type==Decl::Matrix4x4 )
      bufsz += sizeof(Tempest::Matrix4x4);
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
    size_t vsz;

    const char* addr = ubo+bufsz;    
    ux.fields[i] = data;
    ++i;

    if( type<Decl::count ){
      vsz = Decl::elementSize(Decl::ComponentType(type));
      memcpy(data, addr, vsz);
      } else {
      void* t=0;
      vsz = 0;//sizeof(t);
      switch (type) {
        case Decl::Texture2d:
          t = (void*)get(*(Texture2d*)addr);
          (*(void**)data) = t;
          data  += sizeof(void*);
          bufsz += sizeof(Texture2d);
          *(Texture2d::Sampler*)data = ((Texture2d*)addr)->sampler();
          data  += sizeof(Texture2d::Sampler);
          break;
        case Decl::Texture3d:
          t = (void*)get(*(Texture3d*)addr);
          (*(void**)data) = t;
          data  += sizeof(void*);
          bufsz += sizeof(Texture3d);
          *(Texture3d::Sampler*)data = ((Texture3d*)addr)->sampler();
          data  += sizeof(Texture2d::Sampler);
          break;
        case Decl::Matrix4x4:
          *(Tempest::Matrix4x4*)data = *(Tempest::Matrix4x4*)addr;
          data  += sizeof(Matrix4x4);
          bufsz += sizeof(Matrix4x4);
          break;
        }
      }

    bufsz += vsz;
    data  += vsz;
    }
  }

AbstractShadingLang::UiShaderOpt::UiShaderOpt() {
  hasTexture = true;
  vertex     = Tempest::Decl::float2;
  texcoord   = Tempest::Decl::float2;
  color      = Tempest::Decl::float4;
  }
