#ifndef FRAGMENTSHADERHOLDER_H
#define FRAGMENTSHADERHOLDER_H

#include <Tempest/AbstractHolder>

#include <Tempest/AbstractShadingLang>

namespace Tempest{

class FragmentShader;
class Device;

class FragmentShaderHolder : public
        AbstractHolderWithLoad< Tempest::FragmentShader,
                        AbstractShadingLang::FragmentShader > {
  public:
    typedef AbstractHolderWithLoad< Tempest::FragmentShader,
                            AbstractShadingLang::FragmentShader  > BaseType;
    FragmentShaderHolder( Device &d );
    ~FragmentShaderHolder();

    using BaseType::load;

    FragmentShader loadFromSource( const std::string& src);
  protected:
    virtual void createObjectFromSrc( AbstractShadingLang::FragmentShader*& t,
                                      const std::string & src );
    virtual void createObject( AbstractShadingLang::FragmentShader*& t,
                               const std::wstring & fname );
    virtual void deleteObject( AbstractShadingLang::FragmentShader* t );

    virtual void reset( AbstractShadingLang::FragmentShader* t );
    virtual AbstractShadingLang::FragmentShader*
                  restore( AbstractShadingLang::FragmentShader* t );
    virtual AbstractShadingLang::FragmentShader*
                  copy( AbstractShadingLang::FragmentShader* t );
  private:
    FragmentShaderHolder( const FragmentShaderHolder &h );
    void operator = ( const FragmentShaderHolder& ){}

    struct Data;
    Data  *data;
  };

}

#endif // FRAGMENTSHADERHOLDER_H
