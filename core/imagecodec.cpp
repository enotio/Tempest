#include "imagecodec.h"

#include <Tempest/SystemAPI>
#include <Tempest/Assert>
#include "core/wrappers/atomic.h"

#ifndef __ANDROID__
#include <IL/il.h>
#endif

#include <libpng/png.h>
#include "../system/ddsdef.h"
#include <squish/squish.h>
#include <cstdint>

#include "thirdparty/ktx/etc_dec.h"

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

      info.alpha  = (info.format==Pixmap::Format_RGBA);

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
    if( !(info.format==Pixmap::Format_RGB || info.format==Pixmap::Format_RGBA) )
      return 0;

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
        if( !png_get_io_ptr(png_ptr) ){
          return;
          }

        Reader& r = *(Reader*)png_get_io_ptr(png_ptr);

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
    info.alpha  = (info.format == Pixmap::Format_RGBA);

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

      info.mipLevels = ddsd.dwMipMapCount;
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
    info.alpha  = (info.format!=Pixmap::Format_DXT1);

    return true;
    }

  static bool dxtFrm( Pixmap::Format fout ){
    return fout==Pixmap::Format_DXT1 ||
           fout==Pixmap::Format_DXT3 ||
           fout==Pixmap::Format_DXT5;
    }

  bool canConvertTo( ImgInfo &info, Pixmap::Format fout ) const{
    return ( info.format==Pixmap::Format_RGB  && dxtFrm(fout) ) ||
           ( info.format==Pixmap::Format_RGBA && dxtFrm(fout) ) ||
           ( dxtFrm(info.format) && fout==Pixmap::Format_RGBA );
    }

  void fromRGB( ImageCodec::ImgInfo &info,
                std::vector<unsigned char> &bytes ) {
    squish::u8 px[4][4][4];
    unsigned char* p  = &bytes[0];

    std::vector<squish::u8> d;
    d.resize( info.w*info.h/2 );

    for( int i=0; i<info.w; i+=4 )
      for( int r=0; r<info.h; r+=4 ){
        for( int y=0; y<4; ++y )
          for( int x=0; x<4; ++x ){
            px[y][x][3] = 255;
            std::copy( p+((x + i + (y+r)*info.w)*info.bpp),
                       p+((x + i + (y+r)*info.w)*info.bpp)+info.bpp,
                       px[y][x] );
            }

        int pos = ((i/4) + (r/4)*info.w/4)*8;
        squish::Compress( (squish::u8*)px,
                          &d[pos], squish::kDxt1 );
        }

    bytes = d;
    info.format = Pixmap::Format_DXT1;
    info.alpha  = (info.format!=Pixmap::Format_DXT1);
    }

  void toRGB( ImageCodec::ImgInfo &info,
              std::vector<unsigned char> &bytes,
              bool a ) {
    squish::u8 pixels[4][4][4];
    int compressType = squish::kDxt1;

    size_t sz = info.w*info.h, s = sz;
    if( info.format==Pixmap::Format_DXT1 ){
      compressType = squish::kDxt1;
      sz /= 2;
      s *= 4;
      } else {
      if( info.format==Pixmap::Format_DXT5 )
        compressType = squish::kDxt5; else
        compressType = squish::kDxt3;
      s *= 4;
      }

    std::vector<unsigned char> ddsv = bytes;
    bytes.resize(s);

    unsigned char* px  = &bytes[0];
    unsigned char* dds = &ddsv[0];
    info.bpp = 4;

    for( int i=0; i<info.w; i+=4 )
      for( int r=0; r<info.h; r+=4 ){
        int pos = ((i/4) + (r/4)*info.w/4)*8;
        squish::Decompress( (squish::u8*)pixels,
                            &dds[pos], compressType );

        for( int x=0; x<4; ++x )
          for( int y=0; y<4; ++y ){
            unsigned char * v = &px[ (i+x + (r+y)*info.w)*info.bpp ];
            std::copy( pixels[y][x], pixels[y][x]+4, v);
            }
        }

    info.format = Pixmap::Format_RGBA;
    info.alpha  = a;
    if( !a )
      removeAlpha(info, bytes);
    }
  };

struct ETCCodec:ImageCodec{
  static bool etcFrm( Pixmap::Format fout ){
    return fout==Pixmap::Format_ETC1_RGB8;
    }

  bool canConvertTo( ImgInfo &info, Pixmap::Format fout ) const{
    return ( info.format==Pixmap::Format_RGB   && etcFrm(fout) ) ||
           ( info.format==Pixmap::Format_RGBA  && etcFrm(fout) ) ||
           ( etcFrm(info.format) && fout==Pixmap::Format_RGB );
    }

  bool canSave(ImgInfo &inf) const{
    return etcFrm(inf.format);
    }

  void compressLayer( std::vector<unsigned char> &img,
                      std::vector<unsigned char> &out,
                      int w, int h ){
    uint32_t block[2];
    uint8_t  bytes[4];

    std::unique_ptr<uint8_t[]> imgdec;
    imgdec.reset( new unsigned char[w*h*3] );

    out.resize( out.size()+ (w/4)*(h/4)*8 );
    uint8_t *pix = &out[ out.size() - (w/4)*(h/4)*8 ];

    for(int y=0;y<h/4;y++)
      for(int x=0;x<w/4;x++) {
        compressBlockDiffFlipFastPerceptual( img.data(), imgdec.get(),
                                             w,h,
                                             4*x,4*y,
                                             block[0], block[1]);

        for( int i=0; i<2; ++i ){
          for( int r=0; r<4; ++r ){
            bytes[3-r] = block[i]%256;
            block[i] /= 256;
            }

          pix[0] = bytes[0];
          pix[1] = bytes[1];
          pix[2] = bytes[2];
          pix[3] = bytes[3];
          pix += 4;
          }
        }
    }

  void fromRGB(ImgInfo &info, std::vector<unsigned char> &img){
    if( info.format==Pixmap::Format_RGBA )
      removeAlpha(info, img);

    int expandedwidth  = ((info.w+3)/4)*4,
        expandedheight = ((info.h+3)/4)*4;

    std::vector<unsigned char> out;
    compressLayer(img, out, expandedwidth, expandedheight);

    bool isPot = ((expandedwidth  & (expandedwidth-1) )==0) &&
                 ((expandedheight & (expandedheight-1))==0);

    if( isPot && expandedwidth==expandedheight ){
      info.mipLevels = 0;
      Pixmap::ImgInfo inf = info;
      while( expandedheight>=2 ){
        if( expandedwidth>4 )
          downsample(inf, img);

        ++info.mipLevels;
        expandedwidth  /= 2;
        expandedheight /= 2;

        compressLayer(img, out, expandedwidth, expandedheight);
        }
      }

    img = std::move(out);
    info.alpha  = false;
    info.format = Pixmap::Format_ETC1_RGB8;
    }

  void toRGB( ImgInfo &info,
              std::vector<unsigned char> &inout,
              bool alpha ){
    uint8_t *bytes = ((uint8_t*)(&inout[0]));
    std::vector<uint8_t> img;
    img.resize( info.w*info.h*3 );

    uint32_t block[2];

    for(int y=0;y<info.h/4;y++) {
      for(int x=0;x<info.w/4;x++) {
        for( int i=0; i<2; ++i ){
          block[i] = 0;
          block[i] |= bytes[0];
          block[i]*=256;
          block[i] |= bytes[1];
          block[i]*=256;
          block[i] |= bytes[2];
          block[i]*=256;
          block[i] |= bytes[3];

          bytes+=4;
          }

        decompressBlockDiffFlip( block[0], block[1], &img[0], info.w, info.h, 4*x,4*y);
        }
      }

    inout       = std::move(img);
    info.bpp    = 3;
    info.format = Pixmap::Format_RGB;

    if( alpha ){
      addAlpha(info, img);
      }
    }

  bool save( const char *file,
             ImgInfo &info,
             std::vector<unsigned char> &img){
    if( info.format!=Pixmap::Format_ETC1_RGB8 )
      return 0;

    std::vector<char> buf;

    KTX_header header;
    static const uint8_t ktx_identifier[12] = { 0xAB, 0x4B, 0x54, 0x58, 0x20, 0x31, 0x31, 0xBB, 0x0D, 0x0A, 0x1A, 0x0A };

    for(int i=0; i<12; i++) {
      header.identifier[i]=ktx_identifier[i];
      }

    header.endianness=KTX_ENDIAN_REF;

    header.glType=0;
    header.glTypeSize=1;
    header.glFormat=0;

    header.pixelWidth  = info.w;
    header.pixelHeight = info.h;
    header.pixelDepth=0;

    header.numberOfArrayElements=0;
    header.numberOfFaces=1;
    header.numberOfMipmapLevels= std::max(1, info.mipLevels);

    header.bytesOfKeyValueData=0;

    int halfbytes=1;
    header.glBaseInternalFormat = 0x1907;//GL_RGB;
    header.glInternalFormat     = 0x8D64;//GL_ETC1_RGB8_OES;

    //write header
    buf.insert(buf.end(), (char*)&header, (char*)&header+sizeof(header) );

    //write size of compressed data.. which depend on the expanded size..
    uint32_t imagesize=(info.w*info.h*halfbytes)/2;
    buf.insert(buf.end(), (char*)&imagesize, (char*)&imagesize+sizeof(imagesize) );

    buf.insert(buf.end(), img.begin(), img.end() );
    SystemAPI::writeBytes(file, buf);
    return 1;
    }

  bool load( const std::vector<char> &data,
             ImgInfo &info,
             std::vector<unsigned char> & out ) {
    static const size_t hsz = sizeof(KTX_header)+sizeof(uint32_t);
    if( data.size()<hsz )
      return 0;

    const KTX_header& h = (*(const KTX_header*)&data[0]);
    static const uint8_t ktx_identifier[12] = { 0xAB, 0x4B, 0x54, 0x58, 0x20, 0x31, 0x31, 0xBB, 0x0D, 0x0A, 0x1A, 0x0A };

    for(int i=0; i<=11; i++) {
      if( h.identifier[i]!=ktx_identifier[i] )
        return 0;
      }

    info.w      = h.pixelWidth;
    info.h      = h.pixelHeight;

    size_t pos = hsz;
    uint32_t mips = std::max<uint32_t>(h.numberOfMipmapLevels, 1);

    int wx = info.w, hx = info.h;
    for( uint32_t i=0; i<mips; ++i ){
      if( (data.size()-pos) < size_t((wx/4)*(hx/4)*8) )
        return 0;
      pos += size_t((wx/4)*(hx/4)*8);

      if(wx>1) wx /= 2;
      if(hx>1) hx /= 2;
      }

    info.mipLevels = h.numberOfMipmapLevels;
    info.alpha     = false;
    info.format    = Pixmap::Format_ETC1_RGB8;
    info.bpp       = 3;

    out.resize( data.size()-hsz );
    std::copy( data.begin()+hsz, data.end(), out.begin() );
    return 1;
    }
  };

ImageCodec::~ImageCodec() {
  }

bool ImageCodec::canSave(ImageCodec::ImgInfo &) const {
  return 0;
  }

bool ImageCodec::canConvertTo(ImageCodec::ImgInfo &, Pixmap::Format ) const {
  return 0;
  }

void ImageCodec::toRGB( ImageCodec::ImgInfo &/*info*/,
                        std::vector<unsigned char> &/*inout*/,
                        bool /*alpha*/ ) {
  T_WARNING_X(0, "overload for ImageCodec::toRGB is unimplemented");
  }

void ImageCodec::fromRGB( ImageCodec::ImgInfo &/*info*/,
                          std::vector<unsigned char> &/*inout*/ ) {
  T_WARNING_X(0, "overload for ImageCodec::fromRGB is unimplemented");
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
  s.installImageCodec( new ETCCodec() );
  s.installImageCodec( new S3TCCodec() );
#ifndef __ANDROID__
  s.installImageCodec( new DevILCodec() );
#endif
  }

void ImageCodec::addAlpha( ImgInfo& info, std::vector<unsigned char> &rgb) {
  rgb.resize( info.w*info.h*4);
  unsigned char * v = &rgb[0];

  for( size_t i=info.w*info.h; i>0; --i ){
    unsigned char * p = &v[ 4*i-4 ];
    unsigned char * s = &v[ 3*i-3 ];
    for( int r=0; r<3; ++r )
      p[r] = s[r];
    p[3] = 255;
    }

  info.bpp    = 4;
  info.alpha  = true;
  info.format = Pixmap::Format_RGBA;
  }

void ImageCodec::removeAlpha(ImageCodec::ImgInfo &info, std::vector<unsigned char> &rgba) {
  unsigned char * v = &rgba[0];

  for( int i=0; i<info.w*info.h; ++i ){
    unsigned char * p = &v[ 3*i ];
    unsigned char * s = &v[ 4*i ];
    for( int r=0; r<3; ++r )
      p[r] = s[r];
    }

  rgba.resize(info.w*info.h*3);
  info.bpp    = 3;
  info.alpha  = false;
  info.format = Pixmap::Format_RGB;
  }

void ImageCodec::downsample( ImageCodec::ImgInfo &info,
                             std::vector<unsigned char> &rgb ) {
  T_ASSERT( info.w%2==0 && info.h%2==0 );
  T_ASSERT( info.format==Pixmap::Format_RGB || info.format==Pixmap::Format_RGBA );

  int bpp = info.bpp, iw = info.w, w = info.w/2, h = info.h/2;

  unsigned char *p = &rgb[0];
  for( int r=0; r<h; ++r)
    for( int i=0; i<w; ++i ){
      int pix[4] = {};
      int x0 = i*2, y0 = r*2;

      for( int r0=0; r0<2; ++r0)
        for( int i0=0; i0<2; ++i0)
          for( int q=0; q<bpp; ++q ){
            pix[q] += p[ (x0+i0 + (y0+r0)*iw)*bpp + q ];
            }

      for( int q=0; q<bpp; ++q )
        p[ (i + r*w)*bpp + q ] = pix[q]/4;
      }

  rgb.resize( rgb.size()/4 );
  info.w = w;
  info.h = h;
  }

}
