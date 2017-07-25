#ifndef APPLICATION_H
#define APPLICATION_H

#include <string>
#include <vector>
#include <cstdint>
#include <Tempest/Platform>
#include <Tempest/Font>
#include <Tempest/UIMetrics>
#include <Tempest/Style>
#include <Tempest/signal>

#ifdef __WINDOWS_PHONE__
#define main Tempest_main
#elif defined(_MSC_VER) && !defined(TEMPEST_LIBRARY_BUILD)
#pragma comment(linker, "/subsystem:windows /ENTRY:mainCRTStartup")
#endif

namespace Tempest{

class Timer;
/**
 * \addtogroup Core
 *  @{
 */

/*!
 * \brief The Application class manages the GUI application's control flow and main settings.
 */
class Application {
  public:
    Application();
    ~Application();

    static signal<const std::u16string,const Rect&> showHint;

    static Font mainFont();
    static void setMainFont(const Font& f);
    static signal<Font> fontChanged;

    static const UiMetrics&  uiMetrics();
    static void              setUiMetrics(const UiMetrics& u);
    static signal<UiMetrics> uiMetricsChanged;

    static void  setStyle(const Tempest::Style* stl);
    static const Tempest::Style& style();

    int exec();
    static bool isQuit();
    static bool processEvents(bool all = true);
    static void sleep(unsigned int msec );

    static uint64_t tickCount();
    static void exit();

  private:
    static struct App{
      int                 ret;
      bool                quit;

      Font                mainFont;
      UiMetrics           uiMetrics;
      std::vector<Timer*> timer;

      std::unique_ptr<Tempest::Style> style;
      } app;

    struct Style;

    static void processTimers();
    static void* execImpl(void*);

  friend class Timer;
  };
/** @}*/

}
#endif // APPLICATION_H
