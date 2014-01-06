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
    volHolder(device),
    vboHolder(device),
    iboHolder(device),
    vsHolder (device),
    fsHolder (device)
    {
  zoom = 1;

  vbo = vboHolder.load(quadVb, sizeof(quadVb)/sizeof(quadVb[0]));
  ibo = iboHolder.load(quadId, sizeof(quadId)/sizeof(quadId[0]));

  VertexDeclaration::Declarator decl;
  decl.add( Decl::float3, Usage::Position )
      .add( Decl::float2, Usage::TexCoord );
  vdecl = VertexDeclaration(device, decl);

  shader.vs = vsHolder.load("shader/volume.vs.glsl");
  shader.fs = fsHolder.load("shader/volume.fs.glsl");

  if( !shader.isValid() ){
    Log() << shader.vs.log();
    Log() << shader.fs.log();
    }
  T_ASSERT( shader.isValid() );

  loadData();
  }

void MainWindow::loadData() {
  const int x = 256, y = 256, z = 256;

  std::vector<uint8_t> data( x*y*z*4 );

  for( int i=0; i<x; ++i )
    for( int r=0; r<y; ++r )
      for( int q=0; q<z; ++q ){
        uint8_t* vx = &data[ ((i*y+r)*z+q)*4 ];

        vx[0] = i;
        vx[1] = r;
        vx[2] = q;

        float px = 2.0*(i-x/2)/x,
              py = 2.0*(r-y/2)/y,
              pz = 2.0*(q-z/2)/z;
        double l = sqrt( px*px+py*py+pz*pz );

        if( i==0||i==x-1 ||
            r==0||r==y-1 ||
            q==0||q==z-1 )
          vx[3] = 0; else
          vx[3] = l<=1.0? 255:0;
        }

  volume = volHolder.load(x,y,z, data.data(), Texture3d::Format::RGBA8 );

  Texture3d::Sampler s;
  s.setClamping( Texture3d::ClampMode::Clamp );

  volume.setSampler(s);
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

void MainWindow::setupShaderConstants( ProgramObject &sh ) {
  Matrix4x4 mvpMatrix, projective, view;

  projective.perspective( 45.0, (float)w()/h(), 0.1, 100.0 );

  view.translate(0,0,4);
  view.rotate(rotate.y, 1, 0, 0);
  view.rotate(rotate.x, 0, 1, 0);
  view.scale(zoom);

  mvpMatrix = projective;
  mvpMatrix.mul(view);

  sh.vs.setUniform("mvpMatrix",    mvpMatrix);
  view.inverse();
  sh.vs.setUniform("mvpMatrixInv", view);
  sh.fs.setUniform("volume",       volume  );
  }
