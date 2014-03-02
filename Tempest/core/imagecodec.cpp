#include "imagecodec.h"

#include <Tempest/SystemAPI>
#include <Tempest/Assert>
#include "core/wrappers/atomic.h"

#include <libpng/png.h>
#include "../system/ddsdef.h"
#include <squish/squish.h>
#include <cstdint>

#include "thirdparty/ktx/etc_dec.h"

#include <libjpeg/jinclude.h>
#include <libjpeg/jpeglib.h>

#include <Tempest/Buffer>
#include <Tempest/Log>
#include <Tempest/File>
#include <stdexcept>

namespace Tempest {

struct PngCodec:ImageCodec {
  PngCodec(){
    rows.reserve(2048);
    }

  bool canSave(const ImgInfo &inf) const{
    return inf.format == Pixmap::Format_RGB || inf.format==Pixmap::Format_RGBA;
    }

  bool load( IDevice &d,
             ImgInfo &info,
             std::vector<unsigned char> &out ) {
    char head[8];
    if( d.readData( head, 8 )!=8 )
      return false;

    if (png_sig_cmp( (unsigned char*)head, 0, 8))
      return false;

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
      IDevice* data;

      static void read( png_structp png_ptr,
                        png_bytep   outBytes,
                        png_size_t  byteCountToRead ){
        if( !png_get_io_ptr(png_ptr) ){
          return;
          }

        Reader& r = *(Reader*)png_get_io_ptr(png_ptr);

        if( r.data->readData((char*)outBytes, byteCountToRead)!=byteCountToRead ){//r.sz<byteCountToRead ){
          r.err = true;
          return;
          }
        }
      };

    Reader r;
    r.err  = false;
    r.data = &d;

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
      if( !r.err )
        png_read_end(png_ptr, info_ptr);
      return !r.err;
      }

    return false;
    }

  static void png_write_data(png_structp png_ptr, png_bytep data, png_size_t length){
    ODevice* f = (ODevice*)png_get_io_ptr(png_ptr);
    f->writeData((const char*)data, length);
    }

  static void png_flush(png_structp png_ptr){
    ODevice* f = (ODevice*)png_get_io_ptr(png_ptr);
    f->flush();
    }

  bool save( ODevice & file, const ImgInfo &info, const std::vector<unsigned char> &img){
    png_structp png_ptr = png_create_write_struct( PNG_LIBPNG_VER_STRING,
                                                   NULL, NULL, NULL );

    if( !png_ptr )
      return 0;

    png_infop  info_ptr = png_create_info_struct(png_ptr);
    if( !info_ptr )
      return 0;

    if( setjmp(png_jmpbuf(png_ptr)) )
      return 0;

    //png_init_io(png_ptr, fp);
    png_set_write_fn(png_ptr, &file, png_write_data, png_flush);

    /* write header */
    if (setjmp(png_jmpbuf(png_ptr)))
      return 0;

    png_set_IHDR( png_ptr, info_ptr, info.w, info.h,
                  8,//bit_depth,
                  (info.bpp==3)?PNG_COLOR_TYPE_RGB:PNG_COLOR_TYPE_RGB_ALPHA,
                  PNG_INTERLACE_NONE,
                  PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

    png_write_info(png_ptr, info_ptr);

    /* write bytes */
    if (setjmp(png_jmpbuf(png_ptr)))
      return 0;

    {
      Detail::Guard guard(spin);
      (void)guard;

      rows.resize( info.h );

      for( int y=0; y<info.h; y++ )
        rows[y] = (png_bytep)&img[ y*info.w*info.bpp ];
      png_write_image(png_ptr, &rows[0]);

      if (setjmp(png_jmpbuf(png_ptr)))
        return 0;

      png_write_end(png_ptr, NULL);
    }

    return 1;
    }

  std::vector<png_bytep> rows;
  Detail::Spin spin;
  };

struct JpegCodec:ImageCodec {
  JpegCodec(){
    }

  struct JpegStream {
    jpeg_source_mgr pub;

    IDevice* stream;
    static const size_t bufferSize = 4096;
    unsigned char buffer [bufferSize];
    };

  static void init_source( j_decompress_ptr ) {}

  static boolean fill_buffer ( j_decompress_ptr cinfo ) {
    JpegStream* src = (JpegStream*)(cinfo->src);
    src->pub.next_input_byte = src->buffer;
    src->pub.bytes_in_buffer = src->stream->readData( (char*)src->buffer,
                                                      JpegStream::bufferSize );

    return (/*src->stream->eof() &&*/ src->pub.bytes_in_buffer==0)?FALSE:TRUE;
    }

  static void skip (j_decompress_ptr cinfo, long count) {
    JpegStream* src = (JpegStream*)(cinfo->src);
    src->stream->skip(count);
    }

  static void term (j_decompress_ptr ) {
    // Close the stream, can be nop
    }

  static void handleLibJpegFatalError(j_common_ptr /*cinfo*/) {
    //(*cinfo->err->output_message)(cinfo);
    throw std::runtime_error("error in libjpeg");
    }

  void make_stream (jpeg_decompress_struct* cinfo, IDevice* in) {
    JpegStream * src;

    if (cinfo->src == NULL) {
      cinfo->src = (jpeg_source_mgr *)
          (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_IMAGE,
                                      sizeof(JpegStream));
      src = reinterpret_cast<JpegStream*> (cinfo->src);
      }

    src = reinterpret_cast<JpegStream*> (cinfo->src);
    src->pub.init_source       = init_source;
    src->pub.fill_input_buffer = fill_buffer;
    src->pub.skip_input_data   = skip;
    src->pub.resync_to_restart = jpeg_resync_to_restart; /* use default method */
    src->pub.term_source       = term;
    src->stream                = in;
    src->pub.bytes_in_buffer   = 0; /* forces fill_input_buffer on first read */
    src->pub.next_input_byte   = 0; /* until buffer loaded */
    }

  struct compress:jpeg_decompress_struct{
    compress(){
      }

    void start(){
      jpeg_create_decompress(this);
      }

    ~compress(){
      jpeg_destroy_decompress(this);
      }
    };

  bool load( IDevice &d,
             ImgInfo &info,
             std::vector<unsigned char> &out ) {
    compress cInfo;
    jpeg_error_mgr cErrMgr;

    cInfo.err = jpeg_std_error(&cErrMgr);
    cInfo.err->error_exit = handleLibJpegFatalError;
    cInfo.start();
    make_stream(&cInfo, &d);

    try{
      if( jpeg_read_header(&cInfo, TRUE)!=JPEG_HEADER_OK )
        return false;

      cInfo.out_color_space = JCS_RGB;

      if( !jpeg_start_decompress(&cInfo) )
        return 0;

      size_t iRowStride = cInfo.output_width * cInfo.output_components;
      //(*a_pBuffer) = new JSAMPLE[ iRowStride * cInfo.output_height ];

      out.resize( iRowStride * cInfo.output_height  );
      // Assign row buffers.
      //
      JSAMPROW pRows[1];

      // Decoding loop:
      //
      size_t i = 0;
      while (cInfo.output_scanline < cInfo.output_height)
      {
        // Decode it !
        pRows[0] = (JSAMPROW)( out.data() + iRowStride * i);  // set row buffer
        (void) jpeg_read_scanlines(&cInfo, pRows, 1);  // decode
        i++;
      }

      info.w   = cInfo.image_width;
      info.h   = cInfo.image_height;
      info.bpp = cInfo.output_components;

      if( !jpeg_finish_decompress(&cInfo) )
        Log() << "jpeg_finish_decompress error";
      }
    catch( const std::runtime_error & ){
      return false;
      }

    return true;
    }


  };

struct S3TCCodec:ImageCodec {
  bool load( IDevice &img,
             ImgInfo &info,
             std::vector<unsigned char> &out ) {
    DDSURFACEDESC2 ddsd;
    char dds4[4];
    if( img.readData(dds4, 4)!=4 )
      return false;

    if( strncmp( dds4, "DDS ", 4 ) != 0 ) {
      return false;
      }

    img.readData( (char*)&ddsd, sizeof(ddsd) );

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
    size_t bufferSize = info.w*info.h/factor;

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

    out.reserve( bufferSize );
    out.resize ( bufferSize );

    if( img.readData( (char*)&out[0], bufferSize)!=bufferSize )
      return false;

    info.alpha  = (info.format!=Pixmap::Format_DXT1);

    return true;
    }

  static bool dxtFrm( Pixmap::Format fout ){
    return fout==Pixmap::Format_DXT1 ||
           fout==Pixmap::Format_DXT3 ||
           fout==Pixmap::Format_DXT5;
    }

  bool canConvertTo( const ImgInfo &info, Pixmap::Format fout ) const{
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

  bool canConvertTo( const ImgInfo &info, Pixmap::Format fout ) const{
    return ( info.format==Pixmap::Format_RGB   && etcFrm(fout) ) ||
           ( info.format==Pixmap::Format_RGBA  && etcFrm(fout) ) ||
           ( etcFrm(info.format) && fout==Pixmap::Format_RGB );
    }

  bool canSave(const ImgInfo &inf) const{
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

  bool save( ODevice& file,
             const ImgInfo &info,
             const std::vector<unsigned char> &img){
    if( info.format!=Pixmap::Format_ETC1_RGB8 )
      return 0;

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

    file.writeData( (char*)&header, sizeof(header) );

    uint32_t imagesize=(info.w*info.h*halfbytes)/2;
    file.writeData( (char*)&imagesize, sizeof(imagesize) );
    file.writeData( (const char*)img.data(), img.size() );
    return 1;
    }

  bool load( IDevice &data,
             ImgInfo &info,
             std::vector<unsigned char> & out ) {
    KTX_header h;
    uint32_t tmp = 0;
    if( data.readData((char*)&h, sizeof(h)) != sizeof(h) )
      return 0;

    if( data.readData((char*)&tmp, sizeof(tmp))!=sizeof(tmp) )
      return 0;

    static const uint8_t ktx_identifier[12] = { 0xAB, 0x4B, 0x54, 0x58, 0x20, 0x31, 0x31, 0xBB, 0x0D, 0x0A, 0x1A, 0x0A };

    for(int i=0; i<=11; i++) {
      if( h.identifier[i]!=ktx_identifier[i] )
        return 0;
      }

    info.w      = h.pixelWidth;
    info.h      = h.pixelHeight;

    size_t pos = 0;
    uint32_t mips = std::max<uint32_t>(h.numberOfMipmapLevels, 1);

    int wx = info.w, hx = info.h;
    for( uint32_t i=0; i<mips; ++i ){
      pos += size_t((wx/4)*(hx/4)*8);

      if(wx>1) wx /= 2;
      if(hx>1) hx /= 2;
      }

    info.mipLevels = h.numberOfMipmapLevels;
    info.alpha     = false;
    info.format    = Pixmap::Format_ETC1_RGB8;
    info.bpp       = 3;

    out.resize( pos );
    if( data.readData( (char*)&out[0], pos )!=pos )
      return 0;

    return 1;
    }
  };

ImageCodec::~ImageCodec() {
  }

bool ImageCodec::canSave(const ImageCodec::ImgInfo &) const {
  return 0;
  }

bool ImageCodec::canConvertTo(const ImageCodec::ImgInfo &, Pixmap::Format ) const {
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

bool ImageCodec::load( IDevice &,
                       ImageCodec::ImgInfo &,
                       std::vector<unsigned char> &) {
  T_WARNING_X(0, "overload for ImageCodec::load is unimplemented");
  return false;
  }

bool ImageCodec::save( ODevice& ,
                       const ImageCodec::ImgInfo &,
                       const std::vector<unsigned char> & ) {
  T_WARNING_X(0, "overload for ImageCodec::save is unimplemented");
  return false;
  }

void ImageCodec::installStdCodecs(SystemAPI &s) {
  s.installImageCodec( new PngCodec()  );
  s.installImageCodec( new ETCCodec()  );
  s.installImageCodec( new S3TCCodec() );
  s.installImageCodec( new JpegCodec() );
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

void ImageCodec::resize( ImageCodec::ImgInfo &info,
                         std::vector<unsigned char> &rgb,
                         int w, int h ) {
  T_ASSERT( w<=info.w && h<=info.h );
  T_ASSERT( info.format==Pixmap::Format_RGB || info.format==Pixmap::Format_RGBA );

  const size_t bpp = info.bpp,
               iw  = info.w,
               ih  = info.h;

  if( w*h*bpp > rgb.size() )
    rgb.resize( w*h*bpp );

  unsigned char *p = &rgb[0];
  for( int r=0; r<h; ++r)
    for( int i=0; i<w; ++i ){
      //size_t  pix[4] = {};
      unsigned char *pix = &p[ (i + r*w)*bpp ];
      size_t  x0 = (i*iw)/w,
              y0 = (r*ih)/h;

      for( size_t q=0; q<bpp; ++q ){
        pix[q] = p[ (x0 + y0*iw)*bpp + q ];
        }
      }

  rgb.resize( w*h*bpp );
  info.w = w;
  info.h = h;
  }

void ImageCodec::downsample( ImageCodec::ImgInfo &info,
                             std::vector<unsigned char> &rgb ) {
  //T_ASSERT( info.w%2==0 && info.h%2==0 );
  T_ASSERT( info.format==Pixmap::Format_RGB || info.format==Pixmap::Format_RGBA );

  int bpp = info.bpp,
      iw = info.w,
      w  = std::max(info.w/2, 1),
      h  = std::max(info.h/2, 1);

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

  rgb.resize( w*h*bpp );
  info.w = w;
  info.h = h;
  }

}
