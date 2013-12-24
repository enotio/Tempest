#ifndef IMAGECODEC_H
#define IMAGECODEC_H

#include <vector>
#include <Tempest/Pixmap>

namespace Tempest{

class SystemAPI;
class IDevice;
class ODevice;

class ImageCodec {
  public:
    virtual ~ImageCodec();

    typedef Pixmap::ImgInfo ImgInfo;

    virtual bool canSave(ImgInfo &) const;  
    virtual bool canConvertTo(ImgInfo &, Pixmap::Format fout ) const;

    virtual void toRGB( ImgInfo &info,
                        std::vector<unsigned char> &inout,
                        bool alpha );

    virtual void fromRGB( ImgInfo &info,
                          std::vector<unsigned char> &inout );

    virtual bool load( const char *file,
                       ImgInfo &info,
                       std::vector<unsigned char> &out );
    virtual bool load( const char16_t *file,
                       ImgInfo &info,
                       std::vector<unsigned char> &out );
    virtual bool load( IDevice& imgBytes,
                       ImgInfo &info,
                       std::vector<unsigned char> &out );

    virtual bool save( const char *file,
                       ImgInfo &info,
                       std::vector<unsigned char> &img );

    virtual bool save( const char16_t *file,
                       ImgInfo &info,
                       std::vector<unsigned char> &img );

    virtual bool save( ODevice &file,
                       ImgInfo &info,
                       std::vector<unsigned char> &img );

    static void installStdCodecs( SystemAPI& s );

  //protected:
    static void addAlpha   (ImgInfo &info, std::vector<unsigned char>& rgb );
    static void removeAlpha(ImgInfo &info, std::vector<unsigned char>& rgba );

    static void resize( ImgInfo &info, std::vector<unsigned char>& rgb,
                        int nw, int nh );
    static void downsample(ImgInfo &info, std::vector<unsigned char>& rgb );
  };

}

#endif // IMAGECODEC_H
