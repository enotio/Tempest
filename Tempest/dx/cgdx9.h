#ifndef CGDX9_H
#define CGDX9_H

#include <Tempest/AbstractShadingLang>
#include <Tempest/AbstractAPI>

namespace Tempest {

class VertexShader;
class FragmentShader;

class Matrix4x4;

class CgDx9 : public AbstractShadingLang {
  public:
    CgDx9( AbstractAPI::DirectX9Device *dev );
    ~CgDx9();

    void enable() const;

    void bind( const Tempest::VertexShader& ) const;
    void bind( const Tempest::FragmentShader& ) const;

    void unBind( const Tempest::VertexShader& ) const;
    void unBind( const Tempest::FragmentShader& ) const;

    void* context() const;

    VertexShader* createVertexShaderFromSource( const std::string& src,
                                                std::string & outputLog ) const;
    void          deleteVertexShader( VertexShader* s ) const;

    FragmentShader* createFragmentShaderFromSource( const std::string& src,
                                                    std::string & outputLog ) const;
    void            deleteFragmentShader( FragmentShader* s ) const;

    std::string surfaceShader( ShaderType t, const UiShaderOpt&,
                               bool& hasHalfpixOffset ) const;
  private:
    struct Data;
    Data *data;

    void setDevice() const;
    static void setNullDevice();

    void endPaint() const;

    void setUniform( const Tempest::FragmentShader &s,
                     const Texture2d& t,
                     const char* name ) const;

    void setUniform( const Tempest::FragmentShader &s,
                     const Matrix4x4& m,
                     const char* name ) const;

    void setUniform( const Tempest::VertexShader &s,
                     const Matrix4x4& m,
                     const char* name ) const;


    void setUniform( const Tempest::FragmentShader &s,
                     const float v[],
                     int l,
                     const char* name ) const;
    void setUniform( const Tempest::VertexShader &s,
                     const float v[],
                     int l,
                     const char* name ) const;

    template< class Sh, class T >
    void setUniforms( const Sh & s, const T & vN, int c ) const;
  };

}

#endif // CGDX9_H
