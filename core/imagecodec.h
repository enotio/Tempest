#ifndef IMAGECODEC_H
#define IMAGECODEC_H

#include <vector>
#include <Tempest/Pixmap>

namespace Tempest{

class SystemAPI;

class ImageCodec {
  public:
    virtual ~ImageCodec();

    typedef Pixmap::ImgInfo ImgInfo;

    virtual bool canSave(ImgInfo &) const;

    virtual bool load( const char *file,
                       ImgInfo &info,
                       std::vector<unsigned char> &out );
    virtual bool load( const wchar_t *file,
                       ImgInfo &info,
                       std::vector<unsigned char> &out );
    virtual bool load( const std::vector<char>& imgBytes,
                       ImgInfo &info,
                       std::vector<unsigned char> &out );

    virtual bool save( const char *file,
                       ImgInfo &info,
                       std::vector<unsigned char> &img );

    virtual bool save( const wchar_t *file,
                       ImgInfo &info,
                       std::vector<unsigned char> &img );

    static void installStdCodecs( SystemAPI& s );
  };

}

#endif // IMAGECODEC_H
