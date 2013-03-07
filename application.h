#ifndef APPLICATION_H
#define APPLICATION_H

#ifdef __ANDROID__

#ifndef SDL_main
#define main	SDL_main
extern "C" int SDL_main(int argc, char *argv[]);
#endif

#endif

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
