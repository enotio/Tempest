#include "glsl.h"

#include <Tempest/Platform>

#ifdef __WINDOWS__
#include <windows.h>
#endif
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
#include <Tempest/Device>
#include <Tempest/Matrix4x4>
#include <Tempest/Texture2d>
#include <Tempest/Texture3d>

#include <Tempest/SystemAPI>
#include "gltypes.h"
#include "utils/sortedvec.h"

#include <iostream>

#include <cstring>
#include <tuple>
#include <algorithm>

#ifdef __ANDROID__
#define GL_TEXTURE_MAX_ANISOTROPY_EXT     0x84FE
#define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT 0x84FF
#endif

#ifndef GL_TESS_CONTROL_SHADER
#define GL_TESS_CONTROL_SHADER 0x00008e88
#endif

#ifndef GL_TESS_EVALUATION_SHADER
#define GL_TESS_EVALUATION_SHADER 0x00008e87
#endif

#ifndef GL_TEXTURE_3D
#define GL_TEXTURE_3D 0x806F
#endif

using namespace Tempest;

struct GLSL::Data{
  AbstractAPI::OpenGL2xDevice * context;
  float maxAnisotropy;

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
    GLuint gs = 0;
    GLuint ts = 0;
    GLuint es = 0;

    ~ShProgram(){
      if(vs)
        glDeleteShader(vs);
      if(fs)
        glDeleteShader(fs);
#ifndef __ANDROID__
      if(gs)
        glDeleteShader(gs);
      if(ts)
        glDeleteShader(ts);
      if(es)
        glDeleteShader(es);
#endif
      }

    AbstractShadingLang::UBO ubo;
    //const void *vdecl;
    //GLuint linked = 0;
    };

  struct LinkedProg {
    ShProgram* shader = 0;
    GLuint linked     = 0;
    const AbstractAPI::VertexDecl* vdecl;

    bool operator < ( const LinkedProg& sh ) const {
      return std::tie(shader, vdecl) < std::tie(sh.shader, sh.vdecl);
      }

    bool operator == ( const LinkedProg& sh ) const {
      return std::tie(shader, vdecl) == std::tie(sh.shader, sh.vdecl);
      }
    };

  SortedVec<LinkedProg> prog;
  bool hasAnisotropic;

  ShProgram*  curProgram;
  LinkedProg  curLinked;
  GLuint      bindedProg;
  std::shared_ptr<std::vector<AbstractShadingLang::UBO>> curUbo;

  void setupId( GLuint prog ){
    for( AbstractShadingLang::UBO& u:*curUbo )
      setupId(prog,u);
    }

  void setupId(GLuint prog, AbstractShadingLang::UBO& ux){
    const char*  name = ux.names.data();
    uintptr_t*    id  = ux.id.data();

    for( int i=ux.desc.size(); i>0; --i ){
      *id = location( prog, name ); // int to uint convertion
      name += strlen(name)+1;
      ++id;
      }
    }

  char  texAttrName[14];
  int32_t notId;

  void setupVAttr( const AbstractAPI::VertexDecl * vdecl, GLuint program ){
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

  GLuint link( Data::ShProgram& p,
               const AbstractAPI::VertexDecl * vdecl,
               std::string* log ) {
    if( curLinked.linked &&
        curLinked.shader == &p &&
        curLinked.vdecl  == vdecl ){
      return curLinked.linked;
      }

    //NON-Cashed
    Data::LinkedProg tmp;
    tmp.shader = &p;
    tmp.vdecl  = vdecl;

    auto l = prog.find(tmp);
    if( l!=prog.end() && *l==tmp ){
      return (*l).linked;
      }

    //Non-Linked
    GLuint program = glCreateProgram();
    T_ASSERT( program!=0 );

    glAttachShader(program, p.vs );
    glAttachShader(program, p.fs  );
#ifndef __ANDROID__
    if( p.ts ){
      glAttachShader(program, p.ts   );
      }
    if( p.es ){
      glAttachShader(program, p.es   );
      }
#endif

    setupVAttr(vdecl, program);
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
    prog.insert(tmp);
    return program;
    }

  void makeCurrent(Data::ShProgram* prog){
    if( !prog )
      return;

    if(!curLinked.linked){
      curLinked.linked = link( *prog, curLinked.vdecl, 0 );
      T_ASSERT( curLinked.linked!=0 );
      }

    if( bindedProg!=curLinked.linked ){
      glUseProgram(curLinked.linked);
      bindedProg = curLinked.linked;
      setupId(curLinked.linked);
      }
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

  const char * ext = (const char*)vstr(glGetString(GL_EXTENSIONS));

  data->hasAnisotropic =
      (strstr(ext, "GL_EXT_texture_filter_anisotropic")!=0);

//#ifndef __ANDROID__
  if( data->hasAnisotropic )
    glGetFloatv( GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &data->maxAnisotropy );
//#endif
  data->curProgram = 0;
  data->bindedProg = 0;
  }

GLSL::~GLSL(){
  delete data;
  }

void Tempest::GLSL::beginPaint() const {
  }

void Tempest::GLSL::endPaint() const {
  glUseProgram( 0 );

  data->curProgram = 0;
  data->bindedProg = 0;
  }

void GLSL::setDevice() const {
  }

AbstractShadingLang::ProgramObject*
  GLSL::createShaderFromSource( const Source &src,
                                std::string &outputLog) const {
  if( src.vs.size()==0 &&
      src.fs.size()==0 ){
    outputLog = "no fragment or vertex shader code to compile";
    return 0;
    }

  Data::ShProgram* sh = new Data::ShProgram();
  outputLog.clear();
  std::string log;

  sh->vs = data->loadShader( GL_VERTEX_SHADER,   src.vs.data(), log );
  outputLog += log;
  sh->fs = data->loadShader( GL_FRAGMENT_SHADER, src.fs.data(), log );
  outputLog += log;

  bool valid = (sh->vs!=0) && (sh->fs!=0);
  if(src.ts.size()){
    sh->ts = data->loadShader( GL_TESS_CONTROL_SHADER, src.ts.data(), log );
    valid &= (sh->ts!=0);
    outputLog += log;
    }

  if(src.es.size()){
    sh->es = data->loadShader( GL_TESS_EVALUATION_SHADER, src.es.data(), log );
    valid &= (sh->es!=0);
    outputLog += log;
    }

  if(!valid){
    delete sh;
    return 0;
    }

  return (AbstractShadingLang::ProgramObject*)sh;
  }

void GLSL::deleteShader(GraphicsSubsystem::ProgramObject *p) const {
  Data::ShProgram* prog = (Data::ShProgram*)p;

  size_t nsz = 0;
  for( size_t i=0; i<data->prog.size(); ++i ){
    data->prog[nsz] = data->prog[i];

    if( data->prog[i].shader == prog ){
      if( glIsProgram(data->prog[i].linked) )
        glDeleteProgram( data->prog[i].linked );
      } else {
      ++nsz;
      }
    }

  data->prog.data.resize( nsz );
  delete prog;

  if(data->curProgram==prog){
    data->curProgram = 0;
    data->curUbo.reset();
    data->bindedProg = 0;
    }
  }

AbstractShadingLang::Source
  GLSL::surfaceShader( const AbstractShadingLang::UiShaderOpt &opt,
                       bool &hasHalfPixelOffset ) const {
  AbstractShadingLang::Source src;
  src.vs = surfaceShader(Vertex,  opt,hasHalfPixelOffset);
  src.fs = surfaceShader(Fragment,opt,hasHalfPixelOffset);
  return src;
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

void GLSL::bind(const Tempest::ShaderProgram &p) const {
  if( data->curProgram != (Data::ShProgram*)get(p) ){
    data->curProgram = (Data::ShProgram*)get(p);
    data->curLinked.linked = 0;
    }

  data->curUbo = inputOf(p);
  }

void GLSL::setVertexDecl(const AbstractAPI::VertexDecl *v ) const {
  if(data->curLinked.vdecl!=v){
    data->curLinked.vdecl  = v;
    data->curLinked.linked = 0;
    }
  }

void GLSL::enable() const {
  data->makeCurrent( data->curProgram );
  auto& ubo = *data->curUbo;

  int slot=0;
  for( UBO u:ubo ){
    if(!u.updated){
      setUniforms( data->bindedProg, u, slot );
      u.updated = true;
      }
    }
  }

void GLSL::disable() const {
  }

void GLSL::setUniforms( unsigned int /*prog*/,
                        const UBO &ux,
                        int& slot ) const {
  const char*  name      = ux.names.data();
  intptr_t const* fields = ux.fields.data();
  void* const * tex      = ux.tex.data();
  uintptr_t*    id       = ux.id.data();
  Texture2d::Sampler* smp2d = (Texture2d::Sampler*)ux.smp[0].data();
  Texture3d::Sampler* smp3d = (Texture3d::Sampler*)ux.smp[1].data();

  for( int t: ux.desc ){
    const char* v = ux.data.data() + fields[0];
    ++fields;

    if(*id!=uintptr_t(-1))
      switch(t){
        case Decl::float1:
          glUniform1fv( *id, 1, (GLfloat*)v );
          break;
        case Decl::float2:
          glUniform2fv( *id, 1, (GLfloat*)v );
          break;
        case Decl::float3:
          glUniform3fv( *id, 1, (GLfloat*)v );
          break;
        case Decl::float4:
          glUniform4fv( *id, 1, (GLfloat*)v );
          break;
        case Decl::Texture2d:{
          Detail::GLTexture* t = *(Detail::GLTexture**)tex;
          if(t)
            data->setupSampler(GL_TEXTURE_2D,*id,slot,t,*smp2d);
          ++slot;
          }
          break;
        case Decl::Texture3d:{
          Detail::GLTexture* t = *(Detail::GLTexture**)tex;
          if(t)
            data->setupSampler(GL_TEXTURE_3D,*id,slot,t,*smp3d);
          ++slot;
          }
          break;
        case Decl::Matrix4x4:
          glUniformMatrix4fv( *id, 1, false, (GLfloat*)v );
          break;
        }
    if(t==Decl::Texture2d){
      ++tex;
      ++smp2d;
      } else
    if(t==Decl::Texture3d){
      ++tex;
      ++smp3d;
      }
    name += strlen(name)+1;
    ++id;
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

    if( data->prog[i].vdecl == e.declaration ){
      if( glIsProgram(data->prog[i].linked) )
        glDeleteProgram( data->prog[i].linked );
      } else {
      ++nsz;
      }
    }

  data->prog.data.resize( nsz );
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
