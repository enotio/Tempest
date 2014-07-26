#ifndef HLSL11_H
#define HLSL11_H

#include <Tempest/AbstractShadingLang>
#include <Tempest/AbstractAPI>

namespace Tempest {

class HLSL11 : public AbstractShadingLang {
  public:
    HLSL11( AbstractAPI::DirectX11Device *dev, void* context );
    ~HLSL11();

    void enable() const;

    void* context() const;
    void setVertexDecl( const Tempest::AbstractAPI::VertexDecl*  ) const;

    std::string surfaceShader( ShaderType t, const UiShaderOpt&,
                               bool& hasHalfpixOffset ) const;
  private:
    struct Data;
    Data *data;

    void setDevice() const;
    static void setNullDevice();
    void event(const DeleteEvent &e);
  };

}

#endif // HLSL11_H
