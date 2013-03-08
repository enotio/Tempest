#ifndef ABSTRACTSHADINGLANG_H
#define ABSTRACTSHADINGLANG_H

#include <string>
#include <Tempest/AbstractAPI>
#include <Tempest/Uniform>

#include <map>

namespace Tempest{

class VertexShader;
class FragmentShader;

class Matrix4x4;
class Texture2d;

template< class T >
class Uniform;

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

    class VertexShader;
    class FragmentShader;

    virtual void* context() const = 0;
    virtual VertexShader*
                 createVertexShader( const std::string& fname ) const = 0;
    virtual VertexShader*
                 createVertexShaderFromSource( const std::string& src ) const = 0;
    virtual void deleteVertexShader( VertexShader* s ) const = 0;

    virtual FragmentShader*
                 createFragmentShader( const std::string& fname ) const = 0;
    virtual FragmentShader*
                 createFragmentShaderFromSource( const std::string& src ) const = 0;
    virtual void deleteFragmentShader( FragmentShader* s ) const = 0;

  protected:
    static VertexShader*   get( const Tempest::VertexShader   & s );
    static FragmentShader* get( const Tempest::FragmentShader & s );
    static AbstractAPI::Texture* get( const Tempest::Texture2d & s );

    static const ShaderInput &inputOf( const Tempest::VertexShader   & s );
    static const ShaderInput &inputOf( const Tempest::FragmentShader & s );
  };

}

#endif // SHADINGLANG_H
