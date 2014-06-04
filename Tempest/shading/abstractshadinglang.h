#ifndef ABSTRACTSHADINGLANG_H
#define ABSTRACTSHADINGLANG_H

#include <string>
#include <Tempest/AbstractAPI>
#include <Tempest/Assert>
#include <Tempest/GraphicsSubsystem>

#include <map>

namespace Tempest{

class Shader;
class VertexShader;
class FragmentShader;
class TessShader;
class EvalShader;

class Matrix4x4;
class Texture2d;
class Texture3d;

class ShaderInput;

class AbstractShadingLang : public GraphicsSubsystem {
  public:
    AbstractShadingLang() = default;
    AbstractShadingLang( const AbstractShadingLang& ) = delete;
    virtual ~AbstractShadingLang(){}

    AbstractShadingLang& operator = ( const AbstractShadingLang& ) = delete;

    virtual void beginPaint() const {}
    virtual void endPaint()   const {}

    virtual void enable()  const {}
    virtual void disable() const {}

    virtual void bind( const Tempest::VertexShader& ) const = 0;
    virtual void bind( const Tempest::FragmentShader& ) const = 0;
    virtual void bind( const Tempest::TessShader&     ) const;
    virtual void bind( const Tempest::EvalShader&     ) const;

    virtual void setVertexDecl( const Tempest::AbstractAPI::VertexDecl*  ) const{}

    virtual void* context() const = 0;

    virtual void* createShader( ShaderType t,
                                const std::string& fname,
                                std::string & outputLog ) const;
    virtual void* createShader( ShaderType t,
                                const std::u16string& fname,
                                std::string & outputLog ) const;
    virtual void* createShaderFromSource( ShaderType t,
                                          const std::string& src,
                                          std::string & outputLog ) const {
      T_WARNING_X( src.size()!=0, "shader source is empty" );

      switch( t ){
        case Vertex:
          return createVertexShaderFromSource(src, outputLog);
        case Fragment:
          return createFragmentShaderFromSource(src, outputLog);
        case Tess:
          return createTessShaderFromSource(src, outputLog);
        case Eval:
          return createEvalShaderFromSource(src, outputLog);
        }

      return 0;
      }

    virtual VertexShader*
                 createVertexShaderFromSource( const std::string& src,
                                               std::string & outputLog ) const = 0;

    virtual FragmentShader*
                 createFragmentShaderFromSource( const std::string& src,
                                                 std::string & outputLog ) const = 0;

    virtual TessShader*
                 createTessShaderFromSource( const std::string& src,
                                                 std::string & outputLog ) const;

    virtual EvalShader*
                 createEvalShaderFromSource( const std::string& src,
                                             std::string & outputLog ) const;

    virtual void deleteShader( VertexShader*   s ) const = 0;
    virtual void deleteShader( FragmentShader* s ) const = 0;
    virtual void deleteShader( TessShader*       ) const{}
    virtual void deleteShader( EvalShader*       ) const{}

    virtual bool link( const Tempest::VertexShader   &/*vs*/,
                       const Tempest::FragmentShader &/*fs*/,
                       const AbstractAPI::VertexDecl */*decl*/,
                       std::string& /*log*/ ) const {
      return true;
      }

    struct UiShaderOpt{
      UiShaderOpt();
      bool hasTexture;
      Tempest::Decl::ComponentType vertex, texcoord, color;
      };

    virtual std::string surfaceShader( ShaderType t, const UiShaderOpt&,
                                       bool& hasHalfpixOffset ) const = 0;
    virtual std::string surfaceShader( ShaderType t, const UiShaderOpt& opt ) const;
  protected:
    static VertexShader*   get( const Tempest::VertexShader   & s );
    static FragmentShader* get( const Tempest::FragmentShader & s );
    static TessShader*     get( const Tempest::TessShader     & s );
    static EvalShader*     get( const Tempest::EvalShader     & s );

    static AbstractAPI::Texture* get( const Tempest::Texture2d & s );
    static AbstractAPI::Texture* get( const Tempest::Texture3d & s );

    static const ShaderInput &inputOf( const Tempest::Shader & s );

  };

}

#endif // SHADINGLANG_H
