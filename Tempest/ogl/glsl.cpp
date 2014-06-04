#include "glsl.h"

#ifdef __ANDROID__
#include <EGL/egl.h>
#include <GLES/gl.h>
#include <GLES2/gl2.h>
#else
#include "glfn.h"
#include <GL/gl.h>
#include "glcorearb.h"

using namespace Tempest::GLProc;
#endif

#include <Tempest/Assert>

#ifdef __ANDROID__
#include <Tempest/Device>
#else
#include <Tempest/DeviceSM5>
#endif

#include <Tempest/Matrix4x4>
#include <Tempest/Texture2d>

#include "shading/uniformcash.h"
#include <Tempest/SystemAPI>
#include "gltypes.h"

#include <iostream>

#include <cstring>
#include <tuple>
#include <algorithm>

#ifdef __ANDROID__
#define GL_TEXTURE_MAX_ANISOTROPY_EXT     0x84FE
#define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT 0x84FF
#endif

using namespace Tempest;

struct GLSL::Data{
  AbstractAPI::OpenGL2xDevice * context;
  float maxAnisotropy;

  static void dbgOut( GLuint /*context*/ ){

    }

  void dbg(){
    //dbg(false, context);
    }

  static void dbg( bool /*ok*/, GLuint /*context*/ ){

    }

  const Tempest::VertexShader*   currentVS;
  const Tempest::FragmentShader* currentFS;
  const Tempest::TessShader*     currentTS = 0;
  const Tempest::EvalShader*     currentES = 0;

  Detail::UniformCash<GLuint> uCash;
  const AbstractAPI::VertexDecl* vdecl;

  std::string readFile( const char* f ){
    return std::move( SystemAPI::loadText(f).data() );
    }

  std::string readFile( const char16_t* f ){
    return std::move( SystemAPI::loadText(f).data() );
    }

  GLuint loadShader( GLenum shaderType,
                     const char* pSource,
                     std::string & log ) {
    GLuint shader = glCreateShader(shaderType);

    if (shader) {
      glShaderSource(shader, 1, &pSource, NULL);
      glCompileShader(shader);
      GLint compiled = 0;
      glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

      if (!compiled) {
        GLint infoLen = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);

        log.resize(infoLen);
        if (infoLen) {
          glGetShaderInfoLog(shader, infoLen, NULL, &log[0]);
          glDeleteShader(shader);
          shader = 0;
          }

        return 0;
        }
      }
    return shader;
    }

  GLint location( GLuint prog, const char* name ){
    return glGetUniformLocation( prog, name );
    }

  template< class Sampler >
  void setupSampler( GLenum texClass,
                     GLint prm, int slot,
                     Detail::GLTexture *tx,
                     const Sampler &s );

  Texture3d::ClampMode::Type clampW( const Texture2d::Sampler& ){
    return Texture2d::ClampMode::Repeat;
    }

  Texture3d::ClampMode::Type clampW( const Texture3d::Sampler& s ){
    return s.wClamp;
    }

  bool isAnisotropic( const Texture2d::Sampler& s ){
    return s.anisotropic;
    }

  bool isAnisotropic( const Texture3d::Sampler&  ){
    return false;
    }

  struct ShProgram{
    GLuint vs;
    GLuint fs;
#ifndef __ANDROID__
    GLuint ts = 0;
    GLuint es = 0;
#endif

    const void *decl;

    bool operator < ( const ShProgram& sh ) const {
#ifndef __ANDROID__
      return std::tie(vs, fs, ts, es, decl) < std::tie(sh.vs, sh.fs, sh.ts, sh.es, sh.decl);
#else
      return std::tie(vs, fs, decl) < std::tie(sh.vs, sh.fs, sh.decl);
#endif
      }

    bool operator == ( const ShProgram& sh ) const {
#ifndef __ANDROID__
      return std::tie(vs, fs, ts, es, decl) == std::tie(sh.vs, sh.fs, sh.ts, sh.es, sh.decl);
#else
      return std::tie(vs, fs, decl) == std::tie(sh.vs, sh.fs, sh.decl);
#endif
      }

    GLuint linked;
    };

  std::vector<ShProgram> prog;
  bool hasAnisotropic;

  //GLuint curProgram;
  ShProgram curProgram;

  char  texAttrName[14];
  int32_t notId;

  void setupVAttr( GLuint program ){
    static const char* uType[] = {
      "Position",
      "BlendWeight",   // 1
      "BlendIndices",  // 2
      "Normal",        // 3
      "PSize",         // 4
      "TexCoord",      // 5
      "Tangent",       // 6
      "BiNormal",      // 7
      "TessFactor",    // 8
      "PositionT",     // 9
      "Color",         // 10
      "Fog",           // 11
      "Depth",         // 12
      "Sample",        // 13
      ""
      };

    const Tempest::VertexDeclaration::Declarator& vd
        = *(const Tempest::VertexDeclaration::Declarator*)vdecl;

    int usrAttr = -1;
    for( int i=0; i<vd.size(); ++i ){
      if( vd[i].usage!=Usage::TexCoord && vd[i].attrName.size()==0 ){
        glBindAttribLocation( program, i, uType[vd[i].usage] );
        } else
      if( vd[i].usage==Usage::TexCoord ){
        texAttrName[8] = '0';
        texAttrName[9] = 0;

        int id = vd[i].index;
        if( id ){
          int idx = 0;
          while( id>0 ){
            ++idx;
            id/=10;
            }

          id = vd[i].index;
          for( int r=0; id>0; ++r ){
            texAttrName[7+idx-r] = '0'+id%10;
            id/=10;
            }

          texAttrName[8+idx] = 0;
          }

        glBindAttribLocation( program, vd.size()+vd[i].index, texAttrName );

        if( vd[i].index==0 )
          glBindAttribLocation( program, vd.size()+vd[i].index, "TexCoord" );
        } else {
        glBindAttribLocation( program,
                              vd.size()+vd.texCoordCount()+usrAttr,
                              vd[i].attrName.c_str() );
        ++usrAttr;
        }
      }
    }

  GLuint link( GLuint vertexShader,
               GLuint pixelShader,
               GLuint tessShader,
               GLuint evalShader,
               const AbstractAPI::VertexDecl * vdecl,
               std::string* log ) {
    (void)tessShader;
    (void)evalShader;

    if( curProgram.vs   == vertexShader &&
        curProgram.fs   == pixelShader  &&
    #ifndef __ANDROID__
        curProgram.ts   == tessShader   &&
        curProgram.es   == evalShader   &&
    #endif
        curProgram.decl == vdecl ){
      return curProgram.linked;
      }

    //NON-Cashed
    Data::ShProgram tmp;
    tmp.vs   = vertexShader;
    tmp.fs   = pixelShader;
#ifndef __ANDROID__
    tmp.ts   = tessShader;
    tmp.es   = evalShader;
#endif
    tmp.decl = vdecl;

    auto l = std::lower_bound(prog.begin(), prog.end(), tmp);
    if( l!=prog.end() && *l==tmp ){
      return (*l).linked;
      }

    //Non-Linked
    GLuint program = glCreateProgram();
    T_ASSERT( program );

    glAttachShader(program, vertexShader );
    glAttachShader(program, pixelShader  );
#ifndef __ANDROID__
    glAttachShader(program, tessShader   );
    glAttachShader(program, evalShader   );
#endif
    setupVAttr(program);
    glLinkProgram (program);

    GLint linkStatus = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);

    GLint bufLength = 0;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &bufLength);

    if( bufLength && log ) {
      log->resize( bufLength );
      glGetProgramInfoLog(program, bufLength, NULL, &(*log)[0]);
      }

    if (linkStatus != GL_TRUE) {
      glDeleteProgram(program);
      program = 0;
      }

    tmp.linked = program;
    {
    auto l = std::lower_bound(prog.begin(), prog.end(), tmp);
    prog.insert(l, tmp);
    }
    return program;
    }

  };

void* GLSL::context() const{
  return data->context;
  }

static const GLubyte* vstr( const GLubyte* v ){
  if( v )
    return v; else
    return (GLubyte*)""; //avoid Android bug
  }

GLSL::GLSL(AbstractAPI::OpenGL2xDevice * dev) {
  data = new Data();

  const char* Tc = "TexCoord";
  memset( data->texAttrName, 0, sizeof(data->texAttrName) );
  for( int i=0; Tc[i]; ++i )
    data->texAttrName[i] = Tc[i];

  data->context = dev;
  data->notId   = -1;

  data->currentVS = 0;
  data->currentFS = 0;

  const char * ext = (const char*)vstr(glGetString(GL_EXTENSIONS));

  data->hasAnisotropic =
      (strstr(ext, "GL_EXT_texture_filter_anisotropic")!=0);

//#ifndef __ANDROID__
  if( data->hasAnisotropic )
    glGetFloatv( GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &data->maxAnisotropy );
//#endif

  data->curProgram.fs = 0;
  data->curProgram.vs = 0;
  data->curProgram.linked = 0;
  }

GLSL::~GLSL(){
  delete data;
  }

void Tempest::GLSL::beginPaint() const {

  }

void Tempest::GLSL::endPaint() const {
  glUseProgram( 0 );

  data->currentVS = 0;
  data->currentFS = 0;
  data->uCash.reset();

  data->curProgram.linked = 0;
  //cgGLDisableProfile( data->pixelProfile );
  }

void GLSL::setDevice() const {
  }

AbstractShadingLang::VertexShader*
  GLSL::createVertexShaderFromSource(const std::string &src, std::string &log) const {
  if( src.size()==0 )
    return 0;

  GLuint *prog = new GLuint(0);
  *prog = data->loadShader( GL_VERTEX_SHADER, src.data(), log );

  if( !*prog ){
    delete prog;
    return 0;
    }

  return reinterpret_cast<AbstractShadingLang::VertexShader*>(prog);
  }

void GLSL::deleteShader( VertexShader* s ) const {
  //setNullDevice();
  GLuint* prog = (GLuint*)(s);

  if( glIsShader(*prog) )
    glDeleteShader( *prog );

  size_t nsz = 0;
  for( size_t i=0; i<data->prog.size(); ++i ){
    data->prog[nsz] = data->prog[i];

    if( data->prog[i].vs == *prog ){
      if( glIsProgram(data->prog[i].linked) )
        glDeleteProgram( data->prog[i].linked );
      } else {
      ++nsz;
      }
    }

  data->prog.resize(nsz);
  delete prog;
  }

AbstractShadingLang::FragmentShader *
  GLSL::createFragmentShaderFromSource(const std::string &src, std::string &log) const {
  GLuint *prog = new GLuint(0);
  *prog = data->loadShader( GL_FRAGMENT_SHADER, src.data(), log );

  if( !*prog ){
    delete prog;
    return 0;
    }

  return reinterpret_cast<AbstractShadingLang::FragmentShader*>(prog);
  }

void GLSL::deleteShader( FragmentShader *s ) const{
  GLuint* prog = (GLuint*)(s);

  if( glIsShader(*prog) )
    glDeleteShader( *prog );

  size_t nsz = 0;
  for( size_t i=0; i<data->prog.size(); ++i ){
    data->prog[nsz] = data->prog[i];
    if( data->prog[i].fs == *prog ){
      if( glIsProgram(data->prog[i].linked) )
        glDeleteProgram( data->prog[i].linked );
      } else {
      ++nsz;
      }
    }

  data->prog.resize(nsz);
  delete prog;
  }

GraphicsSubsystem::TessShader *GLSL::createTessShaderFromSource( const std::string &src,
                                                                 std::string &log ) const {
#ifdef __ANDROID__
  (void)src;
  (void)log;
  return 0;
#else
  if( src.size()==0 )
    return 0;

  GLuint *prog = new GLuint(0);
  *prog = data->loadShader( GL_TESS_CONTROL_SHADER, src.data(), log );

  if( !*prog ){
    delete prog;
    return 0;
    }

  return reinterpret_cast<AbstractShadingLang::TessShader*>(prog);
#endif
  }

void GLSL::deleteShader( TessShader *s ) const{
#ifndef __ANDROID__
  GLuint* prog = (GLuint*)(s);

  if( glIsShader(*prog) )
    glDeleteShader( *prog );

  size_t nsz = 0;
  for( size_t i=0; i<data->prog.size(); ++i ){
    data->prog[nsz] = data->prog[i];
    if( data->prog[i].ts == *prog ){
      if( glIsProgram(data->prog[i].linked) )
        glDeleteProgram( data->prog[i].linked );
      } else {
      ++nsz;
      }
    }

  data->prog.resize(nsz);
  delete prog;
#endif
  }

GraphicsSubsystem::EvalShader *GLSL::createEvalShaderFromSource( const std::string &src,
                                                                 std::string &log ) const {
#ifdef __ANDROID__
  (void)src;
  (void)log;
  return 0;
#else
  if( src.size()==0 )
    return 0;

  GLuint *prog = new GLuint(0);
  *prog = data->loadShader( GL_TESS_EVALUATION_SHADER, src.data(), log );

  if( !*prog ){
    delete prog;
    return 0;
    }

  return reinterpret_cast<AbstractShadingLang::EvalShader*>(prog);
#endif
  }

void GLSL::deleteShader( EvalShader *s ) const{
#ifndef __ANDROID__
  GLuint* prog = (GLuint*)(s);

  if( glIsShader(*prog) )
    glDeleteShader( *prog );

  size_t nsz = 0;
  for( size_t i=0; i<data->prog.size(); ++i ){
    data->prog[nsz] = data->prog[i];
    if( data->prog[i].es == *prog ){
      if( glIsProgram(data->prog[i].linked) )
        glDeleteProgram( data->prog[i].linked );
      } else {
      ++nsz;
      }
    }

  data->prog.resize(nsz);
  delete prog;
#endif
  }

std::string GLSL::surfaceShader( AbstractShadingLang::ShaderType t,
                                 const AbstractShadingLang::UiShaderOpt &vopt,
                                 bool &hasHalfpixOffset ) const {
  hasHalfpixOffset = false;

  const std::string vs_src = std::string(
      "attribute vec2 Position;"
      "attribute vec2 TexCoord;"
      "attribute vec4 TexCoord1;"
) +
  opt("varying vec2 tc;", "", vopt.texcoord != Decl::count && vopt.hasTexture ) +
  opt("varying vec4 cl;", "", vopt.color    != Decl::count ) +

      "void main() {" +
    opt("tc = TexCoord;",  "", vopt.texcoord!=Decl::count && vopt.hasTexture ) +
    opt("cl = TexCoord1;", "", vopt.color   !=Decl::count ) +
        "gl_Position = vec4(Position, 0.0, 1.0);"
        "}";

  static const std::string fs_src =
#ifdef __ANDROID__
      "precision mediump float;"
#endif
      "varying vec2 tc;"
      "varying vec4 cl;"
      "uniform sampler2D brush_texture;"

      "void main() {"
        "gl_FragColor = texture2D(brush_texture, tc)*cl;"
        "}";

  static const std::string vs_src_nt =
      "attribute vec2 Position;"
      "attribute vec2 TexCoord;"
      "attribute vec4 TexCoord1;"

      "varying vec4 cl;"

      "void main() {"
        "cl = TexCoord1;"
        "gl_Position = vec4(Position, 0.0, 1.0);"
        "}";

  static const std::string fs_src_nt =
#ifdef __ANDROID__
      "precision mediump float;"
#endif
      "varying vec4 cl;"

      "void main() {"
        "gl_FragColor = cl;"
        "}";

  if( vopt.hasTexture ){
    switch( t ) {
      case Vertex:
        return vs_src;
        break;
      case Fragment:
        return fs_src;
        break;

      default:
        return "";
        break;
      }
    }

  switch( t ) {
    case Vertex:
      return vs_src_nt;
      break;
    case Fragment:
      return fs_src_nt;
      break;
    default:
      return "";
      break;
    }

  return fs_src;
  }

void GLSL::bind( const Tempest::VertexShader& s ) const {
  if( data->currentVS != &s ){
    data->currentVS = &s;
    inputOf(s).resetID();
    }
  }

void GLSL::bind( const Tempest::FragmentShader& s ) const {
  if( data->currentFS != &s ){
    data->currentFS = &s;
    inputOf(s).resetID();
    }
  }

void GLSL::bind( const Tempest::TessShader& s ) const {
  (void)s;
#ifndef __ANDROID__
  if( data->currentTS != &s ){
    data->currentTS = &s;
    inputOf(s).resetID();
    }
#endif
  }

void GLSL::bind( const Tempest::EvalShader& s ) const {
  (void)s;
#ifndef __ANDROID__
  if( data->currentES != &s ){
    data->currentES = &s;
    inputOf(s).resetID();
    }
#endif
  }

bool GLSL::link( const Tempest::VertexShader &vs,
                 const Tempest::FragmentShader &fs,
                 const AbstractAPI::VertexDecl *decl,
                 std::string &log ) const {
  GLuint* vertexShader = (GLuint*)get( vs );
  GLuint* pixelShader  = (GLuint*)get( fs );

  if( vertexShader==0 ){
    log = "Vertex shader not created";
    return 0;
    }

  if( pixelShader==0 ){
    log = "Fragment shader not created";
    return 0;
    }

  GLuint program = data->link(*vertexShader, *pixelShader, 0, 0, decl, &log);
  return program!=0;
  }

void GLSL::setVertexDecl(const AbstractAPI::VertexDecl *v ) const {
  data->vdecl = v;
  }

void GLSL::enable() const {
  GLuint vertexShader = *(GLuint*)get( *data->currentVS );
  GLuint pixelShader  = *(GLuint*)get( *data->currentFS );

#ifndef __ANDROID__
  GLuint tessShader = 0;
  GLuint evalShader = 0;

  if( data->currentTS )
    tessShader = *(GLuint*)get( *data->currentTS );

  if( data->currentES )
    evalShader = *(GLuint*)get( *data->currentES );

  GLuint program = data->link( vertexShader, pixelShader,
                               tessShader,   evalShader,
                               data->vdecl, 0 );
#else
  GLuint program = data->link(vertexShader, pixelShader, 0, 0, data->vdecl, 0);
#endif

  T_ASSERT( program );

  if( program != data->curProgram.linked ){
    glUseProgram( program );
    data->curProgram.linked = program;
    data->curProgram.vs     = vertexShader;
    data->curProgram.fs     = pixelShader;
#ifndef __ANDROID__
    data->curProgram.ts     = tessShader;
    data->curProgram.es     = evalShader;
#endif
    data->uCash.reset();
    }

  setUniforms( program, inputOf( *data->currentFS ), true  );
  setUniforms( program, inputOf( *data->currentVS ), false );

#ifndef __ANDROID__
  if( data->currentTS )
    setUniforms( program, inputOf( *data->currentTS ), true );

  if( data->currentES )
    setUniforms( program, inputOf( *data->currentES ), true );
#endif
  }

void GLSL::disable() const {
#ifndef __ANDROID__
  data->currentTS = 0;
  data->currentES = 0;
#endif
  }

void GLSL::setUniforms( unsigned int program,
                        const ShaderInput &in,
                        bool textures ) const {
  if( textures ){
    int texSlot = 0;
    for( size_t i=0; i<in.tex3d.names.size(); ++i ){
      setUniform( program,
                  *in.tex3d.values[i],
                  in.tex3d.names[i].data(), texSlot,
                  in.tex3d.id[i] );
      ++texSlot;
      }

    for( size_t i=0; i<in.tex.names.size(); ++i ){
      setUniform( program,
                  *in.tex.values[i],
                  in.tex.names[i].data(), texSlot,
                  in.tex.id[i] );
      ++texSlot;
      }
    }

  for( size_t i=0; i<in.mat.names.size(); ++i ){
    setUniform( program,
                in.mat.values[i],
                in.mat.names[i].data(),
                in.mat.id[i] );
    }

  setUniforms( program, in.v1, 1 );
  setUniforms( program, in.v2, 2 );
  setUniforms( program, in.v3, 3 );
  setUniforms( program, in.v4, 4 );
  }

template< class T >
void GLSL::setUniforms( unsigned int s, const T & vN, int c ) const{
  for( size_t i=0; i<vN.names.size(); ++i ){
    setUniform( s, vN.values[i].v, c, vN.names[i].data(), vN.id[i] );
    }
  }

void GLSL::setUniform(unsigned int s,
                       const Matrix4x4& m,
                       const char *name,
                       int32_t &id  ) const{
  GLint prm = -1;
  if( id==data->notId ){
    GLuint    prog = s;
    prm = data->location( prog, name );
    if( prm==-1 )
      return;
    id  = prm;
    } else {
    prm = id;
    }

  if( !data->uCash.fetch(prm, m) ){
    float r[16] = {};
    std::copy( m.data(), m.data()+16, r );
    glUniformMatrix4fv( prm, 1, false, r );
    //glUniform4fv( prm, 4, r );
    }
  }

void GLSL::setUniform(unsigned int s,
                       const float v[],
                       int l,
                       const char *name,
                       int32_t &id ) const{
  GLint prm = -1;
  if( id==data->notId ){
    GLuint    prog = s;
    prm = data->location( prog, name );
    if( prm==-1 )
      return;
    id  = prm;
    } else {
    prm = id;
    }

  if( !data->uCash.fetch(prm, v, l) ){
    //Data::dbg(prm, data->context);

    switch(l){
      case 1: glUniform1fv( prm, 1, v ); break;
      case 2: glUniform2fv( prm, 1, v ); break;
      case 3: glUniform3fv( prm, 1, v ); break;
      case 4: glUniform4fv( prm, 1, v ); break;
      }
    }
  }

void GLSL::setUniform(unsigned int sh,
                       const Texture2d& u,
                       const char *name,
                       int slot,
                       int32_t &id ) const{
  GLint prm = -1;
  if( id==data->notId ){
    GLuint    prog = sh;
    prm = data->location( prog, name );
    if( prm==-1 )
      return;
    id  = prm;
    } else {
    prm = id;
    }

  if( !data->uCash.fetch(prm, u.handle()) ){
    Detail::GLTexture* tx = (Detail::GLTexture*)get(u);

    if( tx && tx->id ){
      data->setupSampler( GL_TEXTURE_2D, prm, slot, tx, u.sampler() );
      }
    }
  }

void GLSL::setUniform(unsigned int sh,
                       const Texture3d &u,
                       const char *name,
                       int slot,
                       int32_t &id ) const {
  GLint prm = -1;
  if( id==data->notId ){
    GLuint    prog = sh;
    prm = data->location( prog, name );
    if( prm==-1 )
      return;
    id  = prm;
    } else {
    prm = id;
    }

  if( !data->uCash.fetch(prm, u.handle()) ){
    Detail::GLTexture* tx = (Detail::GLTexture*)get(u);

    if( tx && tx->id ){
#ifndef __ANDROID__
      data->setupSampler( GL_TEXTURE_3D, prm, slot, tx, u.sampler() );
#else
      (void)slot;
#endif
      }
    }
  }

const char *GLSL::opt(const char *t, const char *f, bool v) {
  if( v )
    return t;

  return f;
  }

void GLSL::event(const GraphicsSubsystem::DeleteEvent &e) {
  size_t nsz = 0;
  for( size_t i=0; i<data->prog.size(); ++i ){
    data->prog[nsz] = data->prog[i];

    if( data->prog[i].decl == e.declaration ){
      if( glIsProgram(data->prog[i].linked) )
        glDeleteProgram( data->prog[i].linked );
      } else {
      ++nsz;
      }
    }

  data->prog.resize( nsz );
  }

template< class Sampler >
void GLSL::Data::setupSampler( GLenum texClass,
                               GLint prm,
                               int slot,
                               Detail::GLTexture* tx,
                               const Sampler & s ) {
  static const GLenum magFilter[] = {
    GL_NEAREST,
    GL_LINEAR,
    GL_NEAREST
    };

  static const GLenum filter[3][3] = {
    {GL_NEAREST, GL_NEAREST_MIPMAP_LINEAR, GL_NEAREST },
    {GL_LINEAR,  GL_LINEAR_MIPMAP_LINEAR,  GL_LINEAR },
    {GL_NEAREST, GL_NEAREST, GL_NEAREST}
    };

  static const GLenum clamp[6] = {
#ifndef __ANDROID__
    GL_CLAMP_TO_EDGE,
    GL_CLAMP_TO_BORDER,
#else
    GL_CLAMP_TO_EDGE,
    GL_CLAMP_TO_EDGE,
#endif
    GL_CLAMP_TO_EDGE,
    GL_MIRRORED_REPEAT,
    GL_REPEAT,
    GL_REPEAT
    };

  bool isPot = ((tx->w &(tx->w-1))==0) &&
               ((tx->h &(tx->h-1))==0) &&
               ( tx->z==0 || (tx->z &(tx->z-1))==0);

  glActiveTexture( GL_TEXTURE0 + slot );
  glBindTexture( texClass, tx->id );

  bool setupClampAnyway = true;
#ifdef __ANDROID__
   setupClampAnyway = false;
#endif
  if( isPot || setupClampAnyway ){
    if( tx->clampU!=clamp[ s.uClamp ] ){
      tx->clampU = clamp[ s.uClamp ];
      glTexParameteri( texClass, GL_TEXTURE_WRAP_S, tx->clampU );
      }
    if( tx->clampV!=clamp[ s.vClamp ] ){
      tx->clampV = clamp[ s.vClamp ];
      glTexParameteri( texClass, GL_TEXTURE_WRAP_T, tx->clampV );
      }

#ifndef __ANDROID__
    if( tx->z>0 ){
      if( tx->clampW!=clamp[ clampW(s) ] ){
        tx->clampW = clamp[ clampW(s) ];
        glTexParameteri( texClass, GL_TEXTURE_WRAP_R, tx->clampW );
        }
      }
#endif

    if( tx->mips ){
      if( tx->min!=filter[ s.minFilter ][s.mipFilter] ){
        glTexParameteri( texClass,
                         GL_TEXTURE_MIN_FILTER,
                         filter[ s.minFilter ][s.mipFilter] );
        tx->min = filter[ s.minFilter ][s.mipFilter];
        }
      } else {
      if( tx->min!=filter[ s.minFilter ][0] ){
        glTexParameteri( texClass,
                         GL_TEXTURE_MIN_FILTER,
                         filter[ s.minFilter ][0] );
        tx->min = filter[ s.minFilter ][0];
        }
      }

    if( tx->mag!=magFilter[ s.magFilter ] ){
      glTexParameteri( texClass,
                       GL_TEXTURE_MAG_FILTER,
                       magFilter[ s.magFilter ] );
      tx->mag = magFilter[ s.magFilter ];
      }

//#ifndef __ANDROID__
    if( hasAnisotropic ){
      if( isAnisotropic(s) && isPot && tx->mips ){
        if( tx->anisotropyLevel != maxAnisotropy ){
          glTexParameterf( texClass,
                           GL_TEXTURE_MAX_ANISOTROPY_EXT,
                           maxAnisotropy );
          tx->anisotropyLevel = maxAnisotropy;
          }
        } else {
        if( tx->anisotropyLevel!=1 ){
          glTexParameterf( texClass,
                           GL_TEXTURE_MAX_ANISOTROPY_EXT,
                           1 );
          tx->anisotropyLevel = 1;
          }
        }
      }
    }
//#endif

  glUniform1iv( prm, 1, &slot );
  glActiveTexture( GL_TEXTURE0 );
  }
