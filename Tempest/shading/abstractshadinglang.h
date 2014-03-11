#ifndef ABSTRACTSHADINGLANG_H
#define ABSTRACTSHADINGLANG_H

#include <string>
#include <Tempest/AbstractAPI>
#include <Tempest/Assert>
#include <Tempest/GraphicsSubsystem>

#include <map>

namespace Tempest{

class VertexShader;
class FragmentShader;

class Matrix4x4;
class Texture2d;
class Texture3d;

class ShaderInput;

class AbstractShadingLang : public GraphicsSubsystem {
  public:
    virtual ~AbstractShadingLang(){}

    virtual void beginPaint() const {}
    virtual void endPaint()   const {}

    virtual void enable()  const {}
    virtual void disable() const {}

    virtual void bind( const Tempest::VertexShader& ) const = 0;
    virtual void bind( const Tempest::FragmentShader& ) const = 0;

    virtual void unBind( const Tempest::VertexShader& ) const = 0;
    virtual void unBind( const Tempest::FragmentShader& ) const = 0;

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
        }

      return 0;
      }

    virtual VertexShader*
                 createVertexShader( const std::string& fname,
                                     std::string & outputLog ) const;
    virtual VertexShader*
                 createVertexShader( const std::u16string& fname,
                                     std::string & outputLog ) const;
    virtual VertexShader*
                 createVertexShaderFromSource( const std::string& src,
                                               std::string & outputLog ) const = 0;

    virtual FragmentShader*
                 createFragmentShader( const std::string& fname,
                                       std::string & outputLog ) const;
    virtual FragmentShader*
                 createFragmentShader( const std::u16string& fname,
                                       std::string &log ) const;
    virtual FragmentShader*
                 createFragmentShaderFromSource( const std::string& src,
                                                 std::string & outputLog ) const = 0;

    void deleteShader( VertexShader* s ) const {
      deleteVertexShader(s);
      }

    void deleteShader( FragmentShader* s ) const {
      deleteFragmentShader(s);
      }
    virtual void deleteVertexShader( VertexShader* s ) const = 0;
    virtual void deleteFragmentShader( FragmentShader* s ) const = 0;

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
    static AbstractAPI::Texture* get( const Tempest::Texture2d & s );
    static AbstractAPI::Texture* get( const Tempest::Texture3d & s );

    static const ShaderInput &inputOf( const Tempest::VertexShader   & s );
    static const ShaderInput &inputOf( const Tempest::FragmentShader & s );

  };

}

#endif // SHADINGLANG_H
