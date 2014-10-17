#ifndef HLSL11_H
#define HLSL11_H

#include <Tempest/AbstractShadingLang>
#include <Tempest/AbstractAPI>

namespace Tempest {

class HLSL11 : public AbstractShadingLang {
  public:
    HLSL11( AbstractAPI::DirectX11Device *dev,
            void* context,
            unsigned featureLevel );
    ~HLSL11();

    void enable() const;

    void* context() const;
    void bind( const Tempest::ShaderProgram&  ) const;
    void setVertexDecl( const Tempest::AbstractAPI::VertexDecl*  ) const;

    ProgramObject* createShaderFromSource( const Source &src,
                                           std::string &outputLog) const;
    void           deleteShader(ProgramObject* ) const;
    Source         surfaceShader( const UiShaderOpt &opt,
                                  bool &hasHalfPixelOffset ) const;

    std::string    surfaceShader( ShaderType t, const UiShaderOpt&,
                                  bool& hasHalfpixOffset ) const;
  private:
    struct Data;
    Data *data;

    void setUniforms( const UBO &in,
                      int& slot ) const;

    void setDevice() const;
    static void setNullDevice();
    void event(const DeleteEvent &e);
  };

}

#endif // HLSL11_H
