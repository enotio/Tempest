#ifndef LAYOUT_H
#define LAYOUT_H

#include <vector>
#include <algorithm>

#include <Tempest/Widget>

namespace Tempest{

class WidgetBase;

class LinearLayout;

class Layout {
  public:
    Layout();
    virtual ~Layout();

    Widget * owner();
    const Widget * owner() const;

    void add( Widget* widget );
    void del( Widget* widget );
    void removeAll();

    Widget* take( Widget* widget );

    const std::vector<Widget*>& widgets();

    virtual void applyLayout(){}

    void setSpacing( int s );
    int  spacing() const;

    void setMargin( const Margin & m );
    void setMargin( int l, int r, int t, int b );
    const Margin& margin() const;

  protected:
    static void placeIn( Widget *wx, int x, int y, int w, int h );
    static void placeIn( Widget *wx, const Rect & r );

  private:
    std::vector<Widget*> w;
    Widget* ow;
    int mspacing;
    Margin mmargin;

    void rebind( Widget* w);
    void swap( Layout* other );

    void execDelete();

  friend class Widget;
  };

class LinearLayout : public Layout {
  public:
    LinearLayout( Orientation ori = Vertical ):orient(ori){
      this->setSpacing(2);
      }

    virtual void applyLayout(){
      if( orient==Vertical ){
        applyLayoutPrivate( &h, &w, &typeV, &typeH, false, &setGeometryR );
        } else {
        applyLayoutPrivate( &w, &h, &typeH, &typeV,  true, &setGeometryD );
        }

      }

    Orientation orientation(){
      return orient;
      }

    void setOrientation( Orientation ori ){
      if( orient!=ori ){
        orient = ori;
        applyLayout();
        }
      }

  private:
    Orientation orient;

    static void setGeometryD( Widget* wd, int x, int y, int w, int h){
      //wd->setGeometry(x,y,w,h);
      placeIn(wd, x, y, w, h );
      }
    static void setGeometryR( Widget* wd, int x, int y, int w, int h ){
      //wd->setGeometry(y,x,h,w);
      placeIn(wd, y, x, h, w );
      }

    static int w( const Size& wd ){
      return wd.w;
      }

    static int h( const Size& wd ){
      return wd.h;
      }

    static SizePolicyType typeH( const Widget* wd ){
      return wd->sizePolicy().typeH;
      }

    static SizePolicyType typeV( const Widget* wd ){
      return wd->sizePolicy().typeV;
      }

    int marg(bool h){
      if( h )
        return this->margin().left + this->margin().right; else
        return this->margin().top + this->margin().bottom;
      }

    void applyLayoutPrivate( int (*w)( const Size& wd ),
                             int (*h)( const Size& wd ),
                             SizePolicyType (*typeH)( const Widget*  ),
                             SizePolicyType (*typeV)( const Widget*  ),
                             bool hor,
                             void (*setGeometry)( Widget* wd,
                                                  int x, int y,
                                                  int w, int h ) );
  };

}

#endif // LAYOUT_H
