#ifndef APPLICATION_H
#define APPLICATION_H

#include <string>
#include <vector>
#include <cstdint>
#include <Tempest/Platform>

#ifdef __WINDOWS_PHONE__
#define main Tempest_main
#endif

namespace Tempest{

class Timer;
/**
 * \addtogroup Core
 */
class Application {
  public:
    Application();
    ~Application();

    int exec();
    static bool isQuit();
    static bool processEvents(bool all = true);
    static void sleep(unsigned int msec );

    static uint64_t tickCount();
    static void exit();
  private:
    static struct App{
      int  ret;
      bool quit;

      std::vector<Timer*> timer;
      } app;

    static void processTimers();
    static void* execImpl(void*);

    friend class Timer;
    //static bool processMessage();
  };

}
#endif // APPLICATION_H
