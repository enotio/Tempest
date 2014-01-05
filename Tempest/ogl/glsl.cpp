#include "glsl.h"

#ifdef __ANDROID__
#include <EGL/egl.h>
#include <GLES/gl.h>
#include <GLES2/gl2.h>
#else
#include "glfn.h"
#include <GL/gl.h>

using namespace Tempest::GLProc;
#endif

#include <Tempest/Assert>

#include <Tempest/Device>
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

    bool operator < ( const ShProgram& sh ) const {
      return std::tie(vs, fs) < std::tie(sh.vs, sh.fs);
      }

    GLuint linked;
    };

  std::vector<ShProgram> prog;
  bool hasAnisotropic;

  //GLuint curProgram;
  ShProgram curProgram;

  char  texAttrName[14];
  void* notId;
  };

void* GLSL::context() const{
  return data->context;
  }

GLSL::GLSL( AbstractAPI::OpenGL2xDevice * dev ) {
  data = new Data();

  const char* Tc = "TexCoord";
  memset( data->texAttrName, 0, sizeof(data->texAttrName) );
  for( int i=0; Tc[i]; ++i )
    data->texAttrName[i] = Tc[i];

  data->context = dev;
  data->notId = (void*)size_t(-1);

  data->currentVS = 0;
  data->currentFS = 0;

  const char * ext = (const char*)glGetString(GL_EXTENSIONS);
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
  GLuint *prog = new GLuint(0);
  *prog = data->loadShader( GL_VERTEX_SHADER, src.data(), log );

  if( !*prog ){
    delete prog;
    return 0;
    }

  return reinterpret_cast<AbstractShadingLang::VertexShader*>(prog);
  }

void GLSL::deleteVertexShader( VertexShader* s ) const {
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

void GLSL::deleteFragmentShader( FragmentShader* s ) const{
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
      "uniform sampler2D texture;"

      "void main() {"
        "gl_FragColor = texture2D(texture, tc)*cl;"
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
      }
    }

  switch( t ) {
    case Vertex:
      return vs_src_nt;
      break;
    case Fragment:
      return fs_src_nt;
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

void GLSL::unBind( const Tempest::VertexShader& s ) const {
  data->currentVS = 0;
  data->uCash.reset();
  inputOf(s).resetID();

  //setDevice();
  }

void GLSL::unBind( const Tempest::FragmentShader& s ) const {
  //CGprogram prog = CGprogram( get(s) );
  data->currentFS = 0;
  data->uCash.reset();
  inputOf(s).resetID();

  //setDevice();
  }

void GLSL::setVertexDecl(const AbstractAPI::VertexDecl *v ) const {
  data->vdecl = v;
  }

void GLSL::enable() const {
  GLuint vertexShader = *(GLuint*)get( *data->currentVS );
  GLuint pixelShader  = *(GLuint*)get( *data->currentFS );
  GLuint program = 0;

  if( data->curProgram.vs == vertexShader &&
      data->curProgram.fs == pixelShader ){
    program = data->curProgram.linked;
    }

  //NON-Cashed
  if( program==0 ){
    Data::ShProgram tmp;
    tmp.vs = vertexShader;
    tmp.fs = pixelShader;

    auto l = std::lower_bound(data->prog.begin(), data->prog.end(), tmp);
    if( l!=data->prog.end() && l->vs==tmp.vs && l->fs==tmp.fs ){
      program = (*l).linked;
      }

    /*
    for( size_t i=0; i<data->prog.size(); ++i )
      if( data->prog[i].vs == vertexShader &&
          data->prog[i].fs == pixelShader ){
        program = data->prog[i].linked;
        }*/
    }

  //Non-Linked
  if( program==0 ){
    program = glCreateProgram();
    T_ASSERT( program );

    glAttachShader(program, vertexShader);
    glAttachShader(program, pixelShader);

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
        = *(const Tempest::VertexDeclaration::Declarator*)data->vdecl;

    int usrAttr = -1;
    for( int i=0; i<vd.size(); ++i ){
      if( vd[i].usage!=Usage::TexCoord && vd[i].attrName.size()==0 ){
        glBindAttribLocation( program, i, uType[vd[i].usage] );
        } else
      if( vd[i].usage==Usage::TexCoord ){
        data->texAttrName[8] = '0';
        data->texAttrName[9] = 0;

        int id = vd[i].index;
        if( id ){
          int idx = 0;
          while( id>0 ){
            ++idx;
            id/=10;
            }

          id = vd[i].index;
          for( int r=0; id>0; ++r ){
            data->texAttrName[7+idx-r] = '0'+id%10;
            id/=10;
            }

          data->texAttrName[8+idx] = 0;
          }

        glBindAttribLocation( program, vd.size()+vd[i].index, data->texAttrName );

        if( vd[i].index==0 )
          glBindAttribLocation( program, vd.size()+vd[i].index, "TexCoord" );
        } else {
        glBindAttribLocation( program,
                              vd.size()+vd.texCoordCount()+usrAttr,
                              vd[i].attrName.c_str() );
        ++usrAttr;
        }
      }

    //glBindAttribLocation( program, 0, "vPosition" );
    glLinkProgram (program);

    GLint linkStatus = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);

    if (linkStatus != GL_TRUE) {
      GLint bufLength = 0;
      glGetProgramiv(program, GL_INFO_LOG_LENGTH, &bufLength);

      if (bufLength) {
        char* buf = new char[bufLength];
        if (buf) {
          glGetProgramInfoLog(program, bufLength, NULL, buf);
          std::cerr << buf;
          delete[] (buf);
          T_ASSERT(0);
          }
        }

      glDeleteProgram(program);
      program = 0;
      }

    Data::ShProgram s;
    s.vs     = vertexShader;
    s.fs     = pixelShader;
    s.linked = program;

    auto l = std::lower_bound(data->prog.begin(), data->prog.end(), s);
    data->prog.insert(l, s);

    //data->prog.push_back( s );
    }

  if( program != data->curProgram.linked ){
    glUseProgram( program );
    data->curProgram.linked = program;
    data->curProgram.vs = vertexShader;
    data->curProgram.fs = pixelShader;

    data->uCash.reset();
    }

  setUniforms( program, inputOf( *data->currentFS ), true  );
  setUniforms( program, inputOf( *data->currentVS ), false );
  }

void GLSL::setUniforms( unsigned int program,
                        const ShaderInput &in,
                        bool textures ) const {
  if( textures ){
    for( size_t i=0; i<in.tex3d.names.size(); ++i ){
      setUniform( program,
                  *in.tex3d.values[i],
                  in.tex3d.names[i].data(), i,
                  in.tex3d.id[i] );
      }

    for( size_t i=0; i<in.tex.names.size(); ++i ){
      setUniform( program,
                  *in.tex.values[i],
                  in.tex.names[i].data(), i,
                  in.tex.id[i] );
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

void GLSL::setUniform( unsigned int s,
                       const Matrix4x4& m,
                       const char *name,
                       void *&id  ) const{
  GLint prm = -1;
  if( id==data->notId ){
    GLuint    prog = s;
    prm = data->location( prog, name );
    if( prm==-1 )
      return;
    id = (void*)size_t(prm);
    } else {
    prm = (GLint)id;
    }

  if( !data->uCash.fetch(prm, m) ){
    float r[16] = {};
    std::copy( m.data(), m.data()+16, r );
    glUniformMatrix4fv( prm, 1, false, r );
    //glUniform4fv( prm, 4, r );
    }
  }

void GLSL::setUniform( unsigned int s,
                       const float v[],
                       int l,
                       const char *name,
                       void *&id ) const{
  GLint prm = -1;
  if( id==data->notId ){
    GLuint    prog = s;
    prm = data->location( prog, name );
    if( prm==-1 )
      return;
    id = (void*)size_t(prm);
    } else {
    prm = (GLint)id;
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

void GLSL::setUniform( unsigned int sh,
                       const Texture2d& u,
                       const char *name,
                       int slot,
                       void *&id ) const{
  GLint prm = -1;
  if( id==data->notId ){
    GLuint    prog = sh;
    prm = data->location( prog, name );
    if( prm==-1 )
      return;
    id = (void*)size_t(prm);
    } else {
    prm = (GLint)id;
    }

  if( !data->uCash.fetch(prm, u.handle()) ){
    Detail::GLTexture* tx = (Detail::GLTexture*)get(u);

    if( tx && tx->id ){
      data->setupSampler( GL_TEXTURE_2D, prm, slot, tx, u.sampler() );
      }
    }
  }

void GLSL::setUniform( unsigned int sh,
                       const Texture3d &u,
                       const char *name,
                       int slot, void *&id) const {
  GLint prm = -1;
  if( id==data->notId ){
    GLuint    prog = sh;
    prm = data->location( prog, name );
    if( prm==-1 )
      return;
    id = (void*)size_t(prm);
    } else {
    prm = (GLint)id;
    }

  if( !data->uCash.fetch(prm, u.handle()) ){
    Detail::GLTexture* tx = (Detail::GLTexture*)get(u);

    if( tx && tx->id ){
#ifndef __ANDROID__
      data->setupSampler( GL_TEXTURE_3D, prm, slot, tx, u.sampler() );
#endif
      }
    }
  }

const char *GLSL::opt(const char *t, const char *f, bool v) {
  if( v )
    return t;

  return f;
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
    GL_CLAMP,
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

  if( isPot ){
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
