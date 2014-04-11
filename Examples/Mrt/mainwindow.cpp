#include "mainwindow.h"

#include <Tempest/Assert>
#include <Tempest/RenderState>
#include <Tempest/Painter>

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
    texHolder (device),
    ltexHolder(device),
    vboHolder (device),
    iboHolder (device),
    lvboHolder(device),
    liboHolder(device),
    vsHolder  (device),
    fsHolder  (device),
    spHolder  (texHolder),
    uiRender  ( vsHolder, fsHolder )
    {
  zoom = 1;

  vbo = vboHolder.load(quadVb, sizeof(quadVb)/sizeof(quadVb[0]));
  ibo = iboHolder.load(quadId, sizeof(quadId)/sizeof(quadId[0]));

  VertexDeclaration::Declarator decl;
  decl.add( Decl::float3, Usage::Position )
      .add( Decl::float2, Usage::TexCoord );
  vdecl = VertexDeclaration(device, decl);

  texture         = texHolder.load("data/texture.png");

  renderTarget[0] = texHolder.create(512, 512, Texture2d::Format::RGBA);
  renderTarget[1] = texHolder.create(512, 512, Texture2d::Format::RGBA);

  shader.vs = vsHolder.load("shader/mrt.vs.glsl");
  shader.fs = fsHolder.load("shader/mrt.fs.glsl");

  T_ASSERT( shader.isValid() );  
  }

void MainWindow::paintEvent(PaintEvent &e) {
  Painter p(e);

  p.setTexture(renderTarget[0]);
  p.drawRect( Rect(0,0, 256, 256), renderTarget[0].rect() );

  p.setTexture(renderTarget[1]);
  p.drawRect( Rect(256,0, 256, 256), renderTarget[1].rect() );

  p.setFont( Font("data/arial", 16) );
  p.drawText(256, 256, "This is cat!");
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

void MainWindow::mrt( Texture2d* a ) {
  Texture2d depth = ltexHolder.create( a[0].width(),
                                       a[0].height(),
                                       Texture2d::Format::Depth24 );
  device.beginPaint(a, 2, depth);
  //device.beginPaint(a[0], depth);
  device.clear( Color(1.0), 1 );
  device.setRenderState( RenderState() );

  setupShaderConstants(shader, a[0].size());
  device.drawIndexed( AbstractAPI::Triangle,
                      shader, vdecl,
                      vbo, ibo,
                      0,0, ibo.size()/3 );
  device.endPaint();
  }

void MainWindow::render() {
  if( !device.startRender() )
    return;

  mrt(renderTarget);

  uiRender.buildVbo(*this, vboHolder, iboHolder, spHolder );

  device.beginPaint();
  device.clear( Color(0,0,1), 1 );
  device.draw( uiRender );
  device.endPaint();

  device.present();
  //uiRender.clearVbo();
  }

void MainWindow::resizeEvent( SizeEvent & ) {
  device.reset();
  }

void MainWindow::setupShaderConstants( ProgramObject &sh,
                                       const Size& rtSize ) {
  Matrix4x4 mvpMatrix, projective, view;

  projective.perspective( 45.0, (float)rtSize.w/rtSize.h, 0.1, 100.0 );

  view.translate(0,0,4);
  view.rotate(rotate.y, 1, 0, 0);
  view.rotate(rotate.x, 0, 1, 0);
  view.scale(zoom);

  mvpMatrix = projective;
  mvpMatrix.mul(view);

  sh.vs.setUniform("mvpMatrix", mvpMatrix);
  sh.fs.setUniform("texture",   texture  );
  }
