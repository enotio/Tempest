#include "scrollwidget.h"

#include <Tempest/Layout>

using namespace Tempest;


struct ScrollWidget::BoxLayout: public Tempest::LinearLayout {
  BoxLayout( ScrollWidget* owner,
             Tempest::Orientation ori ):LinearLayout(ori), sc(owner){}

  void applyLayout(){
    if(widgets().size()==0)
      return;

    const Margin& m = margin();
    Size sz = Size{0,0};
    Widget* helper = owner()->owner();
    if(helper){
      sz.w = std::max(helper->w(),sz.w);
      sz.h = std::max(helper->h(),sz.h);
      }

    int sw = 0, sh = 0;

    const Widget* first = widgets().size()>0 ? widgets()[0] : nullptr;
    for(const Widget* w : widgets())
      if(w->isVisible()){
        Size s = sizeHint(w);
        if(orientation()==Horizontal){
          sw += s.w;
          if(w!=first)
            sw+=spacing();
          sh = std::max(sh,s.h);
          } else {
          sw = std::max(sw,s.w);
          sh += s.h;
          if(w!=first)
            sh+=spacing();
          }
        }

    sw += m.xMargin();
    sh += m.yMargin();

    sz.w = std::max(sz.w,sw);
    sz.h = std::max(sz.h,sh);
    owner()->resize(sz);

    sc->updateScrolls();
    LinearLayout::applyLayout();
    }

  ScrollWidget* sc;
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
  : sbH(nullptr), sbV(nullptr), vert(AsNeed), hor(AsNeed) {
  helper.setLayout(new HelperLayout());
  helper.onResize.bind(this,&ScrollWidget::resizeEv);

  layout().add(&helper);

  cen = new Widget();
  cen->setLayout(new BoxLayout(this,orient));
  helper.layout().add(cen);

  setScrollBars( createScrollBar(Horizontal),
                 createScrollBar(Vertical  ),
                 false );

#ifdef __MOBILE_PLATFORM__
  hideScrollBars();
#endif
  }

ScrollWidget::ScrollWidget(bool noUi)
  : sbH(nullptr), sbV(nullptr), vert(AsNeed), hor(AsNeed) {
  helper.setLayout(new HelperLayout());
  helper.onResize.bind(this,&ScrollWidget::resizeEv);

  layout().add(&helper);

  cen = new Widget();
  cen->setLayout(new BoxLayout(this,orient));
  helper.layout().add(cen);

  if(!noUi)
    setScrollBars( createScrollBar(Horizontal),
                   createScrollBar(Vertical  ),
                   false );

#ifdef __MOBILE_PLATFORM__
  hideScrollBars();
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
  layout().add(sbH);
  layout().add(sbV);

  if(sbH!=nullptr)
    sbH->valueChanged.bind(this, &ScrollWidget::scrollH);
  if(sbV!=nullptr)
    sbV->valueChanged.bind(this, &ScrollWidget::scrollV);

  setHscrollViewMode(this->hor);
  setVscrollViewMode(this->vert);

  updateScrolls();
  }

void ScrollWidget::initializeList() {
  if(list!=nullptr)
    return;

  delete cen;
  list = new ListView(orient);
  cen  = list;
  helper.layout().add(cen);
  list->onItemListChanged.bind(this,&ScrollWidget::updateScrolls);
  }

void ScrollWidget::updateScrolls() {
  using std::max;
  using std::min;

  const std::vector<Widget*>& wx = cen->layout().widgets();
  const Widget* first = wx.size()>0 ? wx[0]     : nullptr;
  const Widget* last  = wx.size()>0 ? wx.back() : nullptr;

  bool hasScH = (hor ==AlwaysOn || cen->w()>w());
  bool hasScV = (vert==AlwaysOn || cen->h()>h());

  for(bool recalc=true;recalc;){
    recalc = false;
    if(hasScH){
      if(!hasScV && cen->h()>(h()-sbH->h()) && vert==AsNeed){
        hasScV = true;
        recalc = true;
        }
      }

    if(hasScV){
      if(!hasScH && cen->w()>(w()-sbV->w()) && hor==AsNeed){
        hasScH = true;
        recalc = true;
        }
      }
    }

  const Margin& m = margin();
  const int cenW = w()-m.xMargin()-(hasScV ? sbV->w() : 0);
  const int cenH = h()-m.yMargin()-(hasScH ? sbH->h() : 0);

  const int sw = sbV==nullptr ? 0 : sbV->minSize().w;
  const int sh = sbH==nullptr ? 0 : sbH->minSize().h;

  const int dx = max(cen->w()-helper.w(),0);
  const int dy = max(cen->h()-helper.h(),0);
  if(sbH!=nullptr){
    Layout::placeIn(sbH,
                    m.left, m.top+h()-sh-m.yMargin(),
                    w()-m.xMargin()-(hasScV ? sh : 0), sh);
    sbH->setVisible(hasScH && dx>0);
    if(hor==AsNeed && dx<=0)
      sbH->setValue(0);

    int maxSc = dx, minSc=0;
    if(scAfterEndH && last!=nullptr)
      maxSc = max(cen->w()-min(last->w(),helper.w()),0);
    if(scBeforeBeginH && first!=nullptr)
      minSc = max(cen->w()-min(first->w(),helper.w()),0);
    sbH->setRange( minSc,maxSc );
    }

  if(sbV!=nullptr){
    Layout::placeIn(sbV,
                    m.left+w()-sw-m.xMargin(), m.top,
                    sw, h()-m.yMargin()-(hasScH ? sw : 0));
    sbV->setVisible(hasScV && dy>0);
    if(vert==AsNeed && dy<=0)
      sbV->setValue(0);

    int maxSc = dy, minSc = 0;
    if(scAfterEndV && last!=nullptr)
      maxSc = max(cen->h()-min(last->h(),helper.h()),0);
    if(scBeforeBeginV && first!=nullptr)
      minSc = max(cen->h()-min(first->h(),helper.h()),0);
    sbV->setRange( minSc, maxSc );
    }

  Layout::placeIn(&helper, m.left, m.top, cenW, cenH);
  }

ScrollWidget::~ScrollWidget() {
  if(list!=nullptr)
    list->onItemListChanged.ubind(this,&ScrollWidget::updateScrolls);
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
  cen->setLayout(new BoxLayout(this,orient));
  helper.layout().add(cen);
  }

void ScrollWidget::setLayout(Orientation ori) {
  orient = ori;

  if(list!=nullptr)
    list->setOrientation(orient); else
    cen->setLayout(new BoxLayout(this,ori));
  }

void ScrollWidget::hideScrollBars() {
  setScrollBarsVisible(0,0);
  }

void ScrollWidget::setScrollBarsVisible(bool vh, bool vv) {
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
  scAfterEndH = s;
  updateScrolls();
  }

bool ScrollWidget::hasScrollAfterEndH() const {
  return scAfterEndH;
  }

void ScrollWidget::scrollBeforeBeginH(bool s) {
  scBeforeBeginH = s;
  updateScrolls();
  }

bool ScrollWidget::hasScrollBeforeBeginH() const {
  return scBeforeBeginH;
  }

void ScrollWidget::scrollAfterEndV(bool s) {
  scAfterEndV = s;
  updateScrolls();
  }

bool ScrollWidget::hasScrollAfterEndV() const {
  return scAfterEndV;
  }

void ScrollWidget::scrollBeforeBeginV(bool s) {
  scBeforeBeginV = s;
  updateScrolls();
  }

bool ScrollWidget::hasScrollBeforeBeginV() const {
  return scBeforeBeginV;
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

  if( e.gestureType()==Tempest::AbstractGestureEvent::gtDragGesture ){
    Tempest::DragGesture &d = (Tempest::DragGesture&)(e);
    if(sbH!=nullptr && sbH->range()>w() && !sbH->isVisible()){
      int v = sbH->value();
      int dpos = d.dpos.x;

      sbH->setValue(sbH->value() - dpos );
      if( v!=sbH->value() )
        e.accept();
      }
    if(sbV!=nullptr && sbV->range()>h() && !sbV->isVisible()){
      int v = sbV->value();
      int dpos = d.dpos.y;

      sbV->setValue(sbV->value() - dpos );
      if( v!=sbV->value() )
        e.accept();
      }
    }
  }

void ScrollWidget::resizeEvent(SizeEvent &) {
  resizeEv(helper.w(), helper.h());
  }

ScrollBar *ScrollWidget::createScrollBar(Orientation ori) {
  return new ScrollBar(ori);
  }

void ScrollWidget::scrollH( int v ) {
  if(sbH!=nullptr){
    sbH->setValue( v );
    cen->setPosition(-sbH->value(), cen->y());
    } else
    cen->setPosition(cen->x(), -v);
  }

void ScrollWidget::scrollV(int v) {
  if(sbV!=nullptr){
    sbV->setValue( v );
    cen->setPosition(cen->x(), -sbV->value());
    } else
    cen->setPosition(cen->x(), -v);
  }

void ScrollWidget::resizeEv(int,int) {
  updateScrolls();
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
