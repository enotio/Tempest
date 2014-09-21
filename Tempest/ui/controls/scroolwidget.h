#ifndef SCROOLWIDGET_H
#define SCROOLWIDGET_H

#include <Tempest/Widget>
#include <Tempest/ScroolBar>

namespace Tempest {

class ScroolWidget : public Tempest::Widget {
  public:
    ScroolWidget();

    Widget& centralWidget();

    void hideScroolBar();
    void setScroolBarVisible( bool v );
    void setOrientation( Tempest::Orientation ori );
    Tempest::Orientation orientation() const;

    void scroolAfterEnd( bool s );
    bool hasScroolAfterEnd() const;

    void scroolBeforeBegin( bool s );
    bool hasScroolBeforeBegin() const;

    void scrool( int v );
  protected:
    void mouseWheelEvent(Tempest::MouseEvent &e);
    void mouseMoveEvent(Tempest::MouseEvent &e);

    void gestureEvent(Tempest::AbstractGestureEvent &e);
    void resizeEvent(Tempest::SizeEvent& e);

  private:
    ScroolBar sb;
    struct Box:Widget {
      Box();
      void paintEvent( Tempest::PaintEvent& e );

      ScroolWidget* owner;
      } box;

    Widget *cen;

    struct ProxyLayout;
    ProxyLayout *mlay;

    void resizeEv(int w, int h);
    static Tempest::Size sizeHint( const Widget *w );

    using Tempest::Widget::layout;
  };

  }

#endif // SCROOLWIDGET_H
