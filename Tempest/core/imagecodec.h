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

    virtual bool canSave(const ImgInfo &, const char* ext) const;
    virtual bool canConvertTo(const ImgInfo &, Pixmap::Format fout ) const;

    virtual void toRGB( ImgInfo &info,
                        std::vector<uint8_t> &inout,
                        bool alpha );

    virtual void fromRGB( ImgInfo &info,
                          std::vector<uint8_t> &inout );

    virtual bool load( IDevice& imgBytes,
                       ImgInfo &info,
                       std::vector<uint8_t> &out );

    virtual bool save( ODevice &file,
                       const ImgInfo &info,
                       const std::vector<uint8_t> &img );

    static void installStdCodecs( SystemAPI& s );

  //protected:
    static void addAlpha   (ImgInfo &info, std::vector<uint8_t>& rgb );
    static void removeAlpha(ImgInfo &info, std::vector<uint8_t>& rgba );
    static void resize     (ImgInfo &info, std::vector<uint8_t> &rgb,int nw, int nh );
    static void downsample (ImgInfo &info, std::vector<uint8_t>& rgb );
  };

}

#endif // IMAGECODEC_H
