#include "imagecodec.h"

#include <Tempest/SystemAPI>
#include <Tempest/Assert>
#include "core/wrappers/atomic.h"

#ifndef __ANDROID__
#include <IL/il.h>
#endif

#include <libpng/png.h>
#include "../system/ddsdef.h"

namespace Tempest {

#ifndef __ANDROID__
struct DevILCodec:ImageCodec {
  static void initImgLib() {
    static Detail::Spin spin;
    Detail::Guard guard(spin);
    (void)guard;

    static bool wasInit = false;
    if( !wasInit )
      ilInit();
    ilEnable(IL_FILE_OVERWRITE);

    wasInit = true;
    }

  template< class ChanelType, int mul >
  void initRawData( std::vector<unsigned char> &d,
                    void * input,
                    int bpp,
                    int w,
                    int h,
                    int * ix ){
    ChanelType * img = reinterpret_cast<ChanelType*>(input);

    d.resize( bpp*w*h );

    for( int i=0; i<w; ++i )
      for( int r=0; r<h; ++r ){
        ChanelType * raw = &img[ bpp*(i+r*w) ];
        for( int q=0; q<bpp; ++q ){
          d[ bpp*(i+r*w)+q ] = ( raw[ ix[q] ]*mul );
          }

        }
    }

  bool canSave(ImageCodec::ImgInfo &inf) const {
    return inf.format == Pixmap::Format_RGB || inf.format == Pixmap::Format_RGBA;
    }

  bool load( const std::vector<char> &imgBytes,
             ImageCodec::ImgInfo &info,
             std::vector<unsigned char> &out) {
    initImgLib();
    bool ok = true;

    ILuint	id;
    ilGenImages ( 1, &id );
    ilBindImage ( id );

    //if( ilLoadImage( file ) ){
    if( ilLoadL( IL_TYPE_UNKNOWN, &imgBytes[0], imgBytes.size() ) ){
      int format = ilGetInteger( IL_IMAGE_FORMAT );

      int size_of_pixel = 1;
      info.bpp = 3;
      info.format = Pixmap::Format_RGB;

      int idx[5][4] = {
        { 0, 1, 2, 3 }, //IL_RGB, IL_RGBA
        { 0, 0, 0, 0 }, //IL_ALPHA
        { 2, 1, 0, 3 },
        { 2, 1, 0, 3 },
        { 0, 0, 0, 1 }
        };
      int *ix = idx[0];


      switch( format ){
        case IL_RGB:   size_of_pixel = 3; break;
        case IL_RGBA:  size_of_pixel = 4; info.bpp = 4; break;
        case IL_ALPHA: size_of_pixel = 1; ix = idx[1]; break;

        case IL_BGR:   size_of_pixel = 3; ix = idx[2]; break;
        case IL_BGRA:  size_of_pixel = 4; ix = idx[3]; break;
        case IL_LUMINANCE_ALPHA:
          size_of_pixel = 4; ix = idx[4];
          break;
        default: ok = false;
        }

      if( info.bpp==4 )
        info.format = Pixmap::Format_RGBA;

      if( ok ){
        ILubyte * data = ilGetData();
        //Data * image = new Data();

        info.w = ilGetInteger ( IL_IMAGE_WIDTH  );
        info.h = ilGetInteger ( IL_IMAGE_HEIGHT );

        if( ilGetInteger ( IL_IMAGE_TYPE ) == IL_UNSIGNED_BYTE )
          initRawData<ILubyte,   1> ( out, data, size_of_pixel, info.w, info.h, ix ); else
          initRawData<ILdouble, 255>( out, data, size_of_pixel, info.w, info.h, ix );
        }
      } else {
      ok = false;
      }

    ilDeleteImages( 1, &id );

    return ok;
    }

  bool save( const char *file,
             ImgInfo &info,
             std::vector<unsigned char> &in ){
    initImgLib();

    ILuint	id;
    ilGenImages ( 1, &id );
    ilBindImage ( id );

    ilTexImage( info.w, info.h, 1,
                info.bpp,
                (info.bpp==4)? IL_RGBA : IL_RGB,
                IL_UNSIGNED_BYTE,
                in.data() );

    ILubyte * img = ilGetData();

    int h2 = info.h/2;
    for( int i=0; i<info.w; ++i )
      for( int r=0; r<h2; ++r ){
        ILubyte * raw1 = &img[ info.bpp*(i+r*info.w) ];
        ILubyte * raw2 = &img[ info.bpp*(i+(info.h-r-1)*info.w) ];

        for( int q=0; q<info.bpp; ++q )
          std::swap( raw1[q], raw2[q] );
        }

    bool ok = ilSaveImage( file );
    ilDeleteImages( 1, &id );

    return ok;
    }

  };
#endif

struct PngCodec:ImageCodec {
  PngCodec(){
    rows.reserve(2048);
    }

  bool load( const std::vector<char> &d,
             ImgInfo &info,
             std::vector<unsigned char> &out ) {
    if( d.size()<8 )// 8 is the maximum size that can be checked
      return false;

    unsigned char* data = (unsigned char*)&d[0];
    if (png_sig_cmp( data, 0, 8))
      return false;
    data += 8;

    png_structp png_ptr;

    /* initialize stuff */
    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

    if (!png_ptr)
      return 0;

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
      return 0;

    if( setjmp(png_jmpbuf(png_ptr)) )
      return 0;

    struct Reader{
      bool err;
      unsigned char* data;
      size_t sz;

      static void read( png_structp png_ptr,
                        png_bytep   outBytes,
                        png_size_t  byteCountToRead ){
        if( !png_ptr->io_ptr ){
          return;
          }

        Reader& r = *(Reader*)png_ptr->io_ptr;

        if( r.sz<byteCountToRead ){
          r.err = true;
          return;
          }

        r.sz -= byteCountToRead;

        for(; byteCountToRead>0; --byteCountToRead ){
          *outBytes = *r.data;
          ++r.data;
          ++outBytes;
          }
        }
      };

    Reader r;
    r.err  = false;
    r.data = data;
    r.sz   = d.size()-8;
    //png_init_io(png_ptr, data);
    png_set_read_fn( png_ptr, &r, &Reader::read );

    png_set_sig_bytes(png_ptr, 8);

    png_read_info(png_ptr, info_ptr);

    info.w = png_get_image_width (png_ptr, info_ptr);
    info.h = png_get_image_height(png_ptr, info_ptr);

    png_byte color_type = png_get_color_type(png_ptr, info_ptr);
    png_byte bit_depth  = png_get_bit_depth(png_ptr, info_ptr);

    if( color_type == PNG_COLOR_TYPE_PALETTE )
      png_set_palette_to_rgb(png_ptr);

    color_type = png_get_color_type(png_ptr, info_ptr);

    if( color_type==PNG_COLOR_TYPE_RGB ){
      info.bpp=3;
      info.format = Pixmap::Format_RGB;
      }

    if( color_type==PNG_COLOR_TYPE_RGB_ALPHA ){
      info.bpp=4;
      info.format = Pixmap::Format_RGBA;
      }

    if( ( color_type==PNG_COLOR_TYPE_RGB ||
          color_type==PNG_COLOR_TYPE_RGB_ALPHA ) &&
        bit_depth==8 ){
      out.resize( info.w*info.h*info.bpp );
      //int number_of_passes =
          png_set_interlace_handling(png_ptr);
      png_read_update_info(png_ptr, info_ptr);

      if (setjmp(png_jmpbuf(png_ptr)))
        return false;

      if( !(info.bpp==3 || info.bpp==4 ) )
        return false;

      Detail::Guard guard(spin);
      (void)guard;

      rows.resize( info.h );

      for( int y=0; y<info.h; y++ )
        rows[y] = &out[ y*info.w*info.bpp ];

      png_read_image(png_ptr, &rows[0]);
      return !r.err;
      }

    return false;
    }

  std::vector<png_bytep> rows;
  Detail::Spin spin;
  };

struct S3TCCodec:ImageCodec {
  bool load( const std::vector<char> &img,
             ImgInfo &info,
             std::vector<unsigned char> &out ) {
    DDSURFACEDESC2 ddsd;
    //std::vector<char> img = loadBytesImpl(data);
    const char* pos = &img[0];
    if( img.size()<4+sizeof(ddsd) )
      return false;

    if( strncmp( pos, "DDS ", 4 ) != 0 ) {
      return false;
      }

    //std::cout << sizeof(DWORD) << std::endl;
    pos+=4;
    memcpy(&ddsd, pos, sizeof(ddsd) );
    pos += sizeof(ddsd);

    int factor;
    switch( ddsd.ddpfPixelFormat.dwFourCC ) {
      case FOURCC_DXT1:
        info.bpp    = 3;
        info.format = Pixmap::Format_DXT1;
        factor = 2;
        break;

      case FOURCC_DXT3:
        info.bpp    = 4;
        info.format = Pixmap::Format_DXT3;
        factor = 4;
        break;

      case FOURCC_DXT5:
        info.bpp    = 4;
        info.format = Pixmap::Format_DXT5;
        factor = 4;
        break;

      default:
        return false;
      }

    if( ddsd.dwLinearSize == 0 ) {
      //return false;
      }

    info.w = ddsd.dwWidth;
    info.h = ddsd.dwHeight;
    int bufferSize = info.w*info.h/factor;

    if( ddsd.dwMipMapCount > 1 ){
      int mipW = info.w, mipH = info.h;
      for( size_t i=0; i<ddsd.dwMipMapCount; ++i ){
        if(mipW>1)
          mipW/=2;
        if(mipH>1)
          mipH/=2;

        bufferSize += mipW*mipH/factor;
        }
      }
    //if( ddsd.dwMipMapCount > 1 )
    //  bufferSize = ddsd.dwLinearSize * factor; else
      //bufferSize = ddsd.dwLinearSize;

    size_t sz = 4+sizeof(ddsd)+bufferSize;
    if( img.size() < sz )
      return false;

    out.reserve( bufferSize );
    out.resize ( bufferSize );
    memcpy( &out[0], pos, bufferSize );

    return true;
    }
  };

ImageCodec::~ImageCodec() {
  }

bool ImageCodec::canSave(ImageCodec::ImgInfo &) const {
  return 0;
  }

bool ImageCodec::load( const char *file,
                       ImageCodec::ImgInfo &info,
                       std::vector<unsigned char> &out) {
  std::vector<char> imgBytes = SystemAPI::loadBytes(file);
  return load( imgBytes, info, out );
  }

bool ImageCodec::load(const wchar_t *file,
                       ImgInfo &info,
                       std::vector<unsigned char> &out ) {
  std::vector<char> imgBytes = SystemAPI::loadBytes(file);
  return load( imgBytes, info, out );
  }

bool ImageCodec::load( const std::vector<char> &,
                       ImageCodec::ImgInfo &,
                       std::vector<unsigned char> &) {
  T_WARNING_X(0, "overload for ImageCodec::load is unimplemented");
  return false;
  }

bool ImageCodec::save( const char *,
                       ImageCodec::ImgInfo &,
                       std::vector<unsigned char> &) {
  T_WARNING_X(0, "overload for ImageCodec::save is unimplemented");
  return false;
  }

bool ImageCodec::save( const wchar_t *,
                       ImageCodec::ImgInfo &,
                       std::vector<unsigned char> & ) {
  T_WARNING_X(0, "overload for ImageCodec::save is unimplemented");
  return false;
  }

void ImageCodec::installStdCodecs(SystemAPI &s) {
  s.installImageCodec( new PngCodec() );
  s.installImageCodec( new S3TCCodec() );
#ifndef __ANDROID__
  s.installImageCodec( new DevILCodec() );
#endif
  }

}
