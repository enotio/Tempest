#ifndef WIDGET_H
#define WIDGET_H

#include <memory>
#include <algorithm>

#include <Tempest/Utility>
#include <Tempest/signal>
#include <Tempest/SizePolicy>
#include <Tempest/Painter>
#include <Tempest/Event>

#include <Tempest/ResourceContext>

namespace Tempest{

namespace Bind{
  class UserTexture;
  }

enum Orientation{
  Horizontal,
  Vertical
  };

enum FocusPolicy{
  NoFocus,
  BackgroundFocus,
  TabFocus,
  ClickFocus
  };

class Layout;
class LinearLayout;
class Shortcut;

// class Painter;

class Widget {
  public:
    Widget( ResourceContext* context = 0 );
    virtual ~Widget();
    void deleteLater();

    int x() const;
    int y() const;
    int w() const;
    int h() const;

    Point pos()  const;
    Rect  rect() const;
    Size  size() const;

    virtual void setPosition( int x, int y );
    void setPosition( const Point & p );
    virtual void resize(int w, int h );
    void resize( const Size & s );

    void setGeometry( const Rect & r );
    void setGeometry( int x, int y, int w, int h );

    void setMaximumSize( const Size & s );
    void setMinimumSize( const Size & s );

    void setMaximumSize( int w, int h );
    void setMinimumSize( int w, int h );

    Size minSize() const;
    Size maxSize() const;

    signal<int, int> onPositionChange, onResize;
    signal<bool>     onFocusChange, onChildFocusChange;
    signal<Widget*>  onDestroy;

    const SizePolicy sizePolicy() const;
    void setSizePolicy( const SizePolicy& s );
    void setSizePolicy( Tempest::SizePolicyType f );
    void setSizePolicy( Tempest::SizePolicyType f0,
                        Tempest::SizePolicyType f1 );

    void setSpacing( int s );
    int  spacing() const;

    void setMargin( const Margin & m );
    void setMargin( int l, int r, int t, int b );
    const Margin& margin() const;

    FocusPolicy focusPolicy() const;
    void setFocusPolicy( FocusPolicy f );

    void setLayout( Orientation ori );
    void setLayout( Layout* l );

    Layout& layout();
    const Layout& layout() const;

    Layout* parentLayout();
    const Layout* parentLayout() const;

    Widget * owner();
    const Widget * owner() const;

    void setFocus( bool f );
    bool hasFocus() const;
    bool hasChildFocus() const;

    void useScissor( bool u );
    bool isScissorUsed() const;

    Point mapToRoot( const Point & p ) const;

    virtual void paintEvent ( Tempest::PaintEvent & e );
    virtual void mouseMoveEvent ( Tempest::MouseEvent & e );
    virtual void mouseDragEvent ( Tempest::MouseEvent & e );
    virtual void mouseDownEvent ( Tempest::MouseEvent & e );
    virtual void mouseUpEvent   ( Tempest::MouseEvent & e );

    virtual void mouseWheelEvent( Tempest::MouseEvent & e );

    virtual void keyDownEvent( Tempest::KeyEvent & e );
    virtual void keyUpEvent( Tempest::KeyEvent & e );
    virtual void customEvent( Tempest::CustomEvent & e );
    virtual void closeEvent( Tempest::CloseEvent & e );

    virtual void shortcutEvent( Tempest::KeyEvent & e );
    virtual void resizeEvent( Tempest::SizeEvent& e );

    virtual void gestureEvent( Tempest::AbstractGestureEvent & e );

    signal<> intentToUpdate;
    void update();
    bool needToUpdate() const;

    void setMultiPassPaint( bool a );
    bool hasMultiPassPaint() const;

    Widget* findRoot();

    bool isVisible() const;
    void setVisible( bool v );

    ResourceContext* context() const;
    void setContext( ResourceContext* context );

    bool hasMultitouch() const;
    void setMultiTouchTracking( bool multiTouch );
  protected:
    virtual void paintNested( Tempest::PaintEvent & p );

  private:
    Widget( const Widget& ){}
    Widget& operator = ( const Widget& ) { return *this; }

    void rootMouseDownEvent ( Tempest::MouseEvent & e );
    void rootMouseUpEvent   ( Tempest::MouseEvent & e );
    void rootMouseDragEvent ( Tempest::MouseEvent & e );
    void rootMouseMoveEvent ( Tempest::MouseEvent & e );
    void rootMouseWheelEvent( Tempest::MouseEvent & e );

    void rootKeyDownEvent( Tempest::KeyEvent & e );
    void rootKeyUpEvent  ( Tempest::KeyEvent & e );

    void rootShortcutEvent( Tempest::KeyEvent & e );
    void rootCloseEvent   ( Tempest::CloseEvent & e );

    void rootGestureEvent( Tempest::AbstractGestureEvent & e );


    static Widget* impl_mouseEvent( Tempest::MouseEvent & e,
                                    Widget *root,
                                    void (Widget::*f)(Tempest::MouseEvent &),
                                    bool focus = true, bool mpress = false );
    static void impl_mouseUpEvent  ( Widget* w, Tempest::MouseEvent & e );
    static void impl_mouseDragEvent( Widget* w, Tempest::MouseEvent & e );
    static void impl_keyPressEvent ( Widget* w, Tempest::KeyEvent & e,
                                     void (Widget::*f)(Tempest::KeyEvent &) );

    static void impl_customEvent(Widget *w, Tempest::CustomEvent & e );
    static void impl_closeEvent (Widget *w, Tempest::CloseEvent & e );

    static void impl_gestureEvent( Widget *w, Tempest::AbstractGestureEvent & e );

    void unsetChFocus( Widget* root, Widget* emiter );

    Rect wrect;
    bool wvisible;
    SizePolicy sp;
    FocusPolicy fpolicy;

    bool focus, chFocus, uscissor;
    std::vector<Widget*> mouseReleseReciver;
    Layout * mlay;
    Layout * parentLay;

    ResourceContext* rcontext;

    bool nToUpdate, multiPaint, deleteLaterFlag, multiTouch;

    int  deleteLaterFlagGuard;
    void lockDelete();
    void unlockDelete();

    std::vector<Shortcut*> skuts;

    static size_t count;
    struct DeleteGuard;

  friend class Layout;
  friend class Shortcut;
  friend class SystemAPI;
  friend class Application;
  };

}

#endif // WIDGET_H