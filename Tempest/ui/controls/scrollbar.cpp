#include "scrollbar.h"

#include <Tempest/Layout>
#include <Tempest/Application>

using namespace Tempest;

ScrollBar::ScrollBar(Tempest::Orientation ori) {
  mvalue = 0;
  msmallStep = 10;
  mlargeStep = 20;

  setRange(0, 100);  
  orient = Tempest::Orientation(-1);
  setOrientation( ori );

  setupUi();

  onResize.bind( *this, &ScrollBar::alignCenBtn );
  alignCenBtn(-1, -1);
  }

void ScrollBar::setOrientation( Tempest::Orientation ori ) {
  if(orient==ori)
    return;

  const int ls = linearSize();

  setLayout( ori );
  layout().setMargin(0);
  orient = ori;

  Tempest::SizePolicy p;
  if( ori==Tempest::Vertical ){
    p.maxSize.w = ls;
    p.typeH     = Tempest::FixedMax;
    } else {
    p.maxSize.h = ls;
    p.typeV     = Tempest::FixedMax;
    }

  p.minSize = Tempest::Size(ls);
  setSizePolicy(p);

  onOrientationChanged(ori);
  updateView();
  }

Tempest::Orientation ScrollBar::orientation() const {
  return orient;
  }

void ScrollBar::setRange(int min, int max) {
  if( min>max )
    std::swap(min, max);

  rmin = min;
  rmax = max;
  int nmvalue = std::max( rmin, std::min(mvalue, rmax) );

  msmallStep = std::max<int>(1, int(std::min<int64_t>( 10, range()/100 )));
  mlargeStep = std::max<int>(1, int(std::min<int64_t>( 20, range()/10 )));

  setValue( nmvalue );
  }

int64_t ScrollBar::range() const {
  return rmax-rmin;
  }

int ScrollBar::minValue() const {
  return rmin;
  }

int ScrollBar::maxValue() const {
  return rmax;
  }

void ScrollBar::setValue(int v) {
  v = std::max( rmin, std::min(v, rmax) );

  if( v!=mvalue ){
    mvalue = v;
    onValueChanged(v);
    updateView();
    }
  }

int ScrollBar::value() const {
  return mvalue;
  }

int ScrollBar::smallStep() const {
  return msmallStep;
  }

int ScrollBar::largeStep() const {
  return mlargeStep;
  }

void ScrollBar::setSmallStep(int step) {
  msmallStep = step>=0 ? step : 0;
  }

void ScrollBar::setLargeStep(int step) {
  mlargeStep = step>=0 ? step : 0;
  }

void ScrollBar::setCentralButtonSize(int sz) {
  const UiMetrics& metric = Application::uiMetrics();
  cenBtnSize = std::max<int>(int(metric.scrollButtonSize*metric.uiScale), sz);
  alignCenBtn(0,0);
  }

int ScrollBar::centralAreaSize() const {
  const Widget* w = cenBtn==nullptr ? nullptr : cenBtn->owner();
  if(w==nullptr)
    return 0;

  if(orientation()==Vertical)
    return w->h(); else
    return w->w();
  }

void ScrollBar::hideArrowButtons() {
  arrUp->setVisible(false);
  arrDw->setVisible(false);
  }

void ScrollBar::showArrawButtons() {
  arrUp->setVisible(true);
  arrDw->setVisible(true);
  }

void ScrollBar::paintEvent(PaintEvent &e) {
  if(arrUp->isVisible()) {
    Painter p(e);
    if( orientation()==Vertical )
      style().draw(p,this,Style::E_ArrowUp,  arrUp->state(),arrUp->rect(),Style::Extra(*arrUp)); else
      style().draw(p,this,Style::E_ArrowLeft,arrUp->state(),arrUp->rect(),Style::Extra(*arrUp));
    }

  if(arrDw->isVisible()) {
    Painter p(e);
    if( orientation()==Vertical )
      style().draw(p,this,Style::E_ArrowDown, arrDw->state(),arrDw->rect(),Style::Extra(*arrDw)); else
      style().draw(p,this,Style::E_ArrowRight,arrDw->state(),arrDw->rect(),Style::Extra(*arrDw));
    }

  if(cenBtn->isVisible()) {
    Painter p(e);
    Rect r=cenBtn->rect();
    r.x += cenBtn->owner()->x();
    r.y += cenBtn->owner()->y();
    style().draw(p,this,Style::E_CentralButton,cenBtn->state(),r,Style::Extra(*cenBtn));
    }
  }

void ScrollBar::resizeEvent(Tempest::SizeEvent &) {
  updateView();
  }

void ScrollBar::mouseWheelEvent(MouseEvent &e) {
  if(!isEnabled())
    return;

  if(e.delta>0)
    decL(); else
  if(e.delta<0)
    incL();
  }

void ScrollBar::setupUi() {
  Tempest::SizePolicy p;
  p.typeH   = Tempest::FixedMax;
  p.typeV   = Tempest::FixedMax;
  p.maxSize = Tempest::Size(linearSize());

  Widget*  cen;

  arrUp = new MoveBtn(*this,false);
  arrDw = new MoveBtn(*this,true );
  cen   = new CentralWidget(*this);

  arrUp->setSizePolicy(p);
  arrDw->setSizePolicy(p);

  layout().removeAll();
  layout().setMargin(0);
  layout().add( arrUp );
  layout().add( cen   );
  layout().add( arrDw );

  cenBtn = new CentralButton();
  cenBtn->onPositionChange.bind( *this, &ScrollBar::updateValueFromView );
  cen->layout().add( cenBtn );
  }

void ScrollBar::inc() {
  setValue( value()+msmallStep );
  }

void ScrollBar::dec() {
  setValue( value()-msmallStep );
  }

void ScrollBar::incL() {
  setValue( value()+mlargeStep );
  }

void ScrollBar::decL() {
  setValue( value()-mlargeStep );
  }

void ScrollBar::updateValueFromView(int, int) {
  if( cenBtn==nullptr )
    return;

  Widget* cen = cenBtn->owner();
  if(cen==nullptr)
    return;

  int v;
  const int xrange = cen->w() - cenBtn->w();
  const int yrange = cen->h() - cenBtn->h();
  if( orient==Tempest::Vertical )
    v = int((range()*cenBtn->y())/(yrange>0 ? yrange : 1)); else
    v = int((range()*cenBtn->x())/(xrange>0 ? xrange : 1));

  const int nvalue = std::min(rmax,std::max(v,0) + rmin);
  if(nvalue!=mvalue){
    mvalue = nvalue;
    onValueChanged(nvalue);
    }
  }

int ScrollBar::linearSize() const {
  const UiMetrics& metric = Application::uiMetrics();
  return int(metric.scrollButtonSize*metric.uiScale);
  }

void ScrollBar::updateView() {
  if( cenBtn==nullptr )
    return;

  Widget* cen = cenBtn->owner();
  if(cen==nullptr)
    return;

  Tempest::Point p;
  int cBtnSize=cenBtnSize;
  if( orient==Tempest::Vertical )
    cBtnSize=std::min(cBtnSize,cen->h()); else
    cBtnSize=std::min(cBtnSize,cen->w());

  if( orient==Tempest::Vertical )
    p.y = int(((mvalue-rmin)*(cen->h()-cBtnSize))/std::max<int64_t>(1, range())); else
    p.x = int(((mvalue-rmin)*(cen->w()-cBtnSize))/std::max<int64_t>(1, range()));

  int w = cen->w(), h = cen->h();

  if( orient==Tempest::Vertical )
    h = cBtnSize; else
    w = cBtnSize;

  cenBtn->setGeometry( p.x, p.y, w,h );
  }

void ScrollBar::alignCenBtn( int, int ) {
  updateView();
  }

void ScrollBar::buttonScrollStart(bool up) {
  timer.timeout.removeBinds();
  if(up){
    timer.timeout.bind(this,&ScrollBar::inc);
    inc();
    } else {
    timer.timeout.bind(this,&ScrollBar::dec);
    dec();
    }

  timer.start(50);
  }

void ScrollBar::buttonScrollStop() {
  timer.stop();
  }

void ScrollBar::CentralButton::mouseDownEvent(Tempest::MouseEvent &e) {
  if( !isEnabled() ){
    e.ignore();
    return;
    }
  Button::mouseDownEvent(e);

  mpos   = mapToRoot( e.pos() );
  oldPos = pos();
  update();
  }

void ScrollBar::CentralButton::mouseDragEvent(Tempest::MouseEvent &e) {
  moveTo( oldPos - ( mpos - mapToRoot(e.pos()) ) );
  }

void ScrollBar::CentralButton::keyPressEvent(Tempest::KeyEvent &e) {/*
  if( orient==Tempest::Vertical ){
    if( e.key==Tempest::KeyEvent::K_Up )
      moveTo( pos()+Tempest::Point(0,10) );
    }*/

  e.ignore();
  }

void ScrollBar::CentralButton::moveTo( Tempest::Point p ) {
  p.x = std::max(p.x, 0);
  p.y = std::max(p.y, 0);

  p.x = std::min( owner()->w() - w(), p.x );
  p.y = std::min( owner()->h() - h(), p.y );

  if( pos()!=p )
    setPosition( p );
  }

ScrollBar::CentralWidget::CentralWidget(ScrollBar &owner):owner(owner) {
  }

void ScrollBar::CentralWidget::mouseDownEvent(Tempest::MouseEvent &) {
  update();
  }

void ScrollBar::CentralWidget::mouseUpEvent(Tempest::MouseEvent &e) {
  if(!isEnabled())
    return;

  if( owner.orientation()==Tempest::Vertical && owner.cenBtn!=nullptr ){
    if( e.pos().y < owner.cenBtn->y() )
      owner.decL();

    if( e.pos().y > owner.cenBtn->y()+owner.cenBtn->h() )
      owner.incL();
    } else {
    if( e.pos().x < owner.cenBtn->x() )
      owner.decL();

    if( e.pos().x > owner.cenBtn->x()+owner.cenBtn->w() )
      owner.incL();
    }
  }

void ScrollBar::MoveBtn::mouseDownEvent(MouseEvent &e) {
  Button::mouseDownEvent(e);
  if( isEnabled() )
    owner.buttonScrollStart(dir);
  }

void ScrollBar::MoveBtn::mouseUpEvent(MouseEvent &e) {
  Button::mouseUpEvent(e);
  owner.buttonScrollStop();
  }
