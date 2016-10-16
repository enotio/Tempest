#ifndef WINDOWSAPI_H
#define WINDOWSAPI_H

#include <Tempest/SystemAPI>

namespace Tempest{

class Window;

#ifdef __WINDOWS__
class WindowsAPI:public SystemAPI {
  public:
    static void setCursor( Tempest::Window& w,
                           const Pixmap& pinput,
                           int hotSpotX,
                           int hotSpotY );
    static std::string iso3Locale();
    void clearPressedImpl();

  protected:
    WindowsAPI();
    ~WindowsAPI();

    bool testDisplaySettings( Window* w, const DisplaySettings& );
    bool setDisplaySettings ( Window* w, const DisplaySettings& );
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

  friend class SystemAPI;
  };
#endif

}

#endif // WINDOWSAPI_H
