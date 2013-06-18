#include "render.h"

#include <Tempest/Model>
#include <Tempest/AbstractMaterial>
#include <Tempest/RenderState>
#include <Tempest/UniformTable>
#include <Tempest/AbstractGraphicObject>

using namespace Tempest;

Render::Render(Tempest::Device & d,
                VertexShader &vs, FragmentShader &fs )
  :device(d), vsh(vs), fsh(fs) {
  device.beginPaint();

  device.setRenderState( Tempest::RenderState() );
  }

Render::Render(Device &d, Texture2d &rt,
               VertexShader &vs, FragmentShader &fs)
  : device(d), vsh(vs), fsh(fs){
  device.beginPaint( rt );

  device.setRenderState( Tempest::RenderState() );
  }

Render::Render(Device &d, Texture2d &rt, Texture2d &depthStencil,
               VertexShader &vs, FragmentShader &fs)
  : device(d), vsh(vs), fsh(fs) {
  device.beginPaint( rt, depthStencil );

  device.setRenderState( Tempest::RenderState() );
  }

Render::Render(Tempest::Device & d, Texture2d rt[], int count,
                VertexShader &vs, FragmentShader &fs )
  : device(d), vsh(vs), fsh(fs) {
  device.beginPaint(rt, count);

  device.setRenderState( Tempest::RenderState() );
  }

Render::Render(Tempest::Device &d,
                Texture2d rt[], int count,
                Texture2d &tex,
                VertexShader &vs, FragmentShader &fs)
  : device(d), vsh(vs), fsh(fs) {
  device.beginPaint(rt, count, tex);

  device.setRenderState( Tempest::RenderState() );
  }

Render::~Render(){
  setRenderState( RenderState() );
  device.endPaint();
  }

void Render::clear( const Tempest::Color & cl, double z ){
  device.clear( cl, z );
  }

void Render::setRenderState(const RenderState &rs) {
  if( rs!=state ){
    device.setRenderState( rs );
    state = rs;
    }
  }

void Tempest::Render::clear( const Tempest::Color &cl ) {
  device.clear(cl);
  }

void Render::draw( const AbstractMaterial &mat,
                   const AbstractSceneObject &obj,
                   const Matrix4x4 &object,
                   const AbstractCamera &camera ) {
  obj.render( mat, *this, object, camera );
  }

void Render::draw( const AbstractSceneObject &obj ) {
  obj.render( *this );
  }
