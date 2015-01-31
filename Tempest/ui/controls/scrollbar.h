#ifndef SCROLLBAR_H
#define SCROLLBAR_H

#include <Tempest/Widget>
#include <Tempest/Button>
#include <Tempest/Timer>
#include <cstdint>

namespace Tempest{

class ScrollBar : public Tempest::Widget {
  public:
    ScrollBar(Tempest::Orientation ori = Tempest::Vertical);

    void setOrientation( Tempest::Orientation ori );
    Tempest::Orientation orientation() const;
    Tempest::signal<Tempest::Orientation> onOrientationChanged;

    void    setRange( int min, int max );
    int64_t range() const;

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

  protected:
    ScrollBar(bool noUi, Tempest::Orientation ori = Tempest::Vertical);

    void resizeEvent( Tempest::SizeEvent& e );
    void mouseWheelEvent(Tempest::MouseEvent& e);

    virtual Sprite  buttonIcon(bool inc) const;
    virtual Button* createMoveButton(bool inc);
    virtual Widget* createCentralWidget();
    virtual Widget* createCentralButton();
    virtual void    setupUi();
    void            updateValueFromView(int,int);
    void            assignCentralButton(Widget* btn);
    virtual int     linearSize() const;

    struct MoveBtn: Button {
      MoveBtn(ScrollBar& owner, bool dir):owner(owner), dir(dir){}

      void mouseDownEvent(Tempest::MouseEvent& e);
      void mouseUpEvent(Tempest::MouseEvent& e);

      void setupIcon(Tempest::Orientation scrollBarOrientation);

      ScrollBar& owner;
      const bool dir;
      };

    struct CentralButton: public Button {
      void mouseDownEvent(Tempest::MouseEvent &e);
      void mouseDragEvent(Tempest::MouseEvent &e);
      void keyPressEvent(Tempest::KeyEvent &e);

      void moveTo( Tempest::Point p );

      private:
        Tempest::Point mpos, oldPos;
      };

    struct CentralWidget : public Widget {
      CentralWidget( ScrollBar& owner );

      void mouseDownEvent(Tempest::MouseEvent &e);
      void mouseUpEvent(Tempest::MouseEvent &e);

      ScrollBar& owner;
      };

  private:
    void inc();
    void dec();

    void incL();
    void decL();

    void updateView();

    int rmin, rmax, msmallStep, mlargeStep;
    int mvalue;

    Tempest::Orientation orient;

    void alignCenBtn(int, int);

    Widget * cenBtn = nullptr;
    int cenBtnSize = 40;

    void buttonScroolStart(bool up);
    void buttonScroolStop();

    Timer timer;
  };
}

#endif // SCROLLBAR_H
