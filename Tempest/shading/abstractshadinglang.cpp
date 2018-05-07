#include "abstractshadinglang.h"

#include <Tempest/ShaderProgram>

#include <Tempest/SystemAPI>
#include <Tempest/Device>
#include <Tempest/Matrix4x4>
#include <Tempest/Texture2d>
#include <Tempest/Texture3d>
#include <algorithm>

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
static inline void align(size_t& ptr,size_t mod=1){
  size_t alig = alignof(T)*mod;
  ptr = ((ptr+alig-1)/alig)*alig;
  }

template< class T, class X >
static inline void inc(X& ptr,size_t count=1){
  ptr += sizeof(T)*count;
  }

template< class T >
static inline void align(size_t& offset,const char*& ptr,size_t mod=1){
  size_t alig = alignof(T)*mod;
  size_t dpt  = offset%alig;
  if(dpt)
    dpt=alig-dpt;
  offset += dpt;
  ptr    += dpt;
  }

template< class T >
void write(void* out,void* in){
  memcpy(out,in,sizeof(T));
  }

void AbstractShadingLang::assignUniformBuffer( UBO& ux,
                                               const char *ubo,
                                               const UniformDeclaration &u ) {
  size_t bufsz=0, insz=0;
  size_t sampSz[2]={0,0}, tex[2] = {0,0};
  size_t i=0;

  for( int type:u.type ) {
    const size_t count = u.count[i];
    if( type<=Decl::float4) {
      size_t block=size_t(type)*sizeof(float)*count;
      align<float>(bufsz);
      if(bufsz/16!=(bufsz+block-1)/16){
        bufsz=((bufsz+block-1)/16)*16;
        }
      bufsz += block;
      }
    else if( type<Decl::count ){
      align<float>(bufsz,1);
      bufsz += 4*sizeof(float)*count;
      }
    else if( type==Decl::Texture2d ){
      sampSz[0]+=sizeof(Texture2d::Sampler);
      ++tex[0];
      }
    else if( type==Decl::Texture3d ){
      sampSz[1]+=sizeof(Texture3d::Sampler);
      ++tex[1];
      }
    else if( type==Decl::Matrix4x4 ){
      align<float>(bufsz,2);
      bufsz += sizeof(Matrix4x4)*count;
      }
    }
  ux.id.resize(u.type.size());
  ux.names = u.name;
  ux.desc  = u.type;
  if(bufsz>0)
    ux.data.resize( ((bufsz+15)/16)*16 );

  ux.smp[0].resize(sampSz[0]);
  ux.smp[1].resize(sampSz[1]);
  ux.fields.resize(ux.desc.size());
  ux.count = u.count;
  ux.tex.resize(tex[0]+tex[1]);

  bufsz = 0;
  insz  = 0;
  void** tx   = ux.tex.data();
  char * pubo = ux.data.data();
  Texture2d::Sampler* smp2d = reinterpret_cast<Texture2d::Sampler*>(ux.smp[0].data());
  Texture3d::Sampler* smp3d = reinterpret_cast<Texture3d::Sampler*>(ux.smp[1].data());

  i=0;
  for( int type:ux.desc ){
    const char*  addr  = ubo+insz;
    const size_t count = u.count[i];

    if( type<=Decl::float4 ){
      size_t vsz = Decl::elementSize(Decl::ComponentType(type))*count;
      align<float>(insz);
      align<float>(bufsz);

      if(bufsz/16!=(bufsz+vsz-1)/16){
        bufsz=((bufsz+vsz-1)/16)*16;
        }

      char* data=pubo+bufsz;
      ux.fields[i] = bufsz;
      memcpy(data, addr, vsz);

      insz  += vsz;
      bufsz += vsz;
      }
    else if( type<Decl::count ){
      size_t vsz = Decl::elementSize(Decl::ComponentType(type))*count;
      align<float>(insz);
      align<float>(bufsz);

      char* data=pubo+bufsz;
      ux.fields[i] = bufsz;
      memcpy(data, addr, vsz);

      insz  += vsz;
      bufsz += vsz;
      }
    else {
      void* t=0;
      switch (type) {
        case Decl::Texture2d:
          align<Texture2d>(insz,addr);
          t = (void*)get(*(Texture2d*)addr);
          *tx = t;
          ++tx;
          *smp2d = ((Texture2d*)addr)->sampler();
          ++smp2d;
          inc<Texture2d>(insz);
          break;
        case Decl::Texture3d:
          align<Texture3d>(insz,addr);
          t = (void*)get(*(Texture3d*)addr);
          *tx = t;
          ++tx;
          *smp3d = ((Texture3d*)addr)->sampler();
          ++smp3d;
          inc<Texture3d>(insz);
          break;
        case Decl::Matrix4x4:
          align<float>(bufsz,2);
          ux.fields[i] = bufsz;
          char* data=pubo+bufsz;
          memcpy(data,addr,sizeof(Matrix4x4)*count);
          bufsz += sizeof(Matrix4x4)*count;
          insz  += sizeof(Matrix4x4)*count;
          break;
        }
      }
    ++i;
    }
  }

AbstractShadingLang::UiShaderOpt::UiShaderOpt() {
  hasTexture = true;
  vertex     = Tempest::Decl::float2;
  texcoord   = Tempest::Decl::float2;
  color      = Tempest::Decl::float4;
  }
