#ifndef ABSTRACTSHADINGLANG_H
#define ABSTRACTSHADINGLANG_H

#include <string>
#include <Tempest/AbstractAPI>

#include <map>

namespace Tempest{

class VertexShader;
class FragmentShader;

class Matrix4x4;
class Texture2d;

class ShaderInput;

class AbstractShadingLang {
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

    class VertexShader;
    class FragmentShader;

    virtual void* context() const = 0;

    enum ShaderType{
      Vertex,
      Fragment/*,
      Geometry*/
      };
    virtual void* createShader( ShaderType t,
                                const std::string& fname,
                                std::string & outputLog ) const;
    virtual void* createShader( ShaderType t,
                                const std::wstring& fname,
                                std::string & outputLog ) const;
    virtual void* createShaderFromSource( ShaderType t,
                                          const std::string& src,
                                          std::string & outputLog ) const {
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
                 createVertexShader( const std::wstring& fname,
                                     std::string & outputLog ) const;
    virtual VertexShader*
                 createVertexShaderFromSource( const std::string& src,
                                               std::string & outputLog ) const = 0;

    virtual FragmentShader*
                 createFragmentShader( const std::string& fname,
                                       std::string & outputLog ) const;
    virtual FragmentShader*
                 createFragmentShader( const std::wstring& fname,
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

    static const ShaderInput &inputOf( const Tempest::VertexShader   & s );
    static const ShaderInput &inputOf( const Tempest::FragmentShader & s );
  };

}

#endif // SHADINGLANG_H
