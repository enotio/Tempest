#ifndef CGOGL_H
#define CGOGL_H

#include <Tempest/AbstractShadingLang>
#include <Tempest/AbstractAPI>
#include <Tempest/Uniform>

namespace Tempest {

class VertexShader;
class FragmentShader;

class Matrix4x4;

class CgOGL : public AbstractShadingLang {
  public:
    CgOGL(AbstractAPI::OpenGL2xDevice *dev );
    ~CgOGL();

    void beginPaint() const;
    void endPaint()   const;

    void bind( const Tempest::VertexShader& ) const;
    void bind( const Tempest::FragmentShader& ) const;

    void unBind( const Tempest::VertexShader& ) const;
    void unBind( const Tempest::FragmentShader& ) const;


    void setUniform( Tempest::VertexShader &s,
                     const Uniform<float[2]> & u,
                     Detail::ShInput & in ) const;

    void setUniform( Tempest::VertexShader &s,
                     const Uniform<float[3]> & u,
                     Detail::ShInput & in ) const;

    void setUniform( Tempest::VertexShader &s,
                     const Matrix4x4& m,
                     const char* name ) const;

    void setUniform( Tempest::VertexShader &s,
                     const float v[],
                     int l,
                     const char* name ) const;

    void setUniform( Tempest::FragmentShader &s,
                     const Uniform<float[2]> & u,
                     Detail::ShInput & in ) const;

    void setUniform( Tempest::FragmentShader &s,
                     const Uniform<float[3]> & u,
                     Detail::ShInput & in ) const;

    void setUniform( Tempest::FragmentShader &s,
                     const Uniform<Texture2d> & u,
                     Detail::ShInput & in ) const;

    void setUniform( Tempest::FragmentShader &s,
                     const Matrix4x4& m,
                     const char* name ) const;

    void setUniform( Tempest::FragmentShader &s,
                     const Texture2d& t,
                     const char* name ) const;

    void setUniform( Tempest::FragmentShader &s,
                     const float v[],
                     int l,
                     const char* name ) const;

    void* context() const;

    VertexShader* createVertexShader( const std::string& fname ) const;
    VertexShader* createVertexShaderFromSource( const std::string& src ) const;
    void          deleteVertexShader( VertexShader* s ) const;

    FragmentShader* createFragmentShader( const std::string& fname ) const;
    FragmentShader* createFragmentShaderFromSource( const std::string& src ) const;
    void            deleteFragmentShader( FragmentShader* s ) const;

  private:
    struct Data;
    Data *data;

    void setDevice() const;
    static void setNullDevice();
  };

}


#endif // CGOGL_H
