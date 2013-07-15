#ifndef ABSTRACTSYSTEMAPI_H
#define ABSTRACTSYSTEMAPI_H

#include <string>
#include <vector>

#include <Tempest/Utility>

namespace Tempest{

class Window;
class MouseEvent;
class SizeEvent;

class SystemAPI{
  public:
    virtual ~SystemAPI(){}

    struct Window;
    virtual Window* createWindow( int w, int h ) = 0;
    virtual Window* createWindowMaximized() = 0;
    virtual Window* createWindowMinimized() = 0;
    virtual Window* createWindowFullScr()   = 0;

    virtual Size    windowClientRect( Window* )   = 0;

    virtual void deleteWindow( Window* ) = 0;
    virtual void setGeometry( Window*, int x, int y, int w, int h ) = 0;
    virtual void show( Window* ) = 0;
    virtual void bind( Window*, Tempest::Window* ) = 0;

    static SystemAPI& instance();

    struct ApplicationInitArgs;
    virtual void startApplication( ApplicationInitArgs* ) = 0;
    virtual void endApplication() = 0;
    virtual int  nextEvent(bool &qiut) = 0;

    static std::string loadText( const std::string& file );
    static std::string loadText( const std::wstring& file );

    static std::string loadText( const char* file );
    static std::string loadText( const wchar_t* file );

    static std::vector<char> loadBytes( const char* file );

    static bool loadImage( const wchar_t* file,
                           int &w,
                           int &h,
                           int &bpp,
                           std::vector<unsigned char>& out );
    static bool saveImage( const wchar_t* file,
                           int &w,
                           int &h,
                           int &bpp,
                           std::vector<unsigned char>& in );

    static bool loadImage( const char* file,
                           int &w,
                           int &h,
                           int &bpp,
                           std::vector<unsigned char>& out );
    static bool saveImage( const char* file,
                           int &w,
                           int &h,
                           int &bpp,
                           std::vector<unsigned char>& in );

    static void mkMouseEvent( Tempest::Window *w,
                              MouseEvent& e,
                              int type );

    static void sizeEvent( Tempest::Window *w,
                           int winW , int winH, int cW, int cH);
    static void activateEvent( Tempest::Window*w, bool a );

    virtual bool isGraphicsContextAviable( Tempest::Window *w );
  protected:
    SystemAPI(){}

    virtual std::string       loadTextImpl( const char* file    ) = 0;
    virtual std::string       loadTextImpl( const wchar_t* file ) = 0;

    virtual std::vector<char> loadBytesImpl( const char* file ) = 0;

    virtual bool loadImageImpl( const char* file,
                                int &w,
                                int &h,
                                int &bpp,
                                std::vector<unsigned char>& out ) = 0;
    virtual bool saveImageImpl( const char* file,
                                int &w,
                                int &h,
                                int &bpp,
                                std::vector<unsigned char>& out ) = 0;

    virtual bool loadImageImpl( const wchar_t* file,
                                int &w,
                                int &h,
                                int &bpp,
                                std::vector<unsigned char>& out ) = 0;
    virtual bool saveImageImpl( const wchar_t* file,
                                int &w,
                                int &h,
                                int &bpp,
                                std::vector<unsigned char>& out ) = 0;

    virtual bool loadS3TCImpl(const std::vector<char> &data,
                               int &w,
                               int &h,
                               int &bpp,
                               std::vector<unsigned char> &out );

    virtual bool loadPngImpl(const std::vector<char> &data,
                             int &w,
                             int &h,
                             int &bpp,
                             std::vector<unsigned char> &out );
  private:
    SystemAPI( const SystemAPI& ){}
    SystemAPI& operator = ( const SystemAPI&){return *this;}
  };

}

#endif // ABSTRACTSYSTEMAPI_H
