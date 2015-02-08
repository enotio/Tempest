#include "scrollwidget.h"

#include <Tempest/Layout>

using namespace Tempest;

struct ScrollWidget::ProxyLayout: public Tempest::Layout {
  void applyLayout(){
    const int w = scrollV==nullptr ? 0 : scrollV->minSize().w;
    const int h = scrollH==nullptr ? 0 : scrollH->minSize().h;
    const Margin& m = margin();
    Widget& ow = *owner();

    Size sz = owner()->size();

    sz.w -= m.xMargin();
    sz.h -= m.yMargin();

    if(scrollV!=nullptr && scrollV->isVisible()) {
      if(scrollH!=nullptr && scrollH->isVisible())
        placeIn(scrollV, ow.w()-w-m.right, m.top, w, ow.h()-h-m.yMargin());
      else
        placeIn(scrollV, ow.w()-w-m.right, m.top, w, ow.h()-m.yMargin());
      sz.w -= w;
      sz.w -= spacing();
      }

    if(scrollH!=nullptr && scrollH->isVisible()) {
      if(scrollV!=nullptr && scrollV->isVisible())
        placeIn(scrollH, m.left, ow.h()-h, ow.w()-w-m.xMargin(), h);
      else
        placeIn(scrollH, m.left, ow.h()-h, ow.w()-m.xMargin(), h);
      sz.h -= h;
      sz.h -= spacing();
      }

    helper->setGeometry(margin().left, margin().top, sz.w, sz.h);
    }

  ScrollBar *scrollH = nullptr;
  ScrollBar *scrollV = nullptr;
  Widget    *helper;

  bool scrollAfterEndH    = false;
  bool scrollBeforeBeginH = false;

  bool scrollAfterEndV    = true;
  bool scrollBeforeBeginV = false;
  };

struct ScrollWidget::BoxLayout: public Tempest::LinearLayout {
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

struct ScrollWidget::HelperLayout: public Tempest::Layout {
  void applyLayout(){
    Size ow = owner()->size();

    for(Widget* w : widgets()){
      const SizePolicy& sp = w->sizePolicy();
      Size sz = w->size();

      if(sp.typeH==FixedMin)
        sz.w = sp.minSize.w;
      if(sp.typeH==FixedMax)
        sz.w = sp.maxSize.w;
      if(sp.typeH==Preferred || sp.typeH==Expanding)
        sz.w = ow.w;

      if(sp.typeV==FixedMin)
        sz.h = sp.minSize.h;
      if(sp.typeV==FixedMax)
        sz.h = sp.maxSize.h;
      if(sp.typeV==Preferred || sp.typeV==Expanding)
        sz.h = ow.h;

      w->resize(sz);
      }
    }
  };

ScrollWidget::ScrollWidget()
  : vert(AsNeed), hor(AsNeed), sbH(nullptr), sbV(nullptr) {
  helper.setLayout(new HelperLayout());
  helper.onResize.bind(this,&ScrollWidget::resizeEv);
  mlay = new ProxyLayout();
  mlay->helper  = &helper;

  layout().add(&helper);
  Widget::setLayout(mlay);

  cen = new Widget();
  cen->setLayout(new BoxLayout(orient));
  helper.layout().add(cen);

  setScrollBars( createScrollBar(Horizontal),
                 createScrollBar(Vertical  ),
                 false );

#ifdef __MOBILE_PLATFORM__
  hidescrollBars();
#endif
  }

ScrollWidget::ScrollWidget(bool noUi)
  : vert(AsNeed), hor(AsNeed), sbH(nullptr), sbV(nullptr)  {
  helper.setLayout(new HelperLayout());
  helper.onResize.bind(this,&ScrollWidget::resizeEv);
  mlay = new ProxyLayout();
  mlay->helper  = &helper;

  layout().add(&helper);
  Widget::setLayout(mlay);

  cen = new Widget();
  cen->setLayout(new BoxLayout(orient));
  helper.layout().add(cen);

  if(!noUi)
    setScrollBars( createScrollBar(Horizontal),
                   createScrollBar(Vertical  ),
                   false );

#ifdef __MOBILE_PLATFORM__
  hidescrollBars();
#endif
  }

void ScrollWidget::setScrollBars( Tempest::ScrollBar* hor,
                                  Tempest::ScrollBar* vert,
                                  bool deleteOld ) {
  if(sbH!=nullptr)
    sbH->valueChanged.ubind(this, &ScrollWidget::scrollH);
  if(sbV!=nullptr)
    sbV->valueChanged.ubind(this, &ScrollWidget::scrollV);

  if(deleteOld){
    delete sbH;
    delete sbV;
    }

  sbH           = hor;
  sbV           = vert;
  mlay->scrollH = sbH;
  mlay->scrollV = sbV;
  layout().add(sbH);
  layout().add(sbV);

  if(sbH!=nullptr)
    sbH->valueChanged.bind(this, &ScrollWidget::scrollH);
  if(sbV!=nullptr)
    sbV->valueChanged.bind(this, &ScrollWidget::scrollV);

  setHscrollViewMode(this->hor);
  setVscrollViewMode(this->vert);

  resizeEv( w(), h() );
  }

void ScrollWidget::initializeList() {
  if(list!=nullptr)
    return;

  delete cen;
  list = new ListView(orient);
  cen  = list;
  helper.layout().add(cen);
  list->onItemListChanged.bind(this,&ScrollWidget::updatescrolls);
  }

void ScrollWidget::updatescrolls() {
  resizeEv( w(), h() );
  }

ScrollWidget::~ScrollWidget() {
  if(list!=nullptr)
    list->onItemListChanged.ubind(this,&ScrollWidget::updatescrolls);
  helper.onResize.removeBinds();
  }

Tempest::Widget &ScrollWidget::centralWidget() {
  return *cen;
  }

bool ScrollWidget::isListBased() const {
  return list!=nullptr;
  }

ListView *ScrollWidget::asListView() {
  return list;
  }

const ListView *ScrollWidget::asListView() const {
  return list;
  }

void ScrollWidget::removeList() {
  if(list==nullptr)
    return;

  delete list;
  list = nullptr;

  cen = new Widget();
  cen->setLayout(new BoxLayout(orient));
  helper.layout().add(cen);
  }

void ScrollWidget::setLayout(Orientation ori) {
  orient = ori;

  if(list!=nullptr)
    list->setOrientation(orient); else
    cen->setLayout(new BoxLayout(ori));
  }

void ScrollWidget::hidescrollBars() {
  setscrollBarsVisible(0,0);
  }

void ScrollWidget::setscrollBarsVisible(bool vh, bool vv) {
  if( sbH==nullptr || sbV==nullptr )
    return;

  if( vh==sbH->isVisible() && vv==sbV->isVisible() )
    return;

  sbH->setVisible(vh);
  sbV->setVisible(vv);
  resizeEv( w(), h() );
  }

void ScrollWidget::setVscrollViewMode(scrollViewMode mode) {
  if(sbH==nullptr)
    return;

  vert = mode;
  switch(mode){
    case AsNeed:    resizeEv(cen->w(), cen->h()); break;
    case AlwaysOff: sbV->setVisible(false);     break;
    case AlwaysOn:  sbV->setVisible(true);      break;
    }
  }

void ScrollWidget::setHscrollViewMode(scrollViewMode mode) {
  if(sbH==nullptr)
    return;

  hor = mode;
  switch(mode){
    case AsNeed:    resizeEv(cen->w(), cen->h()); break;
    case AlwaysOff: sbH->setVisible(false);     break;
    case AlwaysOn:  sbH->setVisible(true);      break;
    }
  }

void ScrollWidget::scrollAfterEndH(bool s) {
  mlay->scrollAfterEndH = s;
  resizeEv( w(), h() );
  mlay->applyLayout();
  }

bool ScrollWidget::hasscrollAfterEndH() const {
  return mlay->scrollAfterEndH;
  }

void ScrollWidget::scrollBeforeBeginH(bool s) {
  mlay->scrollBeforeBeginH = s;
  resizeEv( w(), h() );
  mlay->applyLayout();
  }

bool ScrollWidget::hasscrollBeforeBeginH() const {
  return mlay->scrollBeforeBeginH;
  }

void ScrollWidget::scrollAfterEndV(bool s) {
  mlay->scrollAfterEndV = s;
  resizeEv( w(), h() );
  mlay->applyLayout();
  }

bool ScrollWidget::hasscrollAfterEndV() const {
  return mlay->scrollAfterEndV;
  }

void ScrollWidget::scrollBeforeBeginV(bool s) {
  mlay->scrollBeforeBeginV = s;
  resizeEv( w(), h() );
  mlay->applyLayout();
  }

bool ScrollWidget::hasscrollBeforeBeginV() const {
  return mlay->scrollBeforeBeginV;
  }

void ScrollWidget::mouseWheelEvent(Tempest::MouseEvent &e) {
  if(sbV==nullptr)
    return;

  if( !rect().contains(e.x+x(), e.y+y()) || !sbV->isVisible() ){
    e.ignore();
    return;
    }

  if(e.delta>0)
    sbV->setValue(sbV->value() - sbV->largeStep()); else
  if(e.delta<0)
    sbV->setValue(sbV->value() + sbV->largeStep());
  }

void ScrollWidget::mouseMoveEvent(Tempest::MouseEvent &e) {
  e.ignore();
  }

void ScrollWidget::gestureEvent(Tempest::AbstractGestureEvent &e) {
  e.ignore();
  if( sbV==nullptr || sbV->isVisible() )
    return;

  if( e.gestureType()==Tempest::AbstractGestureEvent::gtDragGesture ){
    Tempest::DragGesture &d = (Tempest::DragGesture&)(e);
    int v = sbV->value();
    int dpos = d.dpos.y;
    dpos = d.dpos.x;

    sbV->setValue(sbV->value() - dpos );
    if( v!=sbV->value() )
      e.accept();
    }
  }

void ScrollWidget::resizeEvent(SizeEvent &) {
  resizeEv(helper.w(), helper.h());
  }

ScrollBar *ScrollWidget::createScrollBar(Orientation ori) {
  return new ScrollBar(ori);
  }

void ScrollWidget::scrollH( int v ) {
  if(sbH!=nullptr)
    sbH->setValue( v );
  cen->setPosition(-v, cen->y());
  //mlay->applyLayout();
  }

void ScrollWidget::scrollV(int v) {
  if(sbV!=nullptr)
    sbV->setValue( v );
  cen->setPosition(cen->x(), -v);
  }

void ScrollWidget::resizeEv(int,int) {
  using std::max;
  using std::min;

  const std::vector<Widget*>& wx = cen->layout().widgets();
  const Widget* first = wx.size()>0 ? wx[0]     : nullptr;
  const Widget* last  = wx.size()>0 ? wx.back() : nullptr;

  const int dx = max(cen->w()-helper.w(),0);
  const int dy = max(cen->h()-helper.h(),0);
  if(sbH!=nullptr){
    int maxSc = dy;
    if(mlay->scrollAfterEndH && last!=nullptr)
      maxSc = max(cen->w()-min(last->w(),helper.w()),0);
    sbH->setRange( (mlay->scrollBeforeBeginH && first!=nullptr) ? first->w()-dx : 0,
                   maxSc );
    }

  if(sbV!=nullptr){
    int maxSc = dy;
    if(mlay->scrollAfterEndV && last!=nullptr)
      maxSc = max(cen->h()-min(last->h(),helper.h()),0);
    sbV->setRange( (mlay->scrollBeforeBeginV && first!=nullptr) ? first->h()-dy : 0,
                    maxSc );
    }

  if(hor==AsNeed)
    sbH->setVisible(dx>0);
  if(vert==AsNeed)
    sbV->setVisible(dy>0);

  cen->layout().applyLayout();
  }

Tempest::Size ScrollWidget::sizeHint(const Tempest::Widget *wid) {
  int w = wid->sizePolicy().minSize.w,
      h = wid->sizePolicy().minSize.h;

  if( wid->sizePolicy().typeH==Tempest::FixedMax )
    w = wid->sizePolicy().maxSize.w;
  if( wid->sizePolicy().typeV==Tempest::FixedMax )
    h = wid->sizePolicy().maxSize.h;

  return Tempest::Size(w,h);
  }
