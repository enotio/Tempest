#ifndef WINDOW_H
#define WINDOW_H

#include <Tempest/AbstractSystemAPI>
#include <Tempest/Widget>

namespace Tempest{

class Window : public Widget {
  public:
    Window();
    virtual ~Window();

    void show();

    virtual void setPosition( int x, int y );
    virtual void resize(int w, int h );

    virtual void render(){}

  protected:
    AbstractSystemAPI::Window *handle();

  private:
    AbstractSystemAPI::Window *wnd;
    int pressedC;

    Window( const Window& ){}
    const Window& operator = ( const Window& ){ return *this; }

  friend class AbstractSystemAPI;
  };

}

#endif // WINDOW_H
