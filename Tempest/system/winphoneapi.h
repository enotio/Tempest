#ifndef WINPHONEAPI_H
#define WINPHONEAPI_H

#include <Tempest/SystemAPI>

int WinMain();

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

    struct  WinFile;
    File*  fopenImpl ( const char* fname, const char* mode );
    File*  fopenImpl ( const char16_t* fname, const char* mode );
    size_t readDataImpl ( File* f, char* dest, size_t count );
    size_t writeDataImpl( File* f, const char* data, size_t count );
    void   flushImpl    ( File* f );
    size_t peekImpl     ( File* f, size_t skip, char* dest, size_t count );
    size_t skipImpl     ( File* f, size_t count );
    bool   eofImpl      ( File* f );
    size_t fsizeImpl    ( File* f );
    void   fcloseImpl   ( File* f );
  private:
    struct Wnd;
    static int preMain( int( _cdecl* )(int,const char**) );

  friend class SystemAPI;
  friend int ::WinMain();
  };
#endif

}

#endif // WINPHONEAPI_H
