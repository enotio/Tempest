#ifndef HLSL_H
#define HLSL_H

#include <Tempest/AbstractShadingLang>
#include <Tempest/AbstractAPI>

namespace Tempest {

class HLSL : public AbstractShadingLang {
  public:
    HLSL( AbstractAPI::DirectX9Device *dev );
    ~HLSL();

    void enable() const;
    void endPaint() const;

    void bind( const Tempest::ShaderProgram&  ) const;

    void* context() const;

    std::string surfaceShader( ShaderType t, const UiShaderOpt&,
                               bool& hasHalfpixOffset ) const;
    Source surfaceShader( const AbstractShadingLang::UiShaderOpt &opt,
                          bool &hasHalfPixelOffset ) const;
    ProgramObject* createShaderFromSource( const Source& src,
                                           std::string & outputLog ) const;
    void           deleteShader( ProgramObject* s ) const;
  private:
    struct Data;
    Data *data;

    void setDevice() const;
    static void setNullDevice();

    template< class Sh>
    void setUniforms( Sh* prog,
                      const UBO & input,
                      bool textures ) const;

  };

}

#endif // HLSL_H
