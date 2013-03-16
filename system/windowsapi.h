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
    void deleteWindow( Window* );
    void show( Window* );
    void setGeometry( Window*, int x, int y , int w, int h );
    void bind(Window*, Tempest::Window * );

    std::string loadTextImpl( const char* file );
  private:
    struct Wnd;

  friend class AbstractSystemAPI;
  };
#endif

}

#endif // WINDOWSAPI_H
