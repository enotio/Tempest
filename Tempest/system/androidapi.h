#ifndef ANDROIDAPI_H
#define ANDROIDAPI_H

#include <Tempest/SystemAPI>

#ifdef __ANDROID__
#include <jni.h>

namespace Tempest{

class Window;
class WindowOverlay;

class AndroidAPI:public SystemAPI {
  public:
    static JavaVM *jvm();
    static JNIEnv *jenvi();
    static jclass appClass();
    static jobject activity();

    static const char* internalStorage();
    static const char* externalStorage();

    static int densityDpi();

    static void toast( const std::string & s );
    static void showSoftInput();
    static void hideSoftInput();
    static void toggleSoftInput();
    static const std::string& iso3Locale();
  protected:
    AndroidAPI();
    ~AndroidAPI();

    bool testDisplaySettings( Window* w, const DisplaySettings& );
    bool setDisplaySettings( Window* w, const DisplaySettings& );
    Size implScreenSize();

    static bool startRender( Window* );
    static bool present( Window* );

    void startApplication( ApplicationInitArgs* );
    void endApplication();
    int  nextEvent(bool &qiut);
    int  nextEvents(bool &qiut);

    Window* createWindow(int w, int h);
    Window* createWindowMaximized();
    Window* createWindowMinimized();
    Window* createWindowFullScr();

    Widget* addOverlay( WindowOverlay *ov );

    Point windowClientPos ( Window* );
    Size  windowClientRect( Window* );

    void deleteWindow( Window* );
    void show( Window* );
    void setGeometry( Window*, int x, int y, int w, int h );
    void bind(Window*, Tempest::Window * );

    GraphicsContexState isGraphicsContextAvailable( Tempest::Window *w );
    CpuInfo cpuInfoImpl();

    struct  DroidFile;
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
  public:
    friend class SystemAPI;
    friend class Opengl2x;
  };

}

#endif

#endif // ANDROIDAPI_H
