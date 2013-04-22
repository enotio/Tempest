#ifndef WINDOW_H
#define WINDOW_H

#include <Tempest/SystemAPI>
#include <Tempest/Widget>

namespace Tempest{

class Window : public Widget {
  public:
    Window();
    Window( int w, int h );

    enum ShowMode{
      Normal,
      Minimized,
      Maximized,
      FullScreen
      };
    Window( ShowMode sm );

    virtual ~Window();

    void show();

    virtual void setPosition( int x, int y );
    virtual void resize(int w, int h );
    using Widget::resize;

    virtual void render(){}

    bool isFullScreenMode() const;
    ShowMode showMode() const;
  protected:
    SystemAPI::Window *handle();

  private:
    SystemAPI::Window *wnd;
    int  pressedC;
    bool resizeIntent, isAppActive;

    ShowMode smode;
    int winW, winH;
    void init(int w, int h);

    Window( const Window& )                    = delete;
    const Window& operator = ( const Window& ) = delete;

  friend class SystemAPI;
  };

}

#endif // WINDOW_H
