#ifndef WINPHONEAPI_H
#define WINPHONEAPI_H

#include <Tempest/SystemAPI>

#ifdef __WINDOWS_PHONE__
int __stdcall WinMain();
#endif

namespace Tempest{

class Window;
#ifdef __WINDOWS_PHONE__
class WinPhoneAPI:public SystemAPI {
  protected:
    WinPhoneAPI();
    ~WinPhoneAPI();

    bool testDisplaySettings( Window* w, const DisplaySettings& );
    bool setDisplaySettings( Window* w, const DisplaySettings& );
    Size implScreenSize();

    void startApplication( ApplicationInitArgs* );
    void endApplication();
    int  nextEvent(bool &qiut);
    int  nextEvents(bool &quit);

    Window* createWindow(int w, int h);
    Window* createWindowMaximized();
    Window* createWindowMinimized();
    Window* createWindowFullScr();

    Widget *addOverlay(WindowOverlay *ov );

    Point windowClientPos ( Window* );
    Size  windowClientRect( Window* );

    void deleteWindow( Window* );
    void show( Window* );
    void setGeometry( Window*, int x, int y , int w, int h );
    void bind(Window*, Tempest::Window * );

    CpuInfo cpuInfoImpl();

    File*  fopenImpl ( const char* fname, const char* mode );
    File*  fopenImpl ( const char16_t* fname, const char* mode );
  private:
    struct Wnd;
    static int preMain( int( _cdecl* )(int,const char**) );

  friend class SystemAPI;
  friend int ::WinMain();
  };
#endif

}

#endif // WINPHONEAPI_H
