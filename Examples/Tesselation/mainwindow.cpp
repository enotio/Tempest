#include "mainwindow.h"

#include <Tempest/Log>
#include <Tempest/RenderState>

MainWindow::Vertex MainWindow::quadVertices[] = {
  { -1.0f,-1.0f, 1.0f, 0.0f,0.0f, {0,0, 1}, { 1,0,0} },
  {  1.0f,-1.0f, 1.0f, 1.0f,0.0f, {0,0, 1}, { 1,0,0} },
  {  1.0f, 1.0f, 1.0f, 1.0f,1.0f, {0,0, 1}, { 1,0,0} },
  { -1.0f, 1.0f, 1.0f, 0.0f,1.0f, {0,0, 1}, { 1,0,0} },

  { -1.0f,-1.0f,-1.0f, 1.0f,0.0f, {0,0,-1}, {-1,0,0} },
  { -1.0f, 1.0f,-1.0f, 1.0f,1.0f, {0,0,-1}, {-1,0,0} },
  {  1.0f, 1.0f,-1.0f, 0.0f,1.0f, {0,0,-1}, {-1,0,0} },
  {  1.0f,-1.0f,-1.0f, 0.0f,0.0f, {0,0,-1}, {-1,0,0} },

  { -1.0f, 1.0f,-1.0f, 0.0f,1.0f, {0, 1,0}, { 1,0,0} },
  { -1.0f, 1.0f, 1.0f, 0.0f,0.0f, {0, 1,0}, { 1,0,0} },
  {  1.0f, 1.0f, 1.0f, 1.0f,0.0f, {0, 1,0}, { 1,0,0} },
  {  1.0f, 1.0f,-1.0f, 1.0f,1.0f, {0, 1,0}, { 1,0,0} },

  { -1.0f,-1.0f,-1.0f, 1.0f,1.0f, {0,-1,0}, {-1,0,0} },
  {  1.0f,-1.0f,-1.0f, 0.0f,1.0f, {0,-1,0}, {-1,0,0} },
  {  1.0f,-1.0f, 1.0f, 0.0f,0.0f, {0,-1,0}, {-1,0,0} },
  { -1.0f,-1.0f, 1.0f, 1.0f,0.0f, {0,-1,0}, {-1,0,0} },

  {  1.0f,-1.0f,-1.0f, 1.0f,0.0f, { 1,0,0}, {0,0,-1} },
  {  1.0f, 1.0f,-1.0f, 1.0f,1.0f, { 1,0,0}, {0,0,-1} },
  {  1.0f, 1.0f, 1.0f, 0.0f,1.0f, { 1,0,0}, {0,0,-1} },
  {  1.0f,-1.0f, 1.0f, 0.0f,0.0f, { 1,0,0}, {0,0,-1} },

  { -1.0f,-1.0f,-1.0f, 0.0f,0.0f, {-1,0,0}, {0,0, 1} },
  { -1.0f,-1.0f, 1.0f, 1.0f,0.0f, {-1,0,0}, {0,0, 1} },
  { -1.0f, 1.0f, 1.0f, 1.0f,1.0f, {-1,0,0}, {0,0, 1} },
  { -1.0f, 1.0f,-1.0f, 0.0f,1.0f, {-1,0,0}, {0,0, 1} }
  };

uint16_t MainWindow::quadIndexes[] = {
  0,  1,  2,  0,  2, 3,
  4,  5,  6,  4,  6, 7,
  8,  9, 10,  8, 10, 11,
  12, 13, 14, 12, 14, 15,
  16, 17, 18, 16, 18, 19,
  20, 21, 22, 20, 22, 23
  };

using namespace Tempest;

MainWindow::MainWindow(Tempest::AbstractAPI &api)
  : device( api, options(), handle() ),
    texHolder( device ),
    vboHolder( device ),
    iboHolder( device ),
    shHolder ( device ) {
  mProj.perspective( 45, w()/double(h()), 0.1, 100.0 );

  udecl.add("modelMatrix",Decl::Matrix4x4)
       .add("mvpMatrix",  Decl::Matrix4x4)
       .add("diffuse",    Decl::Texture2d)
       .add("heightMap",  Decl::Texture2d);

  vbo     = vboHolder.load( quadVertices, 24  );
  ibo     = iboHolder.load( quadIndexes,  6*6 );

  texture = texHolder.load("data/rocks.ktx");
  normal  = texHolder.load("data/rocks_norm.png");
  height  = texHolder.load("data/rock_h.png");

  shader = shHolder.load({"shader/tess.vs.glsl",
                          "shader/tess.fs.glsl",
                          "",
                          "shader/tess.es.glsl",
                          "shader/tess.ts.glsl"});

  VertexDeclaration::Declarator decl;
  decl.add( Decl::float3, Usage::Position )
      .add( Decl::float2, Usage::TexCoord )
      .add( Decl::float3, Usage::Normal   )
      .add( Decl::float3, Usage::BiNormal );

  vdecl = VertexDeclaration( device, decl );

  if( !shader.isValid() )
    Log::e("sh:",shader.log());
  }

void MainWindow::render() {
  if( !device.startRender() )
    return;

  device.beginPaint();
  device.clear( Tempest::Color(0,0,1), 1.0 );

  setShaderConstants( spin.x, spin.y, texture, normal, height );

  shader.setUniform(ubo,udecl,0);
  device.drawIndexed( AbstractAPI::PrimitiveType(0),
                      shader, vdecl,
                      vbo, ibo,
                      0, 0,
                      ibo.size()/3 );

  device.endPaint();
  device.present();
  }

void MainWindow::resizeEvent(Tempest::SizeEvent &) {
  mProj.perspective( 45, w()/double(h()), 0.1, 100.0 );
  device.reset( options() );
  }

void MainWindow::setShaderConstants( float spinX, float spinY,
                                     const Texture2d &tex,
                                     const Texture2d &/*normal*/,
                                     const Texture2d &height  ) {
  Matrix4x4 mvpMatrix, projective, view;

  projective.perspective( 45.0, (float)w()/h(), 0.1, 100.0 );

  view.translate(0,0,4);
  view.rotate(spinY, 1, 0, 0);
  view.rotate(spinX, 0, 1, 0);

  mvpMatrix = projective;
  mvpMatrix.mul(view);

  ubo.modelView = view;
  ubo.mvpMatrix = mvpMatrix;
  ubo.heightMap = height;
  ubo.diffuse   = tex;
  }

void MainWindow::mouseDownEvent(MouseEvent &e){
  mpos = e.pos();
  }

void MainWindow::mouseDragEvent(MouseEvent &e) {
  spin += (e.pos() - mpos);
  mpos = e.pos();
  }

Device::Options MainWindow::options() {
  Device::Options opt;

  opt.vSync = true;

  return opt;
  }
