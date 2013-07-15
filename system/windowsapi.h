#ifndef WINDOWSAPI_H
#define WINDOWSAPI_H

#include <Tempest/SystemAPI>

namespace Tempest{

class Window;

#ifdef __WIN32__
class WindowsAPI:public SystemAPI {
  protected:
    WindowsAPI();
    ~WindowsAPI();

    void startApplication( ApplicationInitArgs* );
    void endApplication();
    int  nextEvent(bool &qiut);

    Window* createWindow(int w, int h);
    Window* createWindowMaximized();
    Window* createWindowMinimized();
    Window* createWindowFullScr();

    Size windowClientRect( Window* );

    void deleteWindow( Window* );
    void show( Window* );
    void setGeometry( Window*, int x, int y , int w, int h );
    void bind(Window*, Tempest::Window * );

    std::string loadTextImpl( const char* file );
    std::string loadTextImpl( const wchar_t* file );

    std::vector<char> loadBytesImpl( const char* file );
    std::vector<char> loadBytesImpl( const wchar_t* file );

    bool loadImageImpl( const char* file,
                        int &w,
                        int &h,
                        int &bpp,
                        std::vector<unsigned char>& out );

    bool saveImageImpl( const char* file,
                        int &w,
                        int &h,
                        int &bpp,
                        std::vector<unsigned char>& in );

    bool loadImageImpl( const wchar_t* file,
                        int &w,
                        int &h,
                        int &bpp,
                        std::vector<unsigned char>& out );
    bool saveImageImpl( const wchar_t* file,
                        int &w,
                        int &h,
                        int &bpp,
                        std::vector<unsigned char>& out );
  private:
    struct Wnd;

    template< class ChanelType, int  >
    void initRawData( std::vector<unsigned char> &d,
                      void * input,
                      int pixSize,
                      int w,
                      int h,
                      int * ix );

    static void initImgLib();

  friend class SystemAPI;
  };
#endif

}

#endif // WINDOWSAPI_H
