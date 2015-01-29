#include "scrollwidget.h"

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

  ScrollBar *scroolH;
  ScrollBar *scroolV;
  Widget    *helper;

  bool scroolAfterEndH    = false;
  bool scroolBeforeBeginH = false;

  bool scroolAfterEndV    = true;
  bool scroolBeforeBeginV = false;
  };

struct ScroolWidget::BoxLayout: public Tempest::LinearLayout {
  BoxLayout(Tempest::Orientation ori):LinearLayout(ori){}

  void applyLayout(){
    LinearLayout::applyLayout();
    if(widgets().size()==0)
      return;

    Size sz = Size{0,0};
    Widget* cen = owner()->owner();
    if(cen){
      sz.w = std::max(cen->w(),sz.w);
      sz.h = std::max(cen->h(),sz.h);
      }

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

  Widget::setLayout(mlay);
  layout().add(&sbH);
  layout().add(&sbV);
  layout().add(&helper);
  cen.setLayout(new BoxLayout(Vertical));
  helper.layout().add(&cen);

  sbH.valueChanged.bind(this, &ScroolWidget::scroolH);
  sbV.valueChanged.bind(this, &ScroolWidget::scroolV);
#ifdef __MOBILE_PLATFORM__
  hideScroolBars();
#endif
  }

ScroolWidget::~ScroolWidget() {
  helper.onResize.removeBinds();
  }

Tempest::Widget &ScroolWidget::centralWidget() {
  return cen;
  }

void ScroolWidget::setLayout(Orientation ori) {
  cen.setLayout(new BoxLayout(ori));
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
  switch(mode){
    case AsNeed:    resizeEv(cen.w(), cen.h()); break;
    case AlwaysOff: sbV.setVisible(false);      break;
    case AlwaysOn:  sbV.setVisible(true);       break;
    }
  }

void ScroolWidget::setHScroolViewMode(ScroolViewMode mode) {
  hor = mode;
  switch(mode){
    case AsNeed:    resizeEv(cen.w(), cen.h()); break;
    case AlwaysOff: sbH.setVisible(false);      break;
    case AlwaysOn:  sbH.setVisible(true);       break;
    }
  }

void ScroolWidget::scroolAfterEndH(bool s) {
  mlay->scroolAfterEndH = s;
  resizeEv( w(), h() );
  mlay->applyLayout();
  }

bool ScroolWidget::hasScroolAfterEndH() const {
  return mlay->scroolAfterEndH;
  }

void ScroolWidget::scroolBeforeBeginH(bool s) {
  mlay->scroolBeforeBeginH = s;
  resizeEv( w(), h() );
  mlay->applyLayout();
  }

bool ScroolWidget::hasScroolBeforeBeginH() const {
  return mlay->scroolBeforeBeginH;
  }

void ScroolWidget::scroolAfterEndV(bool s) {
  mlay->scroolAfterEndV = s;
  resizeEv( w(), h() );
  mlay->applyLayout();
  }

bool ScroolWidget::hasScroolAfterEndV() const {
  return mlay->scroolAfterEndV;
  }

void ScroolWidget::scroolBeforeBeginV(bool s) {
  mlay->scroolBeforeBeginV = s;
  resizeEv( w(), h() );
  mlay->applyLayout();
  }

bool ScroolWidget::hasScroolBeforeBeginV() const {
  return mlay->scroolBeforeBeginV;
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
  }

void ScroolWidget::resizeEv(int,int) {
  using std::max;

  const std::vector<Widget*>& wx = cen.layout().widgets();
  const Widget* first = wx.size()>0 ? wx[0]     : nullptr;
  const Widget* last  = wx.size()>0 ? wx.back() : nullptr;

  const int dx = max(cen.w()-helper.w(),0);
  const int dy = max(cen.h()-helper.h(),0);
  sbH.setRange( mlay->scroolBeforeBeginH && first!=nullptr ? first->w()-dx : 0,
                mlay->scroolAfterEndH && last!=nullptr ? max(last->w()-helper.w(),0) : dx );
  sbV.setRange( mlay->scroolBeforeBeginV && first!=nullptr ? first->h()-dy : 0,
                mlay->scroolAfterEndV && last!=nullptr ? max(last->h()-helper.h(),0) : dy );

  if(hor==AsNeed)
    sbH.setVisible(dx>0);
  if(vert==AsNeed)
    sbV.setVisible(dy>0);

  cen.layout().applyLayout();
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
