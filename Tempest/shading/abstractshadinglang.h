#ifndef ABSTRACTSHADINGLANG_H
#define ABSTRACTSHADINGLANG_H

#include <string>
#include <Tempest/AbstractAPI>
#include <Tempest/Assert>
#include <Tempest/GraphicsSubsystem>
#include <Tempest/UniformDeclaration>

namespace Tempest{

class ShaderProgram;

class Matrix4x4;
class Texture2d;
class Texture3d;

class AbstractShadingLang : public GraphicsSubsystem {
  private:
    AbstractShadingLang(const AbstractShadingLang&) = delete;
    AbstractShadingLang& operator = (const AbstractShadingLang&) = delete;
  public:    
    struct UiShaderOpt{
      UiShaderOpt();
      bool hasTexture;
      Tempest::Decl::ComponentType vertex, texcoord, color;
      };

    struct Source {
      std::string vs,fs,gs,es,ts;
      };

    AbstractShadingLang(){}
    virtual ~AbstractShadingLang(){}
    
    virtual void beginPaint() const {}
    virtual void endPaint()   const {}

    virtual void enable()  const {}
    virtual void disable() const {}

    virtual void bind( const Tempest::ShaderProgram&  ) const = 0;

    virtual void setVertexDecl( const Tempest::AbstractAPI::VertexDecl*  ) const{}

    virtual void* context() const = 0;

    virtual ProgramObject* createShaderFromSource( const Source& src,
                                                   std::string & outputLog ) const = 0;

    virtual void deleteShader( ProgramObject*    ) const = 0;

    virtual Source surfaceShader( const AbstractShadingLang::UiShaderOpt &opt,
                                  bool &hasHalfPixelOffset ) const = 0;
    struct UBO {
      // buffer for plain data, like vec3, vec4, matrix
      std::vector<char>     data;
      std::vector<char>     names;
      std::vector<int>      desc;
      std::vector<intptr_t> fields;

      std::vector<void*>    tex;
      std::vector<char*>    smp[2];

      mutable std::vector<uintptr_t> id;
      mutable bool                   updated=false;
      };
    static void assignUniformBuffer( UBO &ux,
                                     const char *ubo,
                                     const UniformDeclaration &u );
  protected:
    static ProgramObject*  get( const Tempest::ShaderProgram  & s );

    static AbstractAPI::Texture* get( const Tempest::Texture2d & s );
    static AbstractAPI::Texture* get( const Tempest::Texture3d & s );

    static const std::shared_ptr<std::vector<AbstractShadingLang::UBO>>&
             inputOf(const Tempest::ShaderProgram & s );

    friend class ProgramObject;
  };

}

#endif // SHADINGLANG_H
