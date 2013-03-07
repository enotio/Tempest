#include "androidapi.h"

#ifdef __ANDROID__

#include "STLConfig.h"

#include <Tempest/Window>
#include <Tempest/Event>
#include <map>

#include <SDL.h>
#include <GLES/gl.h>

static SDL_Window* window;
static SDL_GLContext context;
static SDL_Event event;
static Tempest::Window * wnd = 0;

static int window_w = 640, window_h = 480;

using namespace Tempest;

AndroidAPI::AndroidAPI() {
  }

AndroidAPI::~AndroidAPI() {
  }

void AndroidAPI::startApplication( ApplicationInitArgs * ) {
  if(SDL_Init(SDL_INIT_VIDEO) < 0)
    return;

  window = SDL_CreateWindow("OpenGL Test",
                            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                            640, 480, SDL_WINDOW_OPENGL|SDL_WINDOW_SHOWN);
  SDL_GetWindowSize(window, &window_w, &window_h);

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

  context = SDL_GL_CreateContext(window);
  SDL_GL_MakeCurrent(window, context);

  SDL_GL_SetSwapInterval(1); //vsync

  //glViewport(0, 0, window_w, window_h);
  }

void AndroidAPI::endApplication() {
  SDL_GL_MakeCurrent(NULL, NULL);
  SDL_GL_DeleteContext(context);
  SDL_DestroyWindow(window);

  SDL_Quit();
  }

int AndroidAPI::nextEvent(bool &quit) {
  while (SDL_PollEvent(&event))
    if(event.type == SDL_QUIT){
      quit = true;
      return 0;
      }

  wnd->render();
  SDL_GL_SwapWindow(window);
  SDL_Delay( 0 );
  return 0;
  }

AndroidAPI::Window *AndroidAPI::createWindow(int w, int h) {
  return 0;
  }

void AndroidAPI::deleteWindow( Window *w ) {
  }

void AndroidAPI::show(Window *) {
  wnd->resize(window_w, window_h);
  }

void AndroidAPI::setGeometry( Window *hw, int x, int y, int w, int h ) {
  }

void AndroidAPI::bind( Window *w, Tempest::Window *wx ) {
  wnd = wx;
  }

#endif
