#ifndef CGDX9_H
#define CGDX9_H

#ifdef __WINDOWS__

#include <Tempest/AbstractShadingLang>
#include <Tempest/AbstractAPI>

namespace Tempest {

class CgDx9 : public AbstractShadingLang {
  public:
    CgDx9( AbstractAPI::DirectX9Device *dev );
    ~CgDx9();

    void enable() const;

    void* context() const;

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

#endif // __WINDOWS__
#endif // CGDX9_H
