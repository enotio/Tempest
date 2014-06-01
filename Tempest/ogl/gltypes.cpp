#include "gltypes.h"

Tempest::Detail::GLTexture::GLTexture() {
  id = 0;
  depthId = 0;
  w = 0;
  h = 0;
  z = 0;

  min    = mag    = GL_NEAREST;
  clampU = clampV = GL_REPEAT;
  clampW = GL_REPEAT;
  mips = false;

  format = 0;
  anisotropyLevel = 1;

  pixelFormat  = GL_UNSIGNED_BYTE;
  isInitalized = true;

  fboHash = -1;
  }

Tempest::Detail::ImplDeviceBase::ImplDeviceBase() {
  isTileRenderStarted = false;
  isPainting = false;
  memset( &target, 0, sizeof(target) );
  lbUseed = false;
  tmpLockBuffer.reserve( 8096*32 );

  renderState.setCullFaceMode( RenderState::CullMode::noCull );
  renderState.setZTest(0);
  renderState.setZWriting(1);
  renderState.setAlphaTestMode( RenderState::AlphaTestMode::Always );
  renderState.setBlend(0);

  clearColor = Color(0);
  clearDepth = 1;
  clearS     = 0;

  memset( (char*)&caps, 0, sizeof(caps) );
  }

void Tempest::Detail::ImplDeviceBase::initExt() {
  const char * ext = (const char*)glGetString(GL_EXTENSIONS);
  if( ext==0 )
    ext = "";

  T_ASSERT_X(ext, "opengl context not created");

  hasS3tcTextures =
      (strstr(ext, "GL_OES_texture_compression_S3TC")!=0) ||
      (strstr(ext, "GL_EXT_texture_compression_s3tc")!=0);
  hasETC1Textures = (strstr(ext, "GL_OES_compressed_ETC1_RGB8_texture")!=0);
  hasWriteonlyRendering = (strstr(ext, "GL_QCOM_writeonly_rendering")!=0);

  hasNpotTexture = (strstr(ext, "GL_OES_texture_npot")!=0) ||
                   (strstr(ext, "GL_ARB_texture_non_power_of_two")!=0);
  //dev->hasNpotTexture = 0;

  hasHalfSupport            = (strstr(ext, "GL_OES_vertex_half_float")!=0) ||
                              (strstr(ext, "GL_ARB_half_float_vertex")!=0);

#ifdef __ANDROID__
  hasRenderToRGBTextures    = strstr(ext, "GL_OES_rgb8_rgba8")!=0;
#else
  hasRenderToRGBTextures     = 1;
#endif

  hasQCOMTiles      = strstr(ext, "GL_QCOM_tiled_rendering")!=0;
  hasDiscardBuffers = strstr(ext, "GL_EXT_discard_framebuffer")!=0;

#ifdef __WIN32
  if( strstr(ext, "WGL_EXT_swap_control") ){
    wglSwapInterval = (Detail::PFNGLWGLSWAPINTERVALPROC)wglGetProcAddress("wglSwapIntervalEXT");
    }
#endif

  hasTileBasedRender = hasQCOMTiles | hasDiscardBuffers;

#ifdef __ANDROID__
  if( hasQCOMTiles ){
    glStartTilingQCOM = (Detail::PFNGLSTARTTILINGQCOMPROC)eglGetProcAddress("glStartTilingQCOM");
    glEndTilingQCOM   =   (Detail::PFNGLENDTILINGQCOMPROC)eglGetProcAddress("glEndTilingQCOM");
    }

  if( hasDiscardBuffers ){
    glDiscardFrameBuffer = (Detail::PFNGLDISCARDFRAMEBUFFERPROC)eglGetProcAddress("glDiscardFramebufferEXT");
    }
#endif

  caps.hasHalf2 = hasHalfSupport;
  caps.hasHalf4 = hasHalfSupport;
  caps.hasRedableDepth = (strstr(ext, "GL_OES_depth_texture")!=0) ||
                         (strstr(ext, "GL_ARB_depth_texture")!=0);

  glGetIntegerv( GL_MAX_TEXTURE_SIZE,         &caps.maxTextureSize );
#ifdef __ANDROID__
  glGetIntegerv( GL_MAX_VARYING_VECTORS,      &caps.maxVaryingVectors );
  caps.maxVaryingComponents = caps.maxVaryingVectors*4;
#else
  glGetIntegerv( GL_MAX_VARYING_COMPONENTS,   &caps.maxVaryingComponents );
  caps.maxVaryingVectors = caps.maxVaryingComponents/4;
#endif
  //T_ASSERT_X( errCk(), "OpenGL error" );

#ifdef __ANDROID__
  caps.maxRTCount = 1;
#else
  glGetIntegerv( GL_MAX_COLOR_ATTACHMENTS, &caps.maxRTCount );
  if( caps.maxRTCount>32 )
    caps.maxRTCount = 32;
#endif
  caps.hasNpotTexture = hasNpotTexture;
  wglSwapInterval = 0;
  }
