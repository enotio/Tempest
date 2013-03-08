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


    virtual void setUniform( Tempest::VertexShader &s,
                             const Uniform<float[2]> & u,
                             Detail::ShInput & in ) const = 0;

    virtual void setUniform( Tempest::VertexShader &s,
                             const Uniform<float[3]> & u,
                             Detail::ShInput & in ) const = 0;

    virtual void setUniform( Tempest::VertexShader &s,
                             const Matrix4x4& m,
                             const char* name) const = 0;

    virtual void setUniform( Tempest::VertexShader &s,
                             const float v[], int l,
                             const char* name ) const = 0;

    virtual void setUniform( Tempest::FragmentShader &s,
                             const Uniform<float[2]> & u,
                             Detail::ShInput & in ) const = 0;

    virtual void setUniform( Tempest::FragmentShader &s,
                             const Uniform<float[3]> & u,
                             Detail::ShInput & in ) const = 0;

    virtual void setUniform( Tempest::FragmentShader &s,
                             const Uniform<Texture2d> & u,
                             Detail::ShInput & in ) const = 0;

    virtual void setUniform( Tempest::FragmentShader &s,
                             const Matrix4x4& m,
                             const char* name ) const = 0;

    virtual void setUniform( Tempest::FragmentShader &s,
                             const Texture2d& t,
                             const char* name ) const = 0;

    virtual void setUniform( Tempest::FragmentShader &s,
                             const float v[], int l,
                             const char* name ) const = 0;

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

    template< int s >
    struct Vec{
      float data[s];
      };

    struct UniformBlock{
      std::map<std::string, Vec<1> > v1;
      std::map<std::string, Vec<2> > v2;
      std::map<std::string, Vec<3> > v3;
      std::map<std::string, Vec<4> > v4;
      };
  };

}

#endif // SHADINGLANG_H
