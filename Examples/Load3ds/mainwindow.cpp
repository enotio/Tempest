#include "mainwindow.h"

#include <Tempest/Log>
#include <assimp/Importer.hpp>

aiLogStream stream;

using namespace Tempest;

const Color cl[7] = {
  Color(1,0,0),
  Color(0,0,1),
  Color(1,1,0),
  Color(0,1,0),
  Color(1,0,1),
  Color(0,1,1),
  Color(1,0.8,0.5)
  };

MainWindow::MainWindow(Tempest::AbstractAPI &api)
  : device( api, handle() ),
    texHolder ( device ),
    ltexHolder( device ),
    vboHolder ( device ),
    iboHolder ( device ),
    vsHolder  ( device ),
    fsHolder  ( device ){
  spin.y = -40;
  zoom   = 0.01;

  //stream = aiGetPredefinedLogStream(aiDefaultLogStream_STDOUT,NULL);
  //aiAttachLogStream(&stream);

  VertexDeclaration::Declarator decl;
  decl.add( Decl::float3, Usage::Position )
      .add( Decl::float2, Usage::TexCoord )
      .add( Decl::float3, Usage::Normal   );

  vdecl   = VertexDeclaration( device, decl );
  texture = texHolder.load("data/marble.jpg");

  size_t id = 0;
  loadScene( "data/cubes.3ds",
             [&]( const std::vector<Vertex>   &vert,
                  const std::vector<uint16_t> &index ){
    Tempest::VertexBuffer<Vertex>   vbo;
    Tempest::IndexBuffer <uint16_t> ibo;
    vbo     = vboHolder.load( vert  );
    ibo     = iboHolder.load( index );

    Model<Vertex> md;
    md.load( vbo, ibo, vdecl );

    SceneObject obj(scene);
    Material mat;
    mat.diffuse = texture;
    mat.color   = cl[ (id+1)%7 ];

    if( id%5==1 )
      mat.shaderType = Material::Transparent;

    if( id==5 )
      mat.shaderType = Material::Emission;
    if( id==9 ){
      mat.shaderType = Material::Emission;
      mat.color = Color(1);
      }

    if( id==0 ){
      mat.color = Color(0.6);
      }

    obj.setMaterial( mat );
    obj.setModel(md);
    objects.push_back( obj );
    ++id;
    });

  loadShader( shader,  "data/main",
              {
                "some_def 1"
              } );
  loadShader( refract, "data/main",
              {
                "refractions",
                "emissionv 0.85"
              });
  loadShader( emission, "data/main",
              {
                "emissionv 1.0"
              });
  loadShader( shadow, "data/main",
              {
                "shadow_pass"
              });
  loadShader( shadow_refract, "data/main",
              {
                "shadow_pass",
                "refractions",
                "emissionv 1.0"
              });

  loadShader( gauss,   "data/post/gaus" );
  loadShader( bright,  "data/post/bright" );
  loadShader( combine, "data/post/combine" );

  loadShader( blt, "data/blt" );

  updateCamera();
  }

MainWindow::~MainWindow() {
  aiDetachAllLogStreams();
  }

void MainWindow::loadShader( ProgramObject &shader,
                             const std::string & file ) {
  shader.vs = vsHolder.load( file+".vs.glsl" );
  shader.fs = fsHolder.load( file+".fs.glsl" );

  if( !shader.isValid() ){
    Log() << shader.vs.log();
    Log() << shader.fs.log();
    }
  }

void MainWindow::loadShader( ProgramObject &shader,
                             const std::string &file,
                             const std::initializer_list<const char *> &def ) {
  shader.vs = vsHolder.load( file+".vs.glsl", def );
  shader.fs = fsHolder.load( file+".fs.glsl", def );

  if( !shader.isValid() ){
    Log() << shader.vs.log();
    Log() << shader.fs.log();
    }
  }

template< class F >
bool MainWindow::loadScene(const char* f, const F &func ){
  const aiScene *file3d = aiImportFile(f,aiProcessPreset_TargetRealtime_Fast);

  if( !file3d )
    return false;

  Matrix4x4 mt;
  mt.identity();
  mt.rotate(-90, 1,0,0);

  std::vector<Vertex>   vert;
  std::vector<uint16_t> index;
  vert .reserve(4048);
  index.reserve(4048);

  loadSceneImpl( func, vert, index, file3d, file3d->mRootNode, mt );

  aiReleaseImport(file3d);
  return true;
  }

template< class F, class Vx >
void MainWindow::loadSceneImpl( const F& func,
                                std::vector<Vx>       &vert,
                                std::vector<uint16_t> &index,
                                const aiScene * scene,
                                const aiNode  * nd,
                                const Tempest::Matrix4x4 &transform ){
  aiMatrix4x4 m = nd->mTransformation;
  aiTransposeMatrix4(&m);

  Matrix4x4 mx = transform;
  mx.mul( (float*)&m  );

  for (size_t n=0; n <1 && n < nd->mNumMeshes; ++n) {
    const aiMesh* mesh = scene->mMeshes[nd->mMeshes[n]];
    //apply_material(sc->mMaterials[mesh->mMaterialIndex]);
    vert.clear();
    index.clear();

    size_t base = vert.size();
    for( size_t i=0; i<mesh->mNumVertices; ++i )
      vert.push_back( mkVertex(mesh, i, mx) );

    for (size_t t = 0; t < mesh->mNumFaces; ++t) {
      const aiFace* face = &mesh->mFaces[t];

      for(size_t i = 2; i < face->mNumIndices; ++i) {
        index.push_back( base + face->mIndices[0  ] );
        index.push_back( base + face->mIndices[i-1] );
        index.push_back( base + face->mIndices[i  ] );
        }
      }

    func( vert, index );
    }

  for(size_t n = 0; n < nd->mNumChildren; ++n) {
    loadSceneImpl( func, vert, index, scene, nd->mChildren[n], mx);
    }
  }

void MainWindow::updateCamera() {
  Tempest::Camera cam;
  cam.setSpinX(  spin.x );
  cam.setSpinY( -spin.y );
  cam.setDistance(4);

  cam.setZoom( zoom );
  cam.setPerspective( w(), h() );

  scene.setCamera( cam );
  }

MainWindow::Vertex MainWindow::mkVertex( const aiMesh *mesh,
                                         size_t i,
                                         const Matrix4x4 & m ) {
  Vertex v;
  v.x = mesh->mVertices[i].x;
  v.y = mesh->mVertices[i].y;
  v.z = mesh->mVertices[i].z;

  if( mesh->mTextureCoords[0] ){
    v.tu = mesh->mTextureCoords[0]->x;
    v.tv = mesh->mTextureCoords[0]->y;
    } else {
    v.tu = 0;
    v.tv = 0;
    }

  m.project(v.x, v.y, v.z);

  if( mesh->mNormals ){
    v.normal[0] = mesh->mNormals[i].x;
    v.normal[1] = mesh->mNormals[i].y;
    v.normal[2] = mesh->mNormals[i].z;

    float w = 0;
    m.project( v.normal[0],
               v.normal[1],
               v.normal[2],
               w );
    } else {
    std::fill( v.normal, v.normal+3, 0 );
    }

  /*
  if( mesh->mColors[0] ){
    std::fill( v.color, v.color+4, 1 );
    } else {
    std::fill( v.color, v.color+4, 1 );
    }
  */

  return v;
  }

Texture2d MainWindow::mkBuffer( Size sz ){
  return mkBuffer( sz.w, sz.h );
  }

Texture2d MainWindow::mkBuffer( int w, int h ){
  Texture2d::Sampler sm;
  sm.uClamp = Texture2d::ClampMode::ClampToBorder;
  sm.vClamp = sm.uClamp;

  Texture2d r = ltexHolder.create( w, h, Texture2d::Format::RGBA16 );
  r.setSampler( sm );
  return r;
  }

void MainWindow::render() {
  if( !device.startRender() )
    return;

  setShadowConstants( shadow );

  const int smSz = 1024;
  Texture2d tex   = ltexHolder.create(smSz, smSz, Texture2d::Format::RGB10_A2),
            depth = ltexHolder.create(smSz, smSz, Texture2d::Format::Depth24 );

  RenderState st;
  st.setCullFaceMode( RenderState::CullMode::front );
  device.setRenderState(st);
  device.beginPaint(tex, depth);
  device.clear( Tempest::Color(1), 1.0 );
  for( size_t i=0; i<objects.size(); ++i ){
    SceneObject& obj = objects[i];

    if( obj.material().shaderType != Material::Transparent ){
      setShaderConstants( obj, shadow );
      device.draw( obj, shadow );
      }
    }
  device.endPaint();
  tex = distorsion(tex, depth, shadow_refract);

  mainPasss(shader, tex);

  device.present();
  }

Texture2d MainWindow::distorsion( const Texture2d& scene,
                                  Texture2d& depth,
                                  ProgramObject & refract ){
  Texture2d rt2   = mkBuffer( scene.size());
  device.beginPaint(rt2, depth);
  blt.    fs.setUniform("texture", scene);
  refract.fs.setUniform("screen",  scene);

  device.setRenderState( RenderState::PostProcess );
  device.drawFullScreenQuad( blt );

  RenderState st;
  st.setCullFaceMode( RenderState::CullMode::front );
  device.setRenderState(st);
  for( size_t i=0; i<objects.size(); ++i ){
    SceneObject& obj = objects[i];
    if( obj.material().shaderType == Material::Transparent ){
      setShaderConstants( obj, refract );
      device.draw( obj, refract );
      }
    }

  device.endPaint();

  Texture2d rt3 = mkBuffer( scene.size());
  device.beginPaint(rt3, depth);
  blt.    fs.setUniform("texture", rt2);
  refract.fs.setUniform("screen",  rt2);

  device.setRenderState( RenderState::PostProcess );
  device.drawFullScreenQuad( blt );

  st.setCullFaceMode( RenderState::CullMode::back );
  device.setRenderState(st);
  for( size_t i=0; i<objects.size(); ++i ){
    SceneObject& obj = objects[i];
    if( obj.material().shaderType == Material::Transparent ){
      setShaderConstants( obj, refract );
      device.draw( obj, refract );
      }
    }

  device.endPaint();
  return rt3;
  }

void MainWindow::mainPasss( ProgramObject &shader,
                            const Texture2d & sm ){
  setupLigting( scene, shader,   sm );
  setupLigting( scene, refract,  sm );
  setupLigting( scene, emission, sm );

  device.setRenderState( RenderState() );

  Texture2d rt    = mkBuffer(size()),
            depth = ltexHolder.create(size(), Texture2d::Format::Depth24 );

  device.beginPaint(rt, depth);
  device.clear( Tempest::Color(0), 1.0 );

  for( size_t i=0; i<objects.size(); ++i ){
    SceneObject& obj = objects[i];
    if( obj.material().shaderType == Material::Main ){
      setShaderConstants( obj, shader );
      device.draw( obj, shader );
      }
    }

  for( size_t i=0; i<objects.size(); ++i ){
    SceneObject& obj = objects[i];
    if( obj.material().shaderType == Material::Emission ){
      setShaderConstants( obj, emission );
      device.draw( obj, emission );
      }
    }
  device.endPaint();

  Tempest::Texture2d rt3 = distorsion(rt, depth, refract);

  const int bcount = 4;
  Texture2d bright[bcount];
  bright[0] = brightPass( resizeTex(rt3, Tempest::Size(256)) );
  for( int i=1; i<bcount; ++i ){
    Size sz = bright[i-1].size(), sz2 = Size(sz.w/4, sz.h/4);
    if( sz2.isEmpty() )
      sz2 = sz;

    bright[i] = resizeTex( bright[i-1], sz2 );
    }

  for( int i=0; i<bcount; ++i )
    bright[i] = blur(bright[i]);

  combinePass( rt3, bright );
  }

Texture2d MainWindow::brightPass( const Texture2d &tex ) {
  bright.fs.setUniform("image", tex);

  Texture2d rt    = ltexHolder.create( tex.size(), tex.format() ),
            depth = ltexHolder.create( tex.size(), Texture2d::Format::Depth24 );
  device.beginPaint(rt, depth);
  device.setRenderState( RenderState::PostProcess );
  device.drawFullScreenQuad( bright );
  device.endPaint();

  Texture2d::Sampler sm;
  sm.uClamp = Texture2d::ClampMode::ClampToBorder;
  sm.vClamp = sm.uClamp;
  rt.setSampler(sm);
  return rt;
  }

Texture2d MainWindow::resizeTex(const Texture2d &tex, const Size &nsz) {
  bright.fs.setUniform("image", tex);

  Texture2d rt    = ltexHolder.create( nsz, tex.format() ),
            depth = ltexHolder.create( nsz, Texture2d::Format::Depth24 );
  device.beginPaint(rt, depth);
  device.setRenderState( RenderState::PostProcess );
  device.drawFullScreenQuad( bright );
  device.endPaint();

  Texture2d::Sampler sm;
  sm.uClamp = Texture2d::ClampMode::ClampToBorder;
  sm.vClamp = sm.uClamp;
  rt.setSampler(sm);
  return rt;
  }

void MainWindow::combinePass( const Texture2d &tex,
                              const Texture2d *bloom ) {
  combine.fs.setUniform("image", tex);
  combine.fs.setUniform("bloom0", bloom[0]);
  combine.fs.setUniform("bloom1", bloom[1]);
  combine.fs.setUniform("bloom2", bloom[2]);
  combine.fs.setUniform("bloom3", bloom[3]);

  device.beginPaint();
  device.setRenderState( RenderState::PostProcess );
  device.drawFullScreenQuad( combine );
  device.endPaint();
  }

Texture2d MainWindow::blur( const Texture2d &tex ) {
  float dir[2] = { 1.0f/tex.width(), 0.0f };
  gauss.fs.setUniform("image", tex);
  gauss.fs.setUniform("dir",   dir, 2);

  Texture2d rt    = ltexHolder.create( tex.size(), tex.format() ),
            depth = ltexHolder.create( tex.size(), Texture2d::Format::Depth24 );
  device.beginPaint(rt, depth);
  device.setRenderState( RenderState::PostProcess );
  device.drawFullScreenQuad( gauss );
  device.endPaint();

  Texture2d::Sampler sm;
  sm.uClamp = Texture2d::ClampMode::ClampToBorder;
  sm.vClamp = sm.uClamp;
  rt.setSampler(sm);

  float dir2[2] = { 0.0f, 1.0f/tex.height() };
  gauss.fs.setUniform("image", rt  );
  gauss.fs.setUniform("dir",   dir2, 2);

  Texture2d out = ltexHolder.create( tex.size(), tex.format() );
  device.beginPaint(out, depth);
  device.setRenderState( RenderState::PostProcess );
  device.drawFullScreenQuad( gauss );
  device.endPaint();

  sm.uClamp = Texture2d::ClampMode::ClampToBorder;
  sm.vClamp = sm.uClamp;
  out.setSampler(sm);
  return out;
  }

void MainWindow::setupLigting( Scene &scene,
                               ProgramObject &shader,
                               const Tempest::Texture2d & shadow ) {
  const Matrix4x4& m = scene.camera().view();
  float ld[3];
  getLightDir( ld, m );

  shader.fs.setUniform("lightDir", ld, 3);
  shader.fs.setUniform("shadow", shadow );
  }

void MainWindow::setShaderConstants( SceneObject& obj,
                                     ProgramObject& shader ) {
  setShadowConstants( shader );

  Matrix4x4 view = scene.camera().view();
  view.mul( obj.transform()  );

  Matrix4x4 mvp = scene.camera().projective();
  mvp.mul( view );

  shader.vs.setUniform( "modelView", view );
  shader.vs.setUniform( "mvpMatrix", mvp  );

  shader.fs.setUniform( "diffuse",    obj.material().diffuse );
  shader.fs.setUniform( "projMatrix", scene.camera().projective()  );

  shader.fs.setUniform( "matColor", obj.material().color.data(), 4 );
  }

void MainWindow::getLightDir(float d[], const Matrix4x4& m ) {
  float ld[4], l = 0;
  m.project( -1,-2,-3,0,
             ld[0], ld[1], ld[2], ld[3]);
  for( int i=0; i<3; ++i ){
    l += ld[i]*ld[i];
    }

  l = sqrt(l);
  for( int i=0; i<3; ++i )
    d[i] = ld[i]/l;
  }

void MainWindow::setShadowConstants(ProgramObject &shader) {
  float d[3];
  Matrix4x4 m;
  m.identity();
  getLightDir( d, m );

  float z = 0.004*sqrt(zoom/0.01);
  for( int i=0; i<3; ++i )
    d[i] *= z;

  Matrix4x4 mx = {
      z,    0,  d[0],   0,
      0,    z,  d[1],   0,
      0,    0, -d[2], 1,
      0,    0,     0,   1
    };

  shader.vs.setUniform( "shadowMatrix", mx );
  }

void MainWindow::resizeEvent(Tempest::SizeEvent &) {
  device.reset();
  updateCamera();
  }

void MainWindow::mouseDownEvent(MouseEvent &e){
  mpos = e.pos();
  }

void MainWindow::mouseDragEvent(MouseEvent &e) {
  spin += (e.pos() - mpos);
  mpos = e.pos();

  updateCamera();
  }

void MainWindow::mouseWheelEvent(MouseEvent &e) {
  if( e.delta>0 )
    zoom *= 1.1; else
    zoom /= 1.1;

  updateCamera();
  }
