#ifndef SCROOLWIDGET_H
#define SCROOLWIDGET_H

#include <Tempest/Widget>
#include <Tempest/ScroolBar>

namespace Tempest {

class ScroolWidget : public Tempest::Widget {
  public:
    ScroolWidget();

    enum ScroolViewMode{
      AlwaysOn,
      AsNeed,
      AlwaysOff
      };

    Widget& centralWidget();

    void hideScroolBars();
    void setScroolBarsVisible( bool h, bool v );
    void setVScroolViewMode( ScroolViewMode );
    void setHScroolViewMode( ScroolViewMode );

    void scroolAfterEnd( bool s );
    bool hasScroolAfterEnd() const;

    void scroolBeforeBegin( bool s );
    bool hasScroolBeforeBegin() const;

    void scroolH( int v );
    void scroolV( int v );

  protected:
    void mouseWheelEvent(Tempest::MouseEvent &e);
    void mouseMoveEvent(Tempest::MouseEvent &e);

    void gestureEvent(Tempest::AbstractGestureEvent &e);
    void resizeEvent(Tempest::SizeEvent& e);

  private:
    Widget    helper;
    Widget    cen;
    ScroolBar sbH, sbV;
    ScroolViewMode vert, hor;

    struct Box:Widget {
      ScroolWidget* owner;
      } box;

    struct ProxyLayout;
    ProxyLayout *mlay;

    struct BoxLayout;

    void resizeEv(int w, int h);
    static Tempest::Size sizeHint( const Widget *w );

    using Tempest::Widget::layout;
  };

  }

#endif // SCROOLWIDGET_H
