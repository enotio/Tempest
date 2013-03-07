#ifndef VERTEXSHADERHOLDER_H
#define VERTEXSHADERHOLDER_H

#include <Tempest/AbstractHolder>
#include <Tempest/AbstractShadingLang>

namespace Tempest{

class VertexShader;
class Device;

class VertexShaderHolder : public
        AbstractHolderWithLoad< Tempest::VertexShader,
                                AbstractShadingLang::VertexShader > {
  public:
    typedef AbstractHolderWithLoad< Tempest::VertexShader,
                                    AbstractShadingLang::VertexShader  > BaseType;
    VertexShaderHolder( Device &d );
    ~VertexShaderHolder();

    using BaseType::load;

    VertexShader loadFromSource( const std::string& src);
  protected:
    virtual void createObjectFromSrc( AbstractShadingLang::VertexShader*& t,
                                      const std::string & src );
    virtual void createObject( AbstractShadingLang::VertexShader*& t,
                               const std::string & fname );
    virtual void deleteObject( AbstractShadingLang::VertexShader* t );


    virtual void  reset( AbstractShadingLang::VertexShader* t );
    virtual AbstractShadingLang::VertexShader*
                  restore( AbstractShadingLang::VertexShader* t );
    virtual AbstractShadingLang::VertexShader*
                  copy( AbstractShadingLang::VertexShader* t );
  private:
    VertexShaderHolder( const VertexShaderHolder &h );
    void operator = ( const VertexShaderHolder& ){}

    struct Data;
    Data  *data;
  };

}

#endif // VERTEXSHADERHOLDER_H
