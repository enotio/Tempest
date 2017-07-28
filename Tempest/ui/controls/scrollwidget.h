#ifndef SCROLLWIDGET_H
#define SCROLLWIDGET_H

#include <Tempest/Widget>
#include <Tempest/ScrollBar>
#include <Tempest/ListView>

namespace Tempest {

class ListView;

/** \addtogroup GUI
 *  @{
 */
class ScrollWidget : public Tempest::Widget {
  public:
    ScrollWidget();
    ~ScrollWidget();

    enum scrollViewMode{
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
      layout().applyLayout();
      }

          ListView* asListView();
    const ListView* asListView() const;
    void removeList();

    void setLayout(Tempest::Orientation ori);
    void setLayout(Layout* l);

    void hideScrollBars();
    void setScrollBarsVisible( bool h, bool v );
    void setVscrollViewMode( scrollViewMode );
    void setHscrollViewMode( scrollViewMode );

    void scrollAfterEndH( bool s );
    bool hasScrollAfterEndH() const;

    void scrollBeforeBeginH( bool s );
    bool hasScrollBeforeBeginH() const;

    void scrollAfterEndV( bool s );
    bool hasScrollAfterEndV() const;

    void scrollBeforeBeginV( bool s );
    bool hasScrollBeforeBeginV() const;

    void scrollH( int v );
    void scrollV( int v );

    int  scrollH() const;
    int  scrollV() const;

  protected:
    void mouseWheelEvent(Tempest::MouseEvent &e);
    void mouseMoveEvent(Tempest::MouseEvent &e);

    void gestureEvent(Tempest::AbstractGestureEvent &e);

  private:
    Widget     helper;
    ScrollBar* sbH;
    ScrollBar* sbV;
    scrollViewMode vert, hor;

    Widget*    cen;
    ListView*  list = nullptr;

    bool scAfterEndH    = false;
    bool scBeforeBeginH = false;

    bool scAfterEndV    = true;
    bool scBeforeBeginV = false;

    Tempest::Orientation orient = Tempest::Vertical;
    void initializeList();
    void recalcLayout();
    bool updateScrolls(bool noRetry);
    void emplace(Widget& cen, Widget* scH, Widget* scV, const Rect &place);

    Widget* findFirst();
    Widget* findLast();

    struct Box:Widget {
      ScrollWidget* owner;
      } box;

    struct BoxLayout;
    struct HelperLayout;
    struct MainLayout;

    static Tempest::Size sizeHint( const Widget *w );

    using Tempest::Widget::layout;
  };
/** @}*/

}

#endif // SCROLLWIDGET_H
