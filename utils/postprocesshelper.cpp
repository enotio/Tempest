#include "postprocesshelper.h"

#include <Tempest/VertexDeclaration>
#include <Tempest/VertexBufferHolder>

using namespace Tempest;

PostProcessHelper::PostProcessHelper( Tempest::VertexBufferHolder&  vb,
                                      Tempest::IndexBufferHolder &  ib )
  :vbo(vb), ibo(ib) {

  Tempest::VertexDeclaration::Declarator decl;
  decl.add( Tempest::Decl::double2, Tempest::Usage::Position )
      .add( Tempest::Decl::double2, Tempest::Usage::TexCoord );

  vdecl = Tempest::VertexDeclaration( vbo.device(), decl );

  Vertex q[6] = {
    {-1,-1,  0,1},
    { 1, 1,  1,0},
    { 1,-1,  1,1},

    {-1,-1,  0,1},
    {-1, 1,  0,0},
    { 1, 1,  1,0}
    };

  quad = vbo.load(q, 6);
  }

void PostProcessHelper::drawFullScreenQuad( Device &device,
                                            Tempest::VertexShader   & vs,
                                            Tempest::FragmentShader & fs ) const {
  device.drawPrimitive( AbstractAPI::Triangle, vs, fs, vdecl, quad, 0, 2 );
  }
