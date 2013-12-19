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

    bool testDisplaySettings( const DisplaySettings& );
    bool setDisplaySettings( const DisplaySettings& );
    Size implScreenSize();

    void startApplication( ApplicationInitArgs* );
    void endApplication();
    int  nextEvent(bool &qiut);
    int  nextEvents(bool &quit);

    Window* createWindow(int w, int h);
    Window* createWindowMaximized();
    Window* createWindowMinimized();
    Window* createWindowFullScr();

    Point windowClientPos ( Window* );
    Size  windowClientRect( Window* );

    void deleteWindow( Window* );
    void show( Window* );
    void setGeometry( Window*, int x, int y , int w, int h );
    void bind(Window*, Tempest::Window * );

    std::string loadTextImpl( const char* file );
    std::string loadTextImpl( const wchar_t* file );

    std::vector<char> loadBytesImpl( const char* file );
    std::vector<char> loadBytesImpl( const wchar_t* file );

    bool writeBytesImpl(const wchar_t* file , const std::vector<char> &f);

    CpuInfo cpuInfoImpl();
  private:
    struct Wnd;

  friend class SystemAPI;
  };
#endif

}

#endif // WINDOWSAPI_H
