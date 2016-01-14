#ifndef IOSAPI_H
#define IOSAPI_H

#include <Tempest/Platform>

#ifdef __IOS__
#include <Tempest/SystemAPI>
#include <csetjmp>

namespace Tempest{

class Window;

class iOSAPI:public SystemAPI {
  public:
    static void* initializeOpengl(void* window);
    static bool  glMakeCurrent(void* ctx);
    static bool  glUpdateContext(void* ctx,void* window);
    static void  glSwapBuffers(void* ctx);
    static void  swapContext();
    static void  finish();

    struct Fiber;
    struct FiberCtx;
    struct PBox;

  protected:
    iOSAPI();
    ~iOSAPI();

    bool testDisplaySettings( Window* w, const DisplaySettings& );
    bool setDisplaySettings( Window* w, const DisplaySettings& );
    Size implScreenSize();

    void startApplication( ApplicationInitArgs* );
    void endApplication();
    int  nextEvent(bool &quit);
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

  private:
    struct Wnd;
    static bool processEvent();

  friend class SystemAPI;
  };

}

#endif
#endif // IOSAPI_H
