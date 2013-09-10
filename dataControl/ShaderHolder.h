#ifndef SHADERHOLDER_H
#define SHADERHOLDER_H

#include <Tempest/AbstractHolder>
#include <Tempest/AbstractShadingLang>
#include <Tempest/SystemAPI>

namespace Tempest{

class VertexShader;
class Device;

template< class Shader, class APIDescriptor, AbstractShadingLang::ShaderType type >
class ShaderHolder : public AbstractHolderWithLoad< Shader, APIDescriptor > {
  public:
    typedef AbstractHolderWithLoad< Shader, APIDescriptor  > BaseType;

    ShaderHolder( Device &d ):BaseType( d ) {
      data = new Data();
      }

    ~ShaderHolder(){
      delete data;
      }

    using BaseType::load;

    Shader loadFromSource( const std::string& src){
      Shader obj( *this );

      createObjectFromSrc( obj, obj.data.value(), src );
      return obj;
      }

    Shader surfaceShader( const AbstractShadingLang::UiShaderOpt &opt  ){
      return loadFromSource( this->device().surfaceShader(type, opt) );
      }

    Shader surfaceShader( const AbstractShadingLang::UiShaderOpt &opt,
                          bool& hasHalfpixOffset ){
      return loadFromSource( this->device().
                             surfaceShader(type, opt, hasHalfpixOffset) );
      }
  protected:
    virtual void createObjectFromSrc( Shader &obj,
                                      APIDescriptor*& t,
                                      const std::string & src ) {
      t = (APIDescriptor*)this->device().shadingLang().
          createShaderFromSource( type, src, obj.logv );
      data->shadersSrc[t] = src;
      }

    virtual void createObjectFromSrc( APIDescriptor*& t,
                                      const std::string & src ){
      std::string dummy;

      t = (APIDescriptor*)this->device().shadingLang().
          createShaderFromSource( type, src, dummy );
      data->shadersSrc[t] = src;
      }

    virtual void createObject( Shader& obj,
                               APIDescriptor*& t,
                               const std::wstring & fname ){
      createObjectFromSrc( obj, t, SystemAPI::loadText(fname) );
      }

    virtual void createObject( APIDescriptor*& t,
                               const std::wstring & fname ){
      createObjectFromSrc( t, SystemAPI::loadText(fname) );
      }

    virtual void deleteObject( APIDescriptor* t ){
      reset(t);
      }

    virtual void  reset( APIDescriptor* t ){
      if( t==0 )
        return;

      data->restoreSrc[t] = data->shadersSrc[t];
      data->shadersSrc.erase(t);

      this->device().shadingLang().deleteShader(t);
      }

    virtual APIDescriptor* restore( APIDescriptor* t ){
      if( t==0 )
        return 0;

      std::string src = data->restoreSrc[t];
      data->restoreSrc.erase(t);

      createObjectFromSrc( t, src );
      return t;
      }

    virtual APIDescriptor* copy( APIDescriptor* t ){
      if( t==0 )
        return 0;

      APIDescriptor* ret = 0;

      createObjectFromSrc( ret, data->shadersSrc[t] );
      return ret;
      }

  private:
    ShaderHolder( const ShaderHolder &h )            = delete;
    ShaderHolder& operator = ( const ShaderHolder& ) = delete;

    struct Data{
      std::map< APIDescriptor*, std::string >
        shadersSrc,
        restoreSrc;

      typedef typename std::map< APIDescriptor*, std::string >::iterator iterator;
      };
    Data  *data;
  };

}

#endif // SHADERHOLDER_H
