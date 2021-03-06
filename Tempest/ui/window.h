#ifndef WINDOW_H
#define WINDOW_H

#include <Tempest/SystemAPI>
#include <Tempest/Widget>

#include <Tempest/GestureRecognizer>

#include <memory>

namespace Tempest{

class SurfaceRender;

class WindowOverlay:public Widget{
  public:
    WindowOverlay();
    ~WindowOverlay();

    Window* window() const;
  private:
    Window * owner;

  friend class Window;
  };

class Window : public Widget {
  public:
    Window();
    Window( int w, int h );

    enum ShowMode{
      Minimized,
      Normal,
      Maximized,
      FullScreen
      };
    Window( ShowMode sm );

    virtual ~Window();

    void show();

    virtual void setPosition( int x, int y );
    virtual void resize(int w, int h );

    using Widget::setPosition;
    using Widget::resize;

    virtual void render(){}

    size_t overlayCount() const;
    WindowOverlay& overlay(size_t i );
    const WindowOverlay& overlay(size_t i ) const;

    bool needToUpdate() const;

    bool isFullScreenMode() const;
    ShowMode showMode() const;

    template< class G >
    void instalGestureRecognizer( G *g ){
      removeGestureRecognizer<G>();
      recognizers.push_back( std::unique_ptr<G>( g ) );
      }

    template< class  G >
    void removeGestureRecognizer(){
      for( size_t i=0; i<recognizers.size(); ++i ){
        GestureRecognizer& g=*recognizers[i].get();
        if( typeid(g)==typeid(G) ){
          recognizers[i] = std::move(recognizers.back());
          recognizers.pop_back();
          return;
          }
        }
      }

    bool isActive() const;

  protected:
    SystemAPI::Window *handle();

    Tempest::signal<ShowMode> showModeChanged;

  private:
    SystemAPI::Window *wnd;
    std::vector<int>   pressedC;
    bool resizeIntent, isAppActive;

    void addOverlay( WindowOverlay* w );
    void removeOverlay( WindowOverlay* w );
    std::vector<WindowOverlay*> overlaywd;

    AbstractGestureEvent* sendEventToGestureRecognizer(const Event &e);

    std::vector< std::unique_ptr<GestureRecognizer>> recognizers;

    void setShowMode( ShowMode m );
    ShowMode smode;
    int winW, winH;
    void init(int w, int h);

    Window( const Window& )                    = delete;
    const Window& operator = ( const Window& ) = delete;

    struct DragGestureRecognizer;
  friend class WindowOverlay;
  friend class SystemAPI;
  };

}

#endif // WINDOW_H
