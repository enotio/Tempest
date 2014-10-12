#include "scroolwidget.h"

#include <Tempest/Layout>

using namespace Tempest;

struct ScroolWidget::ProxyLayout: public Tempest::Layout {
  void applyLayout(){
    const int w = scroolV->minSize().w;
    const int h = scroolH->minSize().h;
    const Margin& m = margin();

    Size sz = owner()->size();

    sz.w -= m.xMargin();
    sz.h -= m.yMargin();

    if(scroolV->isVisible()) {
      if(scroolH->isVisible())
        placeIn(scroolV, owner()->w()-w-m.right, m.top, w, owner()->h()-h-m.yMargin());
      else
        placeIn(scroolV, owner()->w()-w-m.right, m.top, w, owner()->h()-m.yMargin());
      sz.w -= w;
      sz.w -= spacing();
      }

    if(scroolH->isVisible()) {
      if(scroolV->isVisible())
        placeIn(scroolH, m.left, owner()->h()-h, owner()->w()-w-m.xMargin(), h);
      else
        placeIn(scroolH, m.left, owner()->h()-h, owner()->w()-m.xMargin(), h);
      sz.h -= h;
      sz.h -= spacing();
      }

    helper->setGeometry(margin().left, margin().top, sz.w, sz.h);
    }

  ScroolBar *scroolH;
  ScroolBar *scroolV;
  Widget    *helper;

  bool scroolAfterEnd    = false;
  bool scroolBeforeBegin = false;
  };

struct ScroolWidget::BoxLayout: public Tempest::LinearLayout {
  void applyLayout(){
    LinearLayout::applyLayout();
    if(widgets().size()==0)
      return;

    Widget* ow = owner()->owner();
    Size sz = ow!=nullptr ? ow->size() : Size{0,0};

    for(const Widget* w : widgets()) {
      sz.w = std::max(sz.w,w->x()+w->w());
      sz.h = std::max(sz.h,w->y()+w->h());
      }

    Widget* w = widgets().back();
    if(orientation()==Horizontal)
      owner()->resize(w->x()+w->w()+margin().xMargin(), sz.h); else
      owner()->resize(sz.w,w->y()+w->h()+margin().yMargin());
    }
  };

ScroolWidget::ScroolWidget()
  : vert(AsNeed), hor(AsNeed) {
  helper.onResize.bind(this,&ScroolWidget::resizeEv);

  mlay=new ProxyLayout();
  mlay->scroolH = &sbH;
  mlay->scroolV = &sbV;
  mlay->helper  = &helper;
  sbH.setOrientation(Horizontal);

  setLayout(mlay);
  layout().add(&sbH);
  layout().add(&sbV);
  layout().add(&helper);
  cen.setLayout(new BoxLayout());
  helper.layout().add(&cen);

  sbH.valueChanged.bind(this, &ScroolWidget::scroolH);
  sbV.valueChanged.bind(this, &ScroolWidget::scroolV);
#ifdef __MOBILE_PLATFORM__
  hideScroolBars();
#endif
  }

Tempest::Widget &ScroolWidget::centralWidget() {
  return cen;
  }

void ScroolWidget::hideScroolBars() {
  setScroolBarsVisible(0,0);
  }

void ScroolWidget::setScroolBarsVisible(bool vh, bool vv) {
  if( vh==sbH.isVisible() && vv==sbV.isVisible() )
    return;

  sbH.setVisible(vh);
  sbV.setVisible(vv);
  resizeEv( w(), h() );
  }

void ScroolWidget::setVScroolViewMode(ScroolViewMode mode) {
  vert = mode;
  }

void ScroolWidget::setHScroolViewMode(ScroolViewMode mode) {
  hor = mode;
  }

void ScroolWidget::scroolAfterEnd(bool s) {
  mlay->scroolAfterEnd = s;
  resizeEv( w(), h() );
  mlay->applyLayout();
  }

bool ScroolWidget::hasScroolAfterEnd() const {
  return mlay->scroolAfterEnd;
  }

void ScroolWidget::scroolBeforeBegin(bool s) {
  mlay->scroolBeforeBegin = s;
  resizeEv( w(), h() );
  mlay->applyLayout();
  }

bool ScroolWidget::hasScroolBeforeBegin() const {
  return mlay->scroolBeforeBegin;
  }

void ScroolWidget::mouseWheelEvent(Tempest::MouseEvent &e) {
  if( !rect().contains(e.x+x(), e.y+y()) || !sbV.isVisible() ){
    e.ignore();
    return;
    }

  if(e.delta>0)
    sbV.setValue(sbV.value() - sbV.largeStep()); else
  if(e.delta<0)
    sbV.setValue(sbV.value() + sbV.largeStep());
  }

void ScroolWidget::mouseMoveEvent(Tempest::MouseEvent &e) {
  e.ignore();
  }

void ScroolWidget::gestureEvent(Tempest::AbstractGestureEvent &e) {
  e.ignore();
  if( sbV.isVisible() )
    return;

  if( e.gestureType()==Tempest::AbstractGestureEvent::gtDragGesture ){
    Tempest::DragGesture &d = (Tempest::DragGesture&)(e);
    int v = sbV.value();
    int dpos = d.dpos.y;
    dpos = d.dpos.x;

    sbV.setValue(sbV.value() - dpos );
    if( v!=sbV.value() )
      e.accept();
    }
  }

void ScroolWidget::resizeEvent(SizeEvent &) {
  resizeEv(helper.w(), helper.h());
  }

void ScroolWidget::scroolH( int v ) {
  sbH.setValue( v );
  cen.setPosition(-v, cen.y());
  //mlay->applyLayout();
  }

void ScroolWidget::scroolV(int v) {
  sbV.setValue( v );
  cen.setPosition(cen.x(), -v);
  //mlay->applyLayout();
  }

void ScroolWidget::resizeEv(int,int) {
  sbH.setRange(0,cen.w());
  sbV.setRange(0,cen.h());
  }

Tempest::Size ScroolWidget::sizeHint(const Tempest::Widget *wid) {
  int w = wid->sizePolicy().minSize.w,
      h = wid->sizePolicy().minSize.h;

  if( wid->sizePolicy().typeH==Tempest::FixedMax )
    w = wid->sizePolicy().maxSize.w;
  if( wid->sizePolicy().typeV==Tempest::FixedMax )
    h = wid->sizePolicy().maxSize.h;

  return Tempest::Size(w,h);
  }
