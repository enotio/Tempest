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
    virtual bool canConvertTo(ImgInfo &, Pixmap::Format fout ) const;

    virtual void toRGB( ImgInfo &info,
                        std::vector<unsigned char> &inout,
                        bool alpha );

    virtual void fromRGB( ImgInfo &info,
                          std::vector<unsigned char> &inout );

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

  protected:
    void addAlpha   (ImgInfo &info, std::vector<unsigned char>& rgb );
    void removeAlpha(ImgInfo &info, std::vector<unsigned char>& rgba );
  };

}

#endif // IMAGECODEC_H
