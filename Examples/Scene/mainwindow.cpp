#include "mainwindow.h"

#include <Tempest/Assert>
#include <Tempest/RenderState>

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
  udecl.add("mvpMatrix",Decl::Matrix4x4)
       .add("xtexture", Decl::Texture2d);

  camera.setPerspective(w(), h());
  camera.setSpinX(45);
  camera.setSpinY(180+45);
  camera.setZoom(0.2);

  Tempest::VertexBuffer<Vertex>  vbo;
  Tempest::IndexBuffer<uint16_t> ibo;
  Tempest::VertexDeclaration     vdecl;

  vbo = vboHolder.load(quadVb, sizeof(quadVb)/sizeof(quadVb[0]));
  ibo = iboHolder.load(quadId, sizeof(quadId)/sizeof(quadId[0]));

  VertexDeclaration::Declarator decl;
  decl.add( Decl::float3, Usage::Position )
      .add( Decl::float2, Usage::TexCoord );
  vdecl = VertexDeclaration(device, decl);

  shader = shHolder.load({"shader/basic.vs.glsl",
                          "shader/basic.fs.glsl",
                          "","",""});

  T_ASSERT( shader.isValid() );

  Model<Vertex> m;
  m.load( vbo, ibo , vdecl );

  Material mat;
  mat.texture = texHolder.load("data/texture.png");

  for( int i=0; i<5; ++i ){
    SceneObject obj(scene);
    obj.setModel(m);
    obj.setMaterial(mat);
    obj.setPosition( i*2,0,0);
    objects.push_back( obj );
    }

  for( int i=1; i<10; ++i ){
    SceneObject obj(scene);
    obj.setModel(m);
    obj.setMaterial(mat);
    obj.setPosition( 0,i*2,0);
    objects.push_back( obj );
    }
  for( int i=1; i<3; ++i ){
    SceneObject obj(scene);
    obj.setModel(m);
    obj.setMaterial(mat);
    obj.setPosition( 0,0,i*2);
    objects.push_back( obj );
    }
  }

void MainWindow::mouseDownEvent(MouseEvent &e) {
  mpos = e.pos();
  }

void MainWindow::mouseDragEvent(MouseEvent &e) {
  auto rotate = (e.pos()-mpos);
  mpos = e.pos();

  camera.setSpinX( camera.spinX()+rotate.x );
  camera.setSpinY( camera.spinY()+rotate.y );
  }

void MainWindow::mouseWheelEvent(MouseEvent &e) {
  if( e.delta>0 )
    camera.setZoom( camera.zoom()*1.1 ); else
    camera.setZoom( camera.zoom()/1.1 );
  }

void MainWindow::render() {
  if( !device.startRender() )
    return;

  updateCamera();
  device.clear( Color(0,0,1), 1 );

  device.beginPaint();

  for( size_t i=0; i<objects.size(); ++i ){
    SceneObject &obj = objects[i];

    setupShaderConstants(obj, shader);
    device.draw( obj, shader );
    }

  device.endPaint();

  device.present();
  }

void MainWindow::resizeEvent( SizeEvent & ) {
  device.reset();
  camera.setPerspective( w(), h() );
  }

void MainWindow::setupShaderConstants( const SceneObject &obj, ShaderProgram &sh ) {
  ubo.mvpMatrix = scene.camera().projective();
  ubo.mvpMatrix.mul( scene.camera().view() );
  ubo.mvpMatrix.mul( obj.transform()       );
  ubo.texture = obj.material().texture;

  sh.setUniform(ubo,udecl,0);
  }

void MainWindow::updateCamera() {
  scene.setCamera( camera );
  }
