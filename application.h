#ifndef APPLICATION_H
#define APPLICATION_H

#include <string>

namespace Tempest{
  
class Application {
  public:
    Application();
    ~Application();

    int exec();
    static bool processEvents();
    static void sleep(unsigned int msec );
  private:
    static struct App{
      int  ret;
      bool quit;
      } app;
    //static bool processMessage();
  };

}
#endif // APPLICATION_H
