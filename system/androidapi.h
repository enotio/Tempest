#ifndef ANDROIDAPI_H
#define ANDROIDAPI_H

#include <Tempest/AbstractSystemAPI>

namespace Tempest{

class Window;

#ifdef __ANDROID__
class AndroidAPI:public AbstractSystemAPI {
  protected:
    AndroidAPI();
    ~AndroidAPI();

    void startApplication( ApplicationInitArgs* );
    void endApplication();
    int  nextEvent(bool &qiut);

    Window* createWindow(int w, int h);
    void deleteWindow( Window* );
    void show( Window* );
    void setGeometry( Window*, int x, int y , int w, int h );
    void bind(Window*, Tempest::Window * );

  public:
  friend class AbstractSystemAPI;
  };
#endif

}

#endif // ANDROIDAPI_H
