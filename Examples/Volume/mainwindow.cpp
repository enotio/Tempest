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
    texHolder(device),
    vboHolder(device),
    iboHolder(device),
    shHolder (device)
    {
  zoom = 1;
  udecl.add("mvpMatrix", Decl::Matrix4x4)
       .add("viewInv",   Decl::Matrix4x4)
       .add("volume",    Decl::Texture3d)
       .add("grad",      Decl::Texture2d);

  vbo = vboHolder.load(quadVb, sizeof(quadVb)/sizeof(quadVb[0]));
  ibo = iboHolder.load(quadId, sizeof(quadId)/sizeof(quadId[0]));

  VertexDeclaration::Declarator decl;
  decl.add( Decl::float3, Usage::Position )
      .add( Decl::float2, Usage::TexCoord );
  vdecl = VertexDeclaration(device, decl);

  shader = shHolder.load({"shader/volume.vs.glsl",
                          "shader/volume.fs.glsl",
                          "","",""});

  if( !shader.isValid() ){
    Log::e(shader.log());
    }
  T_ASSERT( shader.isValid() );

  loadData();
  grad = texHolder.load("data/colors.png");
  }

void MainWindow::loadData() {
  RFile file("data/dataset-stagbeetle-416x416x247.dat");

  uint16_t x,y,z;
  file.readData( (char*)&x, 2 );
  file.readData( (char*)&y, 2 );
  file.readData( (char*)&z, 2 );

  std::vector<uint16_t> xdata( x*y*z );
  file.readData( (char*)&xdata[0], xdata.size()*2 );

  std::vector<uint16_t> data( x*y*z*4 );

  for( int i=0; i<x; ++i )
    for( int r=0; r<y; ++r )
      for( int q=0; q<z; ++q ){
        auto* vx = &data[ ((i*y+r)*z+q)*4 ];

        vx[0] = i;
        vx[1] = r;
        vx[2] = q;

        if( i==0||i==x-1 ||
            r==0||r==y-1 ||
            q==0||q==z-1 )
          vx[3] = 0; else
          vx[3] = xdata[((i*y+r)*z+q)];
        }

  volume = volHolder.load(x,y,z, data.data(), Texture3d::Format::RGBA16 );

  Texture3d::Sampler s;
  s.setClamping  ( Texture3d::ClampMode::ClampToEdge );
  s.setFiltration( Texture3d::FilterType::Nearest    );

  volume.setSampler(s);

  float sz = std::max( std::max(x,y), z );
  modeSize[0] = x/sz;
  modeSize[1] = y/sz;
  modeSize[2] = z/sz;

  view.scale( modeSize[0], modeSize[1], modeSize[2] );
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
  Matrix4x4 mvpMatrix, projective, view;

  projective.perspective( 45.0f, (float)w()/h(), 0.1f, 100.0f );

  view.translate(0,0,4);
  view.rotate(float(rotate.y), 1, 0, 0);
  view.rotate(float(rotate.x), 0, 0, 1);
  view.scale(zoom);
  view.mul( this->view );

  mvpMatrix = projective;
  mvpMatrix.mul(view);

  ubo.mvpMatrix    = mvpMatrix;
  view.inverse();
  ubo.mvpMatrixInv = view;
  ubo.volume       = volume;
  ubo.grad         = grad;
  sh.setUniform(ubo,udecl,0);
  }
