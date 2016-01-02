#include "mainwindow.h"

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
    shHolder ( device ){
  mProj.perspective( 45.f, w()/float(h()), 0.1f, 100.0f );
  udecl.add("modelView",Decl::Matrix4x4)
       .add("mvpMatrix",Decl::Matrix4x4)
       .add("lightDir", Decl::float3)
       .add("texture",  Decl::Texture2d)
       .add("bump",     Decl::Texture2d)
       .add("heightMap",Decl::Texture2d);

  vbo     = vboHolder.load( quadVertices, 24  );
  ibo     = iboHolder.load( quadIndexes,  6*6 );

  texture = texHolder.load("data/rocks.ktx");
  normal  = texHolder.load("data/rocks_norm.png");
  height  = texHolder.load("data/rock_h.png");

  shader = shHolder.load({"shader/bump_vs.glsl",
                          "shader/bump_fs.glsl",
                          "","",""});

  VertexDeclaration::Declarator decl;
  decl.add( Decl::float3, Usage::Position )
      .add( Decl::float2, Usage::TexCoord )
      .add( Decl::float3, Usage::Normal   )
      .add( Decl::float3, Usage::BiNormal );

  vdecl = VertexDeclaration( device, decl );
  }

void MainWindow::render() {
  if( !device.startRender() )
    return;

  device.beginPaint();
  device.clear( Tempest::Color(0,1,0), 1.0 );

  setShaderConstants( float(spin.x), float(spin.y), texture, normal, height );

  shader.setUniform(ubo,udecl,0);
  device.drawIndexed( Tempest::AbstractAPI::Triangle,
                      shader, vdecl,
                      vbo, ibo,
                      0, 0,
                      ibo.size()/3 );

  device.endPaint();
  device.present();
  }

void MainWindow::resizeEvent(Tempest::SizeEvent &) {
  mProj.perspective( 45.f, w()/float(h()), 0.1f, 100.0f );
  device.reset( options() );
  }

void MainWindow::setShaderConstants( float spinX, float spinY,
                                     const Texture2d &tex,
                                     const Texture2d &normal,
                                     const Texture2d &height  ) {
  Matrix4x4 mWorld, mView;

  mWorld.identity();
  mWorld.translate( 0, 0, 4 );
  mWorld.rotate( spinX, 0,1,0 );
  mWorld.rotate( spinY, 1,0,0 );
  mWorld.scale(0.75);

  mView.mul( mWorld );

  Matrix4x4 mvp = mProj;
  mvp.mul( mView  );

  ubo.modelView = mView;
  ubo.mvpMatrix = mvp;
  ubo.texture   = tex;
  ubo.bump      = normal;
  ubo.heightMap = height;
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
