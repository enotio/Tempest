#ifndef SCROLLWIDGET_H
#define SCROLLWIDGET_H

#include <Tempest/Widget>
#include <Tempest/ScrollBar>
#include <Tempest/ListView>

namespace Tempest {

class ListView;

class ScrollWidget : public Tempest::Widget {
  public:
    ScrollWidget();
    ~ScrollWidget();

    enum ScroolViewMode{
      AlwaysOff,
      AsNeed,
      AlwaysOn
      };

    Widget& centralWidget();
    bool    isListBased() const;

    template<class D>
    void setDelegate(const D& d){
      initializeList();
      list->setDelegate(d);
      helper.layout().applyLayout();
      resizeEv( w(), h() );
      }

    ListView* asListView();
    const ListView* asListView() const;
    void removeList();

    void setLayout(Tempest::Orientation ori);

    void hideScroolBars();
    void setScroolBarsVisible( bool h, bool v );
    void setVScroolViewMode( ScroolViewMode );
    void setHScroolViewMode( ScroolViewMode );

    void scroolAfterEndH( bool s );
    bool hasScroolAfterEndH() const;

    void scroolBeforeBeginH( bool s );
    bool hasScroolBeforeBeginH() const;

    void scroolAfterEndV( bool s );
    bool hasScroolAfterEndV() const;

    void scroolBeforeBeginV( bool s );
    bool hasScroolBeforeBeginV() const;

    void scroolH( int v );
    void scroolV( int v );

  protected:
    ScrollWidget(bool noUi);

    void mouseWheelEvent(Tempest::MouseEvent &e);
    void mouseMoveEvent(Tempest::MouseEvent &e);

    void gestureEvent(Tempest::AbstractGestureEvent &e);
    void resizeEvent(Tempest::SizeEvent& e);

    virtual ScrollBar* createScrollBar(Tempest::Orientation ori);
    virtual void setScrollBars(Tempest::ScrollBar* hor,
                               Tempest::ScrollBar* vert , bool deleteOld);

  private:
    Widget     helper;
    ScrollBar* sbH;
    ScrollBar* sbV;
    ScroolViewMode vert, hor;

    Widget*    cen;
    ListView*  list = nullptr;

    Tempest::Orientation orient = Tempest::Vertical;
    void initializeList();
    void updateScrools();

    struct Box:Widget {
      ScrollWidget* owner;
      } box;

    struct ProxyLayout;
    ProxyLayout *mlay;

    struct BoxLayout;
    struct HelperLayout;

    void resizeEv(int w, int h);
    static Tempest::Size sizeHint( const Widget *w );

    using Tempest::Widget::layout;
  };

  }

#endif // SCROLLWIDGET_H
