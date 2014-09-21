#include "scroolbar.h"

#include <Tempest/Layout>
#include <Tempest/Application>

using namespace Tempest;

ScroolBar::ScroolBar() {
  mvalue = 0;
  smallStep = 10;
  largeStep = 20;

  setRange(0, 100);

  const UiMetrics& metric = Application::uiMetrics();

  Tempest::SizePolicy p;
  p.typeH   = Tempest::FixedMax;
  p.typeV   = Tempest::FixedMax;
  p.maxSize = Tempest::Size(metric.scroolButtonSize*metric.uiScale);

  MoveBtn *b[2] = { new MoveBtn(this,false), new MoveBtn(this,true) };
  btn[0] = b[0];
  btn[1] = b[1];

  setOrientation( Tempest::Vertical );

  cen = new CenWidget( this );

  for(int i=0; i<2; ++i){
    b[i]->setSizePolicy(p);
    }

  layout().add( b[0] );
  layout().add( cen );
  layout().add( b[1] );

  cenBtn = new CenBtn();
  cenBtn->onPositionChange.bind( *this, &ScroolBar::updateValueFromView );
  cen->layout().add( cenBtn );

  onResize.bind( *this, &ScroolBar::alignCenBtn );
  alignCenBtn(-1, -1);
  }

void ScroolBar::setOrientation( Tempest::Orientation ori ) {
  if(orient==ori)
    return;

  const UiMetrics& metric = Application::uiMetrics();

  setLayout( ori );
  layout().setMargin(0);
  orient = ori;

  Tempest::SizePolicy p;
  if( ori==Tempest::Vertical ){
    p.maxSize.w = metric.scroolButtonSize*metric.uiScale;
    p.typeH     = Tempest::FixedMax;
    } else {
    p.maxSize.h = metric.scroolButtonSize*metric.uiScale;
    p.typeV     = Tempest::FixedMax;
    }

  p.minSize = Tempest::Size(metric.scroolButtonSize*metric.uiScale);
  setSizePolicy(p);

  btn[0]->icon = buttonIcon(true);
  btn[1]->icon = buttonIcon(false);
  }

Tempest::Orientation ScroolBar::orientation() const {
  return orient;
  }

void ScroolBar::setRange(int min, int max) {
  if( min>max )
    std::swap(min, max);

  rmin = min;
  rmax = max;
  int nmvalue = std::max( rmin, std::min(mvalue, rmax) );

  smallStep = std::max(1, std::min( 10, range()/100 ));
  largeStep = std::max(1, std::min( 20, range()/10 ));

  setValue( nmvalue );
  }

int ScroolBar::range() const {
  return rmax-rmin;
  }

int ScroolBar::minValue() const {
  return rmin;
  }

int ScroolBar::maxValue() const {
  return rmax;
  }

void ScroolBar::setValue(int v) {
  v = std::max( rmin, std::min(v, rmax) );

  if( v!=mvalue ){
    mvalue = v;
    valueChanged(v);
    updateView();
    }
  }

int ScroolBar::value() const {
  return mvalue;
  }

void ScroolBar::setCentralButtonSize(int sz) {
  const UiMetrics& metric = Application::uiMetrics();
  cenBtnSize = std::max<int>(metric.scroolButtonSize*metric.uiScale, sz);
  alignCenBtn(0,0);
  }

const Tempest::Widget &ScroolBar::centralWidget() const {
  return *cen;
  }

void ScroolBar::resizeEvent(Tempest::SizeEvent &) {
  updateView();
  }

Sprite ScroolBar::buttonIcon(bool /*inc*/) const {
  return Sprite();
  }

void ScroolBar::inc() {
  setValue( value()+smallStep );
  }

void ScroolBar::dec() {
  setValue( value()-smallStep );
  }

void ScroolBar::incL() {
  setValue( value()+largeStep );
  }

void ScroolBar::decL() {
  setValue( value()-largeStep );
  }

void ScroolBar::updateValueFromView(int, unsigned) {
  int v;
  if( orient==Tempest::Vertical )
    v = (range()*cenBtn->y())/(cen->h() - cenBtn->h()); else
    v = (range()*cenBtn->x())/(cen->w() - cenBtn->w());

  mvalue = v + rmin;
  valueChanged(v);
  }

void ScroolBar::updateView() {
  Tempest::Point p = cenBtn->pos();

  if( orient==Tempest::Vertical )
    p.y = mvalue*(cen->h()-cenBtnSize)/std::max(1, rmax-rmin); else
    p.x = mvalue*(cen->w()-cenBtnSize)/std::max(1, rmax-rmin);

  int w = cen->w(), h = cen->h();

  if( orient==Tempest::Vertical )
    h = cenBtnSize; else
    w = cenBtnSize;

  cenBtn->setGeometry( p.x, p.y, w,h );
  }

void ScroolBar::alignCenBtn( int, int ) {
  updateView();
  }

void ScroolBar::buttonScroolStart(bool up) {
  timer.timeout.removeBinds();
  if(up){
    timer.timeout.bind(this,&ScroolBar::inc);
    inc();
    } else {
    timer.timeout.bind(this,&ScroolBar::dec);
    dec();
    }

  timer.start(50);
  }

void ScroolBar::buttonScroolStop() {
  timer.stop();
  }

void ScroolBar::CenBtn::mouseDownEvent(Tempest::MouseEvent &e) {
  Button::mouseDownEvent(e);

  mpos   = mapToRoot( e.pos() );
  oldPos = pos();
  }

void ScroolBar::CenBtn::mouseDragEvent(Tempest::MouseEvent &e) {
  moveTo( oldPos - ( mpos - mapToRoot(e.pos()) ) );
  }

void ScroolBar::CenBtn::keyPressEvent(Tempest::KeyEvent &e) {/*
  if( orient==Tempest::Vertical ){
    if( e.key==Tempest::KeyEvent::K_Up )
      moveTo( pos()+Tempest::Point(0,10) );
    }*/

  e.ignore();
  }

void ScroolBar::CenBtn::moveTo( Tempest::Point p ) {
  p.x = std::max(p.x, 0);
  p.y = std::max(p.y, 0);

  p.x = std::min( owner()->w() - w(), p.x );
  p.y = std::min( owner()->h() - h(), p.y );

  if( pos()!=p )
    setPosition( p );
  }

ScroolBar::CenWidget::CenWidget(ScroolBar *owner) {
  ow = owner;
  }

void ScroolBar::CenWidget::mouseDownEvent(Tempest::MouseEvent &) {
  }

void ScroolBar::CenWidget::mouseUpEvent(Tempest::MouseEvent &e) {
  if( ow->orientation()==Tempest::Vertical ){
    if( e.pos().y < ow->cenBtn->y() )
      ow->decL();

    if( e.pos().y > ow->cenBtn->y()+ow->cenBtn->h() )
      ow->incL();
    } else {
    if( e.pos().x < ow->cenBtn->x() )
      ow->decL();

    if( e.pos().x > ow->cenBtn->x()+ow->cenBtn->w() )
      ow->incL();
    }
  }

void ScroolBar::MoveBtn::mouseDownEvent(MouseEvent &e) {
  Button::mouseDownEvent(e);
  owner->buttonScroolStart(dir);
  }

void ScroolBar::MoveBtn::mouseUpEvent(MouseEvent &e) {
  Button::mouseUpEvent(e);
  owner->buttonScroolStop();
  }
