#ifndef RENDER_H
#define RENDER_H

#include <Tempest/Device>
#include <Tempest/RenderState>
#include <Tempest/UniformTable>

#include <Tempest/VertexShader>
#include <Tempest/FragmentShader>

#include <Tempest/AbstractMaterial>

namespace Tempest{

class Color;

template< class ModelVertex >
class Model;

class AbstractMaterial;
class AbstractSceneObject;
class AbstractCamera;

class Render {
  public:
    Render( Tempest::Device & d,
            const Tempest::VertexShader   & vs,
            const Tempest::FragmentShader & fs );

    Render( Tempest::Device & d,
            Texture2d &rt,
            const Tempest::VertexShader   & vs,
            const Tempest::FragmentShader & fs );

    Render( Tempest::Device & d,
            Texture2d &rt,
            Tempest::Texture2d & depthStencil,
            const Tempest::VertexShader   & vs,
            const Tempest::FragmentShader & fs );

    Render( Tempest::Device & d,
            Texture2d rt[], int count,
            const Tempest::VertexShader   & vs,
            const Tempest::FragmentShader & fs );

    Render( Tempest::Device & d,
            Texture2d rt[], int count,
            Tempest::Texture2d & tex,
            const Tempest::VertexShader   & vs,
            const Tempest::FragmentShader & fs );
    ~Render();

    void clear( const Tempest::Color & cl, double z );
    void clear( const Tempest::Color & cl );

    void draw( const AbstractMaterial      & mat,
               const AbstractSceneObject   & obj,
               const Tempest::Matrix4x4       & object,
               const AbstractCamera        & camera  );

    void draw( const AbstractSceneObject   & obj);

    template< class V >
    void draw ( const AbstractMaterial & mat,
                const Model<V> & model,
                const Tempest::Matrix4x4 & object,
                const AbstractCamera & camera  ){
      Tempest::UniformTable table( device, vsh, fsh );
      Tempest::RenderState rs;

      if( mat.bind( rs, object, camera, table ) ){
        device.setRenderState(rs);

        draw(model);

        mat.ubind();
      }
    }

    template< class V >
    void draw ( const Model<V> & model ){
      device.drawPrimitive( model.primitiveType(),
                            vsh,
                            fsh,
                            model.declaration(),
                            model.vertexes(),
                            0,
                            model.size() );
      }

    void setRenderState( const RenderState& s );
  private:
    Tempest::Device & device;
    Tempest::RenderState state;

    Tempest::VertexShader   vsh;
    Tempest::FragmentShader fsh;

  friend class UniformTable;
  };

}

#endif // RENDER_H
