#ifndef APPLICATION_H
#define APPLICATION_H

#include <string>

namespace Tempest{
  
class Application {
  public:
    Application();
    ~Application();

    int exec();
    //static bool processMessage();
  };

}
#endif // APPLICATION_H
