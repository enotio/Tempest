#ifndef HLSL_H
#define HLSL_H

#include <Tempest/AbstractShadingLang>
#include <Tempest/AbstractAPI>

namespace Tempest {

class HLSL  : public AbstractShadingLang {
  public:
    HLSL( AbstractAPI::DirectX9Device *dev );
    ~HLSL();

    void enable() const;

    void bind( const Tempest::VertexShader& ) const;
    void bind( const Tempest::FragmentShader& ) const;

    void* context() const;

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

  };

}

#endif // HLSL_H
