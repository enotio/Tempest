#include "scrollbar.h"

#include <Tempest/Layout>
#include <Tempest/Application>

using namespace Tempest;

ScrollBar::ScrollBar(Tempest::Orientation ori) {
  mvalue = 0;
  msmallStep = 10;
  mlargeStep = 20;

  setRange(0, 100);  
  setOrientation( ori );

  setupUi();

  onResize.bind( *this, &ScrollBar::alignCenBtn );
  alignCenBtn(-1, -1);
  }

ScrollBar::ScrollBar(bool noUi, Tempest::Orientation ori) {
  mvalue = 0;
  msmallStep = 10;
  mlargeStep = 20;

  setRange(0, 100);
  setOrientation( ori );

  if(!noUi)
    setupUi();

  onResize.bind( *this, &ScrollBar::alignCenBtn );
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
    valueChanged(v);
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

void ScrollBar::resizeEvent(Tempest::SizeEvent &) {
  updateView();
  }

void ScrollBar::mouseWheelEvent(MouseEvent &e) {
  if(e.delta>0)
    decL(); else
  if(e.delta<0)
    incL();
  }

Sprite ScrollBar::buttonIcon(bool /*inc*/) const {
  return Sprite();
  }

Button *ScrollBar::createMoveButton(bool inc) {
  MoveBtn* b = new MoveBtn(*this,inc);
  onOrientationChanged.bind(b,&MoveBtn::setupIcon);
  return b;
  }

Widget *ScrollBar::createCentralWidget() {
  return new CentralWidget(*this);
  }

Widget *ScrollBar::createCentralButton() {
  return new CentralButton();
  }

void ScrollBar::setupUi() {
  Tempest::SizePolicy p;
  p.typeH   = Tempest::FixedMax;
  p.typeV   = Tempest::FixedMax;
  p.maxSize = Tempest::Size(linearSize());

  Widget* cen;
  Button* btn[2];
  btn[0] = createMoveButton(false);
  btn[1] = createMoveButton(true);
  cen    = createCentralWidget();

  for(int i=0; i<2; ++i){
    btn[i]->setSizePolicy(p);
    }

  layout().removeAll();
  layout().setMargin(0);
  layout().add( btn[0] );
  layout().add( cen );
  layout().add( btn[1] );

  cenBtn = createCentralButton();
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
    valueChanged(nvalue);
    }
  }

void ScrollBar::assignCentralButton(Widget *btn) {
  cenBtn = btn;
  updateView();
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

  if( orient==Tempest::Vertical )
    p.y = int(((mvalue-rmin)*(cen->h()-cenBtnSize))/std::max<int64_t>(1, range())); else
    p.x = int(((mvalue-rmin)*(cen->w()-cenBtnSize))/std::max<int64_t>(1, range()));

  int w = cen->w(), h = cen->h();

  if( orient==Tempest::Vertical )
    h = cenBtnSize; else
    w = cenBtnSize;

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
  Button::mouseDownEvent(e);

  mpos   = mapToRoot( e.pos() );
  oldPos = pos();
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
  }

void ScrollBar::CentralWidget::mouseUpEvent(Tempest::MouseEvent &e) {
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
  owner.buttonScrollStart(dir);
  }

void ScrollBar::MoveBtn::mouseUpEvent(MouseEvent &e) {
  Button::mouseUpEvent(e);
  owner.buttonScrollStop();
  }

void ScrollBar::MoveBtn::setupIcon(Orientation /*scrollBarOrientation*/) {
  setIcon( owner.buttonIcon(dir) );
  }
