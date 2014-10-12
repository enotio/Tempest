#ifndef SCROOLBAR_H
#define SCROOLBAR_H

#include <Tempest/Widget>
#include <Tempest/Button>
#include <Tempest/Timer>

namespace Tempest{

class ScroolBar : public Tempest::Widget {
  public:
    ScroolBar();

    void setOrientation( Tempest::Orientation ori );
    Tempest::Orientation orientation() const;

    void setRange( int min, int max );
    int range() const;
    int minValue() const;
    int maxValue() const;

    void setValue( int v );
    int  value() const;

    int smallStep() const;
    int largeStep() const;

    void setSmallStep(int step);
    void setLargeStep(int step);

    void setCentralButtonSize( int sz );

    Tempest::signal<int> valueChanged;

    const Widget& centralWidget() const;
  protected:
    void resizeEvent( Tempest::SizeEvent& e );
    void mouseWheelEvent(Tempest::MouseEvent& e);
    virtual Sprite buttonIcon(bool inc) const;

  private:
    void inc();
    void dec();

    void incL();
    void decL();

    void updateValueFromView(int, unsigned);
    void updateView();

    int rmin, rmax, msmallStep, mlargeStep;
    int mvalue;

    Tempest::Orientation orient;

    void alignCenBtn(int, int);

    struct CenBtn: public Button {
      CenBtn(){}

      void mouseDownEvent(Tempest::MouseEvent &e);
      void mouseDragEvent(Tempest::MouseEvent &e);
      void keyPressEvent(Tempest::KeyEvent &e);

      //bool mouseTracking;
      Tempest::Point mpos, oldPos;

      void moveTo( Tempest::Point p );
      };

    struct MoveBtn: Button {
      MoveBtn(ScroolBar *owner, bool dir):owner(owner), dir(dir){}

      void mouseDownEvent(Tempest::MouseEvent& e);
      void mouseUpEvent(Tempest::MouseEvent& e);

      ScroolBar* owner;
      const bool dir;
      };

    struct CenWidget : public Widget {
      CenWidget( ScroolBar *owner );

      void mouseDownEvent(Tempest::MouseEvent &e);
      void mouseUpEvent(Tempest::MouseEvent &e);

      ScroolBar * ow;
      };

    Button    * btn[2];
    CenWidget * cen;
    CenBtn    * cenBtn;
    int cenBtnSize = 40;

    void buttonScroolStart(bool up);
    void buttonScroolStop();

    Timer timer;
  };
}

#endif // SCROOLBAR_H
