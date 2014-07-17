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

    void bind( const Tempest::VertexShader& ) const;
    void bind( const Tempest::FragmentShader& ) const;

    void* context() const;
    void setVertexDecl( const Tempest::AbstractAPI::VertexDecl*  ) const;

    VertexShader* createVertexShaderFromSource( const std::string& src,
                                                std::string & outputLog ) const;
    void          deleteShader( VertexShader* s ) const;

    FragmentShader* createFragmentShaderFromSource( const std::string& src,
                                                    std::string & outputLog ) const;
    void            deleteShader( FragmentShader* s ) const;

    std::string surfaceShader( ShaderType t, const UiShaderOpt&,
                               bool& hasHalfpixOffset ) const;
  private:
    struct Data;
    Data *data;

    void setDevice() const;
    static void setNullDevice();

    template< class Sh>
    void setUniforms( Sh* prog,
                      const ShaderInput & input,
                      bool textures ) const;
    void event(const DeleteEvent &e);

  };

}

#endif // HLSL11_H
