#include "mainwindow.h"

#include <Tempest/Assert>
#include <Tempest/RenderState>
#include <Tempest/Log>

static MainWindow::Vertex quadVb[] = {
  { 1,-1,-1,  1,1},
  {-1,-1,-1,  0,1},
  {-1, 1,-1,  0,0},
  { 1, 1,-1,  1,0},

  {-1,-1, 1,  1,1},
  { 1,-1, 1,  0,1},
  { 1, 1, 1,  0,0},
  {-1, 1, 1,  1,0},

  {-1,  1,-1, 1,0},
  {-1, -1,-1, 1,1},
  {-1, -1, 1, 0,1},
  {-1,  1, 1, 0,0},

  { 1, -1,-1,  0,1},
  { 1,  1,-1,  0,0},
  { 1,  1, 1,  1,0},
  { 1, -1, 1,  1,1},

  {-1, -1,-1,  0,0},
  { 1, -1,-1,  1,0},
  { 1, -1, 1,  1,1},
  {-1, -1, 1,  0,1},

  { 1,  1,-1,  1,1},
  {-1,  1,-1,  0,1},
  {-1,  1, 1,  0,0},
  { 1,  1, 1,  1,0},
  };

static uint16_t quadId[] = {
   0, 1, 2, 0, 2, 3,
   4, 5, 6, 4, 6, 7,
   8, 9,10, 8,10,11,
  12,13,14,12,14,15,
  16,17,18,16,18,19,
  20,21,22,20,22,23
  };

using namespace Tempest;

MainWindow::MainWindow(Tempest::AbstractAPI &api)
   :device( api, handle() ),
    texHolder(device),
    vboHolder(device),
    iboHolder(device),
    shHolder (device)
    {
  zoom = 1;
  udecl.add("mvpMatrix",Decl::Matrix4x4)
       .add("xtexture", Decl::Texture2d);

  vbo = vboHolder.load(quadVb, sizeof(quadVb)/sizeof(quadVb[0]));
  ibo = iboHolder.load(quadId, sizeof(quadId)/sizeof(quadId[0]));

  VertexDeclaration::Declarator decl;
  decl.add( Decl::float3, Usage::Position )
      .add( Decl::float2, Usage::TexCoord );
  vdecl = VertexDeclaration(device, decl);

  texture = texHolder.load("data/texture.png");

  shader = shHolder.load({"shader/basic11.vs.hlsl",
                          "shader/basic11.fs.hlsl",
                          "","",""});

  if( !shader.isValid() )
    Log() << "sh:" << shader.log();

  T_ASSERT( shader.isValid() );
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
    zoom *= 1.1; else
    zoom /= 1.1;
  }

void MainWindow::render() {
  if( !device.startRender() )
    return;

  device.clear( Color(0,0,1), 1 );

  device.beginPaint();
  setupShaderConstants(shader);
  device.drawIndexed( AbstractAPI::Triangle,
                      shader, vdecl,
                      vbo, ibo,
                      0,0, ibo.size()/3 );
  device.endPaint();

  device.present();
  }

void MainWindow::resizeEvent( SizeEvent & ) {
  device.reset();
  }

void MainWindow::setupShaderConstants( ShaderProgram &sh ) {
  Matrix4x4 projective, view;

  projective.perspective( 45.0, (float)w()/h(), 0.1, 100.0 );

  view.translate(0,0,4);
  view.rotate(rotate.y, 1, 0, 0);
  view.rotate(rotate.x, 0, 1, 0);
  view.scale(zoom);

  ubo.mvpMatrix = projective;
  ubo.mvpMatrix.mul(view);
  ubo.texture = texture;

  sh.setUniform(ubo,udecl,0);
  }
