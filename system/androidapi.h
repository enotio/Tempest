#ifndef ANDROIDAPI_H
#define ANDROIDAPI_H

#include <Tempest/SystemAPI>

namespace Tempest{

class Window;

#ifdef __ANDROID__
class AndroidAPI:public SystemAPI {
  protected:
    AndroidAPI();
    ~AndroidAPI();

    static bool startRender( Window* );
    static bool present( Window* );

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

    bool isGraphicsContextAviable( Tempest::Window *w );
  public:
    friend class SystemAPI;
    friend class Opengl2x;

    void * android;
  };
#endif

}

#endif // ANDROIDAPI_H
