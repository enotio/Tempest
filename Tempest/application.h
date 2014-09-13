#ifndef APPLICATION_H
#define APPLICATION_H

#include <string>
#include <vector>
#include <cstdint>
#include <Tempest/Platform>
#include <Tempest/Font>
#include <Tempest/UIMetrics>

#ifdef __WINDOWS_PHONE__
#define main Tempest_main
#endif

namespace Tempest{

class Font;
class Timer;
/**
 * \addtogroup Core
 */
class Application {
  public:
    Application();
    ~Application();

    static signal<const std::u16string,const Rect&> showHint;

    static Font mainFont();
    static void setMainFont(const Font& f);
    static signal<Font> fontChanged;

    static UIMetrics& uiMetrics();
    static void       setUiMetrics(const UIMetrics& u);
    static signal<UIMetrics> uiMetricsChanged;

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

      Font mainFont;
      UIMetrics uiMetrics;

      std::vector<Timer*> timer;
      } app;

    static void processTimers();
    static void* execImpl(void*);

    friend class Timer;
    //static bool processMessage();
  };

}
#endif // APPLICATION_H
