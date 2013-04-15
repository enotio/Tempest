#ifndef WINDOWSAPI_H
#define WINDOWSAPI_H

#include <Tempest/AbstractSystemAPI>

namespace Tempest{

class Window;

#ifdef __WIN32__
class WindowsAPI:public AbstractSystemAPI {
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
    std::vector<char> loadBytesImpl( const char* file );

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

  friend class AbstractSystemAPI;
  };
#endif

}

#endif // WINDOWSAPI_H
