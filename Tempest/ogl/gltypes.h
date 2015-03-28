#ifndef GLTYPES_H
#define GLTYPES_H

#include <Tempest/Platform>

#ifdef __WINDOWS__
#include <windows.h>
#endif

#ifdef __ANDROID__
#include <EGL/egl.h>
#include <GLES/gl.h>
#include <GLES2/gl2.h>
#else
//#include <GL/glew.h>
#include "glfn.h"
#include <GL/gl.h>
#endif

#include <vector>
#include <unordered_set>
#include <Tempest/Texture2d>
#include <Tempest/RenderState>

#ifdef __ANDROID__
#define GLAPIENTRY EGLAPIENTRY
#endif

#define GL_RGBA16F_ARB        0x881A
#define GL_RGBA32F_ARB        0x8814
#define GL_RGB16F_ARB         0x881B
#define GL_RGB32F_ARB         0x8815
#define GL_ALPHA16F_ARB       0x881C
#define GL_ALPHA32F_ARB       0x8816
#define GL_LUMINANCE16F_ARB   0x881E
#define GL_R11F_G11F_B10F_EXT 0x8C3A

#ifndef GL_RGB16
#define GL_RGB16                          0x8054
#define GL_RGBA16                         0x805B
#define GL_LUMINANCE16                    0x8042
#define GL_RGB10                          0x8052
#endif

namespace Tempest{

namespace Detail{
  struct GLBuffer{
    GLuint id;
    char * mappedData;

    unsigned offset, size;
    size_t   vertexCount, byteCount;

    //opengl-es only
    std::vector<char> vba;
    };

  struct GLTexture;
  struct RenderTg{
    static const int maxMRT = 32;

    GLTexture* color[maxMRT];
    int        mip[maxMRT];

    GLTexture* depth;
    int depthMip;
    };

  struct GLTexture{
    GLTexture();

    GLuint id;
    GLuint depthId;

    size_t fboHash;

    GLenum min, mag;
    GLenum clampU,clampV, clampW;
    bool mips, compress;

    bool isInitalized;

    int w,h, z;
    float anisotropyLevel;
    GLenum format, pixelFormat;
    };


  typedef void (GLAPIENTRY *PFNGLSTARTTILINGQCOMPROC) (GLuint x, GLuint y, GLuint width, GLuint height, GLbitfield preserveMask);
  typedef void (GLAPIENTRY *PFNGLENDTILINGQCOMPROC) (GLbitfield preserveMask);
  typedef bool (GLAPIENTRY *PFNGLWGLSWAPINTERVALPROC) (GLint interval);

  typedef void (GLAPIENTRY *PFNGLDISCARDFRAMEBUFFERPROC)(GLenum mode, GLsizei count, const GLenum* att );

  struct ImplDeviceBase {
    ImplDeviceBase();

    void initExt();

  #ifdef __ANDROID__
    EGLDisplay disp;
    EGLSurface s;
    EGLint     swapEfect;
  #endif

  #ifdef __WINDOWS__
    HDC   hDC;
    HGLRC hRC;
  #endif

  #ifdef __LINUX__
    GLXContext   glc;
    Display*     dpy;
    XID          window;
  #endif

    bool hasS3tcTextures,
         hasETC1Textures,
         hasHalfSupport,
         hasWriteonlyRendering;

    bool useVBA( AbstractAPI::BufferUsage usage ){
      (void)usage;
  #ifdef __ANDROID__
      return usage==AbstractAPI::BU_Dynamic;
  #else
      return 0;
  #endif
      }

    int scrW, scrH;
    AbstractAPI::Caps caps;

    VertexDeclaration::Declarator * decl;
    GLuint vbo, curVBO;
    GLuint ibo, curIBO;

    Detail::GLBuffer *cVBO, *cIBO;

    std::vector<bool> vAttrLoc;

    int   vertexSize;
    char* curVboOffsetIndex;
    int   curIboOffsetIndex;
    bool  isPainting;

    Tempest::RenderState renderState;

    Detail::RenderTg target;
    struct FBOs{
      FBOs(){
        buckets.reserve(16);
        }

      struct Bucket{
        int count;
        std::vector<Detail::GLTexture*> targets;
        std::vector<int>    mip;
        std::vector<GLuint> fbo;
        };

      std::vector<Bucket> buckets;

      void onDeleteTexture( Detail::GLTexture* t ){
        for( size_t i=0; i<buckets.size(); ++i )
          onDeleteTexture( buckets[i], t );
        }

      void onDeleteTexture( Bucket &b, Detail::GLTexture* t ){
#ifndef __ANDROID__
        using namespace GLProc;
#endif

        int tgCount = b.count+1;
        for( size_t i=0, fboI = 0; i<b.targets.size();  ){
          bool ok = false;
          for( int r=0; r<tgCount; ++r )
            ok |= (b.targets[i+r]==t);

          if( ok ){
            glDeleteFramebuffers(1, &b.fbo[fboI]);

            b.fbo[fboI] = b.fbo.back();
            b.fbo.pop_back();

            size_t bsz = b.targets.size()-tgCount;
            for( int r=0; r<tgCount; ++r )
              b.targets[i+r] = b.targets[bsz+r];

            for( int r=0; r<tgCount; ++r )
              b.mip[i+r] = b.mip[bsz+r];

            b.targets.resize( b.targets.size()-tgCount );
            b.mip.resize( b.targets.size() );
            } else {
            i+=tgCount;
            fboI++;
            }
          }
        }

      bool cmpTagets( const Detail::RenderTg& tg,
                      Detail::GLTexture** rtg,
                      int *mip,
                      int sz ){
        bool ok = 1;
        for( int r=0; ok && r<sz; ++r )
          ok &=( tg.color[r]==rtg[r] && tg.mip[r]==mip[r]);

        ok &= ( tg.depth == rtg[sz] && tg.depthMip==mip[sz] );

        return ok;
        }

      GLuint& getTarget( const Detail::RenderTg& tg, int sz, size_t& hash ){
        Bucket &b = bucket(sz);
        int tgCount = sz+1;

        if( hash < b.fbo.size() &&
            cmpTagets(tg, &b.targets[hash*tgCount], &b.mip[hash*tgCount], sz ) )
          return b.fbo[hash];

        for( size_t i=0; i<b.targets.size(); i+=tgCount ){
          if( cmpTagets(tg, &b.targets[i], &b.mip[i], sz ) ){
            hash = i/tgCount;
            return b.fbo[hash];
            }
          }

        size_t i0 = b.targets.size();
        b.targets.resize( b.targets.size()+tgCount );
        for( size_t i=0; i<size_t(sz); ++i )
          b.targets[i0+i] = tg.color[i];
        b.targets.back() = tg.depth;

        b.mip.resize( b.targets.size() );
        for( size_t i=0; i<size_t(sz); ++i )
          b.mip[i0+i] = tg.mip[i];
        b.mip.back() = tg.depthMip;

        hash = b.fbo.size();
        b.fbo.push_back(0);
        return b.fbo.back();
        }

      Bucket& bucket( int mrt ){
        for( size_t i=0; i<buckets.size(); ++i ){
          if( buckets[i].count==mrt )
            return buckets[i];

          if( buckets[i].count>mrt ){
            buckets.emplace( buckets.begin()+i );
            return buckets[i];
            }
          }

        buckets.emplace_back();
        buckets.back().count = mrt;
        return buckets.back();
        }
      } fbo;

    std::vector<char> tmpLockBuffer;
    bool lbUseed;

    inline static uint32_t nextPot( uint32_t v ){
      v--;
      v |= v >> 1;
      v |= v >> 2;
      v |= v >> 4;
      v |= v >> 8;
      v |= v >> 16;
      v++;

      return v;
      }

    bool hasAlpha( AbstractTexture::Format::Type f ){
      if( AbstractTexture::Format::RGBA<=f &&
          f<=AbstractTexture::Format::RGBA16 )
        return true;

      if( AbstractTexture::Format::RGBA_DXT1<=f &&
          f<=AbstractTexture::Format::RGBA_DXT5 )
        return true;

      return false;
      }

    void texFormat( AbstractTexture::Format::Type f,
                    GLenum& storage,
                    GLenum& inFrm,
                    GLenum& inBytePkg,
                    bool renderable ){

  #ifndef __ANDROID__
    static const GLenum format[] = {
      GL_LUMINANCE16,
      GL_LUMINANCE4_ALPHA4,
      GL_LUMINANCE8,
      GL_LUMINANCE16,

      GL_RGB8,
      GL_RGB4,
      GL_RGB5,
      GL_RGB10,
      GL_RGB12,
      GL_RGB16,

      GL_RGBA,
      GL_RGB5_A1,
      GL_RGBA8,
      GL_RGB10_A2,
      GL_RGBA12,
      GL_RGBA16F_ARB,

      GL_COMPRESSED_RGB_S3TC_DXT1_EXT,
      GL_COMPRESSED_RGBA_S3TC_DXT1_EXT,
      GL_COMPRESSED_RGBA_S3TC_DXT3_EXT,
      GL_COMPRESSED_RGBA_S3TC_DXT5_EXT,

      GL_DEPTH_COMPONENT, //16
      GL_DEPTH_COMPONENT, //24
      GL_DEPTH_COMPONENT, //32

      GL_RG16,

      GL_DEPTH_COMPONENT16, //16
      GL_DEPTH_COMPONENT32, //32

      GL_RGBA
      };
  #else
    static const GLenum format[] = {
      GL_LUMINANCE,
      GL_LUMINANCE_ALPHA,
      GL_LUMINANCE,
      GL_LUMINANCE,

      GL_RGB,
      GL_RGB,
      GL_RGB,
      GL_RGB,
      GL_RGB,
      GL_RGB,

      GL_RGBA,
      GL_RGBA,
      GL_RGBA,
      GL_RGBA,
      GL_RGBA,
      GL_RGBA,

      GL_RGB,
      GL_RGBA,
      GL_RGBA,
      GL_RGBA,

      GL_LUMINANCE,
      GL_LUMINANCE,
      GL_LUMINANCE,

      GL_RGB,

      GL_DEPTH_COMPONENT, //16
      GL_DEPTH_COMPONENT, //32

      GL_RGBA
      };
  #endif

  #ifndef __ANDROID__
      static const GLenum inputFormat[] = {
      GL_LUMINANCE,
      GL_LUMINANCE_ALPHA,
      GL_LUMINANCE,
      GL_LUMINANCE,

      GL_RGB,
      GL_RGB,
      GL_RGB,
      GL_RGB,
      GL_RGB,
      GL_RGB,

      GL_RGBA,
      GL_RGBA,
      GL_RGBA,
      GL_RGBA,
      GL_RGBA,
      GL_RGBA,

      GL_RGB,
      GL_RGBA,
      GL_RGBA,
      GL_RGBA,

      GL_DEPTH_COMPONENT, //d
      GL_DEPTH_COMPONENT, //d
      GL_DEPTH_COMPONENT, //d

      GL_RGB,

      GL_DEPTH_COMPONENT, //d
      GL_DEPTH_COMPONENT, //d
      GL_RGBA
      };
  #endif

      if( renderable ){
        if( !hasRenderToRGBATexture && hasAlpha(f) )
          f = AbstractTexture::Format::RGBA5;
        else
        if( !hasRenderToRGBTexture && !hasAlpha(f) )
          f = AbstractTexture::Format::RGB5;
        }
      inBytePkg = GL_UNSIGNED_BYTE;
      if( f==AbstractTexture::Format::RGB16 ||
          f==AbstractTexture::Format::RGBA16 )
        inBytePkg = GL_UNSIGNED_SHORT;

  #ifdef __ANDROID__
      if( f==AbstractTexture::Format::RGB ||
          f==AbstractTexture::Format::RGB4 )
        inBytePkg = GL_UNSIGNED_SHORT_5_6_5;

      if( f==AbstractTexture::Format::RGB5 )
        inBytePkg = GL_UNSIGNED_SHORT_5_6_5;

      if( f==AbstractTexture::Format::RGBA5 ||
          f==AbstractTexture::Format::RGBA )
        inBytePkg = GL_UNSIGNED_SHORT_5_5_5_1;

      if( f==AbstractTexture::Format::RedableDepth16 )
        inBytePkg = GL_UNSIGNED_SHORT;

      if( f==AbstractTexture::Format::RedableDepth32 )
        inBytePkg = GL_UNSIGNED_INT;
  #endif

      storage = format[f];
  #ifdef __ANDROID__
      inFrm = storage;
  #else
      inFrm = inputFormat[f];
  #endif
      if(hasTextureFloat){
        switch( storage ){
          case GL_RGB16:
            storage = GL_RGB16F_ARB;
            break;
          case GL_RGBA16:
            storage = GL_RGBA16F_ARB;
            break;
          case GL_LUMINANCE16:
            storage = GL_LUMINANCE16F_ARB;
            break;
          default:
            break;
          }
        }
      if(hasPackedFloat){
        if(storage==GL_RGB10)
          storage = GL_R11F_G11F_B10F_EXT;
        }
      }

    static const char* glErrorDesc( GLenum err ){
      struct Err{
        const char* str;
        GLenum code;
        };

      Err e[] = {
        {"GL_INVALID_ENUM",      GL_INVALID_ENUM},
        {"GL_INVALID_VALUE",     GL_INVALID_VALUE},
        {"GL_INVALID_OPERATION", GL_INVALID_OPERATION},
        {"GL_STACK_OVERFLOW",    GL_STACK_OVERFLOW},
        {"GL_STACK_UNDERFLOW",   GL_STACK_UNDERFLOW},
        {"GL_OUT_OF_MEMORY",     GL_OUT_OF_MEMORY},
        {"GL_INVALID_FRAMEBUFFER_OPERATION", GL_INVALID_FRAMEBUFFER_OPERATION},
        {"", GL_NO_ERROR},
      };

      for( int i=0; e[i].code!=GL_NO_ERROR; ++i )
        if( e[i].code==err )
          return e[i].str;

      return 0;
      }

    Color clearColor;
    float clearDepth;
    unsigned  clearS;

    bool hasTileBasedRender, hasQCOMTiles, hasDiscardBuffers;
    bool hasRenderToRGBTexture, hasRenderToRGBATexture, hasNpotTexture;
    bool hasTextureFloat, hasPackedFloat;

    PFNGLSTARTTILINGQCOMPROC glStartTilingQCOM;
    PFNGLENDTILINGQCOMPROC   glEndTilingQCOM;
    PFNGLWGLSWAPINTERVALPROC wglSwapInterval = 0;

    PFNGLDISCARDFRAMEBUFFERPROC glDiscardFrameBuffer;
    bool isTileRenderStarted;

    MemPool<Detail::GLTexture> texPool;
    MemPool<Detail::GLBuffer>  bufPool;
    MemPool<VertexDeclaration::Declarator> declPool;

    struct DynBuffer{
      std::unordered_set<GLuint> used, freed;
      } dynVbo;

    void free( DynBuffer& buf ){
#ifndef __ANDROID__
        using namespace GLProc;
#endif
      for( GLuint id:buf.freed )
        glDeleteBuffers(1, &id);
      }
    };
  }
}

#endif // GLTYPES_H
