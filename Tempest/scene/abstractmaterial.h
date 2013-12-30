#ifndef ABSTRACTMATERIAL_H
#define ABSTRACTMATERIAL_H

#include <cstdlib>

namespace Tempest{

class VertexShader;
class FragmentShader;
class Device;
class AbstractCamera;
class UniformTable;
class RenderState;

class Matrix4x4;

class AbstractMaterial {
  public:
    virtual ~AbstractMaterial(){}

    virtual bool bind ( RenderState& /*d*/,
                        const Tempest::Matrix4x4 & /*object*/,
                        const Tempest::AbstractCamera&,
                        Tempest::UniformTable & ) const { return 1; }
    virtual void ubind() const {}  

    virtual size_t uberID() const { return 0; }

    struct MaterialID{
      size_t mId, uberId;

      bool operator == ( const MaterialID & id ) const {
        return mId==id.mId && uberId==id.uberId;
        }
      bool operator != ( const MaterialID & id ) const {
        return !(*this==id);
        }

      bool operator < ( const MaterialID & id ) const {
        if( mId < id.mId )
          return 1;
        if( id.mId < mId )
          return 0;

        return uberId < id.uberId;
        }
      };

    template < class M >
    static MaterialID materialid( size_t uberId = 0 ){
      static MaterialID id = idGen();
      id.uberId = uberId;
      return id;
      }

    template < class M >
    static MaterialID materialid( const M& m ){
      MaterialID id = materialid<M>();
      id.uberId = m.uberID();
      return id;
      }

    private:
      static MaterialID idGen(){
        static MaterialID id = {0, 0};
        ++id.mId;
        return id;
        }
  };

}

#endif // ABSTRACTMATERIAL_H
