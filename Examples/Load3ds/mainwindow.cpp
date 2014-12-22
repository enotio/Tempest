#include "mainwindow.h"

#include <Tempest/Log>
#include <Tempest/RenderState>
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
    shHolder  ( device ){
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

    if( id==12 ){
      std::vector<Vertex> v = vert;
      for( size_t i=0; i<v.size(); ++i ){
        v[i].tu = v[i].x*0.01;
        v[i].tv = v[i].y*0.01;
        }

      vbo     = vboHolder.load( v     );
      ibo     = iboHolder.load( index );
      } else {
      vbo     = vboHolder.load( vert  );
      ibo     = iboHolder.load( index );
      }

    Model<Vertex> md;
    md.load( vbo, ibo, vdecl );

    SceneObject obj(scene);
    Material mat;
    mat.diffuse = texture;
    mat.color   = cl[ (id+1)%7 ];

    mat.shaderType = Material::MonoColor;
    if( id%5==1 )
      mat.shaderType = Material::Transparent;

    if( id==5 )
      mat.shaderType = Material::Emission;
    if( id==9 ){
      mat.shaderType = Material::Emission;
      mat.color = Color(1);
      }

    if( id==12 ){ // floor
      mat.color   = Color(0.65);
      mat.diffuse = texture;
      mat.shaderType = Material::Main;
      }

    obj.setMaterial( mat );
    obj.setModel(md);
    objects.push_back( obj );
    ++id;
    });

  loadShader( shader,  "shader/main",
              {
                "use_shadow_map",
                "use_rsm"
              } );  
  loadShader( gbuf,  "shader/main",
              {
                "gbuffer",
                "shadow_pass"
              } );

  loadShader( refract, "shader/main",
              {
                "refractions",
                "emissionv 0.85",
                "no_texture"
              });
  loadShader( mono, "shader/main",
              {
                "no_texture",
                "use_shadow_map",
                "use_rsm"
              });
  loadShader( emission, "shader/main",
              {
                "emissionv 1.0"
              });
  loadShader( gemission, "shader/main",
              {
                "emissionv 1.0",
                "gbuffer",
                "shadow_pass"
              });
  loadShader( shadow, "shader/main",
              {
                "shadow_pass",
                "shadow_out"
              });
  loadShader( shadow_refract, "shader/main",
              {
                "shadow_pass",
                "refractions",
                "emissionv 1.0"
              });
  umaterial.add("shadowMatrix",Decl::Matrix4x4)
           .add("modelView",   Decl::Matrix4x4)
           .add("mvpMatrix",   Decl::Matrix4x4)
           .add("projMatrix",  Decl::Matrix4x4)
           .add("diffuse",     Decl::Texture2d)
           .add("screen",      Decl::Texture2d)
           .add("matColor",    Decl::float4   );

  ulighting.add("lightDir",  Decl::float3)
           .add("shadow",    Decl::Texture2d)
           .add("invMvp",    Decl::Matrix4x4)
           .add("rsmColor",  Decl::Texture2d)
           .add("rsmNormal", Decl::Texture2d);

  loadShader( gauss,   "shader/post/gaus" );
  ugauss.add("image",Decl::Texture2d)
        .add("dir",  Decl::float2   );
  loadShader( bright,  "shader/post/bright" );
  ubright.add("image",Decl::Texture2d);

  loadShader( combine, "shader/post/combine" );
  ucombine.add("image", Decl::Texture2d)
          .add("bloom0",Decl::Texture2d)
          .add("bloom1",Decl::Texture2d)
          .add("bloom2",Decl::Texture2d)
          .add("bloom3",Decl::Texture2d);

  loadShader( blt, "shader/blt" );
  ublt.add("texture", Decl::Texture2d);

  updateCamera();
  }

MainWindow::~MainWindow() {
  aiDetachAllLogStreams();
  }

void MainWindow::loadShader( ShaderProgram &shader,
                             const std::string & file ) {
  shader = shHolder.load({file+".vs.glsl",
                          file+".fs.glsl",
                          "","",""});
  if( !shader.isValid() )
    Log::e("sh:",shader.log());
  }

void MainWindow::loadShader( ShaderProgram &shader,
                             const std::string &file,
                             const std::initializer_list<const char *> &def ) {
  shader = shHolder.compile({shaderSource(file+".vs.glsl", def ),
                             shaderSource(file+".fs.glsl", def ),
                             "","",""});
  if( !shader.isValid() )
    Log::e("sh:",shader.log());
  }

std::string MainWindow::shaderSource(const std::string &f, const std::initializer_list<const char *> &def) {
  RFile file( f.c_str() );
  return (shaderSource(file, def));
  }

std::string MainWindow::shaderSource(RFile &f, const std::initializer_list<const char *> &def) {
  std::string s;
  s.resize( f.size() );
  f.readData( &s[0], s.size() );

  std::stringstream ss;
  for( auto i:def )
    ss <<"#define "<< i << "\n";
  ss << s;

  return ss.str();
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

void MainWindow::renderShadow( ShaderProgram& shadow,
                               Texture2d& sh,
                               Texture2d& depth,
                               bool inv,
                               int smSz ) {
  uboMaterial.shadowMatrix = shadowMatrix();

  //const int smSz = 1024;
  depth = ltexHolder.create(smSz, smSz, Texture2d::Format::Depth24 );

  sh = ltexHolder.create(smSz, smSz, Texture2d::Format::RGB10_A2);
  Texture2d::Sampler sm;
  sm.setClamping( Texture2d::ClampMode::ClampToEdge );
  sh.setSampler(sm);

  RenderState st;
  if( inv )
    st.setCullFaceMode( RenderState::CullMode::front );

  shadow.setUniform(uboMaterial,umaterial,0);
  device.setRenderState(st);

  device.beginPaint( sh, depth);
  device.clear( inv?Color(0.0):Color(0.0), 1.0 );
  for( size_t i=0; i<objects.size(); ++i ){
    SceneObject& obj = objects[i];

    if( obj.material().shaderType != Material::Transparent || inv ){
      setShaderConstants( obj, shadow );
      device.draw( obj, shadow );
      }
    }
  device.endPaint();
  }

void MainWindow::renderShadow( Texture2d* sh, int cnt,
                               Texture2d& depth,
                               int smSz ) {
  uboMaterial.shadowMatrix = shadowMatrix();

  //const int smSz = 1024;
  depth = ltexHolder.create(smSz, smSz, Texture2d::Format::Depth24 );

  for( int i=0; i<cnt; ++i ){
    sh[i] = ltexHolder.create(smSz, smSz, Texture2d::Format::RGB10_A2);
    Texture2d::Sampler sm;
    sm.setClamping( Texture2d::ClampMode::ClampToEdge );
    sh[i].setSampler(sm);
    }

  RenderState st;
  device.setRenderState(st);

  gbuf.setUniform(uboMaterial,umaterial,0);
  device.beginPaint( sh, cnt, depth);
  device.clear( Color(0.0), 1.0 );
  for( size_t i=0; i<objects.size(); ++i ){
    SceneObject& obj = objects[i];

    if( obj.material().shaderType != Material::Emission &&
        obj.material().shaderType != Material::Transparent ){
      setShaderConstants( obj, gbuf );
      device.draw( obj, gbuf );
      }
    }

  gemission.setUniform(uboMaterial,umaterial,0);
  for( size_t i=0; i<objects.size(); ++i ){
    SceneObject& obj = objects[i];

    if( obj.material().shaderType == Material::Emission ){
      setShaderConstants( obj, gemission );
      device.draw( obj, gemission );
      }
    }
  device.endPaint();
  }

void MainWindow::render() {
  if( !device.startRender() )
    return;

  Shadow sh;
  renderShadow( shadow, sh.sm, sh.depth, true, 1024 );
  //sh.sm = distorsion( sh.sm, sh.depth, shadow_refract);

  setupLigting( scene, gbuf, sh, true );
  setupLigting( scene, gemission, sh, true );
  renderShadow( sh.rsm, 3, sh.depth, 128 );

  mainPasss( sh );

  if( 0 ){
    device.beginPaint();
    device.clear( Color(0.0), 1.0 );
    uboBlt.texture = sh.rsm[0];
    blt.setUniform(uboBlt,ublt,0);
    device.drawFullScreenQuad( blt );
    device.endPaint();
    }

  device.present();
  }

Texture2d MainWindow::distorsion( const Texture2d& scene,
                                  Texture2d& depth,
                                  ShaderProgram & refract ){
  Texture2d rt2 = mkBuffer(scene.size());

  device.beginPaint(rt2, depth);
  uboBlt.texture = scene;
  blt.setUniform(uboBlt,ublt,0);
  uboMaterial.screen = scene;

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

  Texture2d rt3 = mkBuffer(scene.size());
  device.beginPaint(rt3, depth);
  uboBlt.texture = scene;
  blt.setUniform(uboBlt,ublt,0);
  uboMaterial.screen = rt2;

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

void MainWindow::mainPasss( const Shadow & sm ){
  setupLigting( scene, shader,   sm );
  setupLigting( scene, mono,     sm );
  setupLigting( scene, refract,  sm );
  setupLigting( scene, emission, sm );

  device.setRenderState( RenderState() );

  Texture2d rt    = mkBuffer(size()),
            depth = ltexHolder.create(size(), Texture2d::Format::Depth24 );

  device.beginPaint(rt, depth);
  device.clear( Tempest::Color(0), 1.0 );

  for( size_t i=0; i<objects.size(); ++i ){
    SceneObject& obj = objects[i];
    if( obj.material().shaderType == Material::MonoColor ){
      setShaderConstants( obj, mono );
      device.draw( obj, mono );
      }
    }

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

  return;
  //result without post-process
  uboBlt.texture = rt;
  blt.setUniform(uboBlt,ublt,0);
  device.beginPaint();
  device.drawFullScreenQuad( blt );
  device.endPaint();
  }

Texture2d MainWindow::brightPass( const Texture2d &tex ) {
  uboBright.image = tex;
  bright.setUniform(uboBright,ubright,0);

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
  uboBright.image = tex;
  bright.setUniform(uboBright,ubright,0);

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
  uboCombine.image = tex;
  std::copy(bloom,bloom+4,uboCombine.bloom);
  combine.setUniform(uboCombine,ucombine,0);

  device.beginPaint();
  device.setRenderState( RenderState::PostProcess );
  device.drawFullScreenQuad( combine );
  device.endPaint();
  }

Texture2d MainWindow::blur( const Texture2d &tex ) {
  uboGauss.image  = tex;
  uboGauss.dir[0] = 1.0f/tex.width();
  uboGauss.dir[1] = 0;
  gauss.setUniform(uboGauss,ugauss,0);

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

  uboGauss.image  = rt;
  uboGauss.dir[0] = 0;
  uboGauss.dir[1] = 1.0f/rt.height();
  gauss.setUniform(uboGauss,ugauss,0);

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
                               ShaderProgram &shader,
                               const Shadow & sm,
                               bool sh ) {
  if( sh ){
    const Matrix4x4 m = shadowMatrix();
    getLightDir( uboLighting.lightDir, m );
    } else {
    const Matrix4x4 m = scene.camera().view();
    getLightDir( uboLighting.lightDir, m );
    }
  uboLighting.shadow = sm.sm;
  uboLighting.invMvp = shadowMatrix();
  uboLighting.invMvp.inverse();
  uboLighting.rsmColor  = sm.rsm[0];
  uboLighting.rsmNormal = sm.rsm[1];
  shader.setUniform(uboLighting,ulighting,1);
  }

void MainWindow::setShaderConstants( SceneObject& obj,
                                     ShaderProgram& shader ) {
  uboMaterial.shadowMatrix = shadowMatrix();

  Matrix4x4 view = scene.camera().view();
  view.mul( obj.transform()  );

  Matrix4x4 mvp = scene.camera().projective();
  mvp.mul( view );

  uboMaterial.modelView  = view;
  uboMaterial.mvpMatrix  = mvp;
  uboMaterial.projMatrix = mvp;
  uboMaterial.diffuse    = obj.material().diffuse;
  uboMaterial.matColor   = obj.material().color;
  shader.setUniform(uboMaterial,umaterial,0);
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

Matrix4x4 MainWindow::shadowMatrix(){
  float dir[3];
  getLightDir( dir, Matrix4x4() );

  const Camera& c = (const Camera&)scene.camera();
  float s0 = 0.05,
        s  = 0.005,//0.05*pow( 0.25*c.zoom(), 1.0/3.0),
        h  = 0.25*s0,
        x  = -s*c.x(), y = -s*c.y(), z = h + (c.z()+1)*h;

  dir[0]*=s;
  dir[1]*=s;

  float d[4][4] = {
    { s,  0,  0,  0 },
    { 0,  s,  0,  0 },
    { dir[0], dir[1], h,  0 },
    { x,  y,  -z,  1 }
    };

  Matrix4x4 m = Matrix4x4( &d[0][0] );
  Matrix4x4 proj;

  //float k = std::min( rtSize.w, rtSize.h );
  //proj.scale( k/rtSize.w, k/rtSize.h, 1 );

  //proj.rotate( c.spinX(), 0, 0, 1 );

  proj.mul( m );

  return proj;
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
