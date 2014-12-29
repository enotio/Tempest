#include "mainwindow.h"
#include "cube.h"

#include <Tempest/Assert>
#include <Tempest/RenderState>
#include <Tempest/Painter>

using namespace Tempest;

MainWindow::MainWindow(Tempest::AbstractAPI &api)
   :device( api, handle() ),
    texHolder(device),
    vboHolder(device),
    iboHolder(device),
    shHolder (device),
    spHolder (texHolder),
    uiRender (shHolder)
    {
  zoom = 1;
  udecl.add("mvpMatrix",Decl::Matrix4x4)
       .add("texture",  Decl::Texture2d);

  vbo = vboHolder.load(quadVb, sizeof(quadVb)/sizeof(quadVb[0]));
  ibo = iboHolder.load(quadId, sizeof(quadId)/sizeof(quadId[0]));

  VertexDeclaration::Declarator decl;
  decl.add( Decl::float3, Usage::Position )
      .add( Decl::float2, Usage::TexCoord );
  vdecl = VertexDeclaration(device, decl);

  texture   = texHolder.load("data/texture.png");

  shader = shHolder.load({"shader/mrt.vs.glsl",
                          "shader/mrt.fs.glsl",
                          "","",""});

  T_ASSERT( shader.isValid() );
  }

void MainWindow::paintEvent(PaintEvent &e) {
  Painter p(e);

  p.setTexture(texture);
  p.drawRect( Rect(0,0, 100, 100), texture.rect() );

  p.setFont( Font("data/arial", 16) );
  p.drawText(100, 100, "This is cat!");
  }

void MainWindow::mouseDownEvent(MouseEvent &e) {
  mpos = e.pos();
  }

void MainWindow::mouseDragEvent(MouseEvent &e) {
  rotate += (e.pos()-mpos);
  mpos = e.pos();
  }

void MainWindow::mouseWheelEvent(MouseEvent &e) {
  if( e.delta>0 )
    zoom *= 1.1f; else
    zoom /= 1.1f;
  }

void MainWindow::render() {
  if( !device.startRender() )
    return;

  uiRender.buildVbo(*this, vboHolder, iboHolder, spHolder );
  device.clear( Color(0,0,1), 1 );

  device.beginPaint();
  device.setRenderState( RenderState() );

  setupShaderConstants(shader);
  device.drawIndexed( AbstractAPI::Triangle,
                      shader, vdecl,
                      vbo, ibo,
                      0,0, ibo.size()/3 );
  device.draw( uiRender );
  device.endPaint();

  device.present();
  }

void MainWindow::resizeEvent( SizeEvent & ) {
  device.reset();
  }

void MainWindow::setupShaderConstants( ShaderProgram &sh ) {
  Matrix4x4 mvpMatrix, projective, view;

  projective.perspective( 45.0f, (float)w()/h(), 0.1f, 100.0f );

  view.translate(0,0,4);
  view.rotate(float(rotate.y), 1, 0, 0);
  view.rotate(float(rotate.x), 0, 1, 0);
  view.scale(zoom);

  mvpMatrix = projective;
  mvpMatrix.mul(view);

  ubo.mvpMatrix = mvpMatrix;
  ubo.texture   = texture;
  sh.setUniform(ubo,udecl,0);
  }
