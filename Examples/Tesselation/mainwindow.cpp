#include "mainwindow.h"

#include <Tempest/Log>

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
    vsHolder ( device ),
    fsHolder ( device ),
    tsHolder ( device ),
    esHolder ( device ) {
  mProj.perspective( 45, w()/double(h()), 0.1, 100.0 );

  vbo     = vboHolder.load( quadVertices, 24  );
  ibo     = iboHolder.load( quadIndexes,  6*6 );

  texture = texHolder.load("data/rocks.ktx");
  normal  = texHolder.load("data/rocks_norm.png");
  height  = texHolder.load("data/rock_h.png");

  vs = vsHolder.load("shader/tess.vs.glsl");
  fs = fsHolder.load("shader/tess.fs.glsl");
  ts = tsHolder.load("shader/tess.ts.glsl");
  es = esHolder.load("shader/tess.es.glsl");

  VertexDeclaration::Declarator decl;
  decl.add( Decl::float3, Usage::Position )
      .add( Decl::float2, Usage::TexCoord )
      .add( Decl::float3, Usage::Normal   )
      .add( Decl::float3, Usage::BiNormal );

  vdecl = VertexDeclaration( device, decl );

  if( !vs.isValid() )
    Log() << vs.log();

  if( !fs.isValid() )
    Log() << fs.log();

  if( !ts.isValid() )
    Log() << ts.log();

  if( !es.isValid() )
    Log() << es.log();
  }

void MainWindow::render() {
  if( !device.startRender() )
    return;

  device.beginPaint();
  device.clear( Tempest::Color(0), 1.0 );

  setShaderConstants( spin.x, spin.y, texture, normal, height );

  device.drawIndexed( Tempest::AbstractAPI::Triangle,
                      vs, fs, vdecl,
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

  vs.setUniform( "modelView", mView );
  vs.setUniform( "mvpMatrix", mvp   );

  fs.setUniform( "lightDir",  0,0,-1 );
  fs.setUniform( "diffuse",   tex    );
  fs.setUniform( "bump",      normal );
  fs.setUniform( "heightMap", height );
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
