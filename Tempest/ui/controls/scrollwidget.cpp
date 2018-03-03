#include "scrollwidget.h"

#include <Tempest/Platform>
#include <Tempest/Layout>

using namespace Tempest;


struct ScrollWidget::BoxLayout: public Tempest::LinearLayout {
  BoxLayout( ScrollWidget* owner,Tempest::Orientation ori ):LinearLayout(ori), sc(owner){}

  void applyLayout(){
    if(widgets().size()==0 || busy)
      return;

    busy=true;
    SizePolicy spolicy;
    spolicy.maxSize=wrapContent(true);
    spolicy.minSize=spolicy.maxSize;
    spolicy.typeH  =FixedMin;
    spolicy.typeV  =FixedMin;

    owner()->setSizePolicy(spolicy);
    LinearLayout::applyLayout();
    busy=false;
    }

  void ajustWidth(){
    const Margin& m = margin();
    const Widget* root=owner()->owner();
    SizePolicy sz=owner()->sizePolicy();

    if(orientation()==Horizontal) {
      sz.minSize.h = root->h()+m.yMargin();
      } else {
      sz.minSize.w = root->w()+m.xMargin();
      }
    sz.maxSize=sz.minSize;
    owner()->setSizePolicy(sz);
    }

  Size wrapContent(bool inParent) {
    const Widget* root=inParent ? owner()->owner() : nullptr;
    const std::vector<Widget*> w=widgets();
    int sw=0,sh=0;

    for(const Widget* wx : w)
      if( wx->isVisible() ){
        Size s = sizeHint(wx);
        if(orientation()==Horizontal){
          sw += s.w;
          sh = std::max(sh,s.h);
          } else {
          sw = std::max(sw,s.w);
          sh += s.h;
          }
        }

    const int vcount=visibleCount();
    if(orientation()==Horizontal) {
      if(vcount>0)
        sw += (vcount-1)*spacing();
      if(root)
        sh=std::max(sh,root->h());
      } else {
      if(vcount>0)
        sh += (vcount-1)*spacing();
      if(root)
        sw=std::max(sw,root->w());
      }

    const Margin& m = margin();
    sw += m.xMargin();
    sh += m.yMargin();
    return Size(sw,sh);
    }

  ScrollWidget* sc;
  bool          busy=false;
  };

struct ScrollWidget::HelperLayout : public Tempest::Layout {
  HelperLayout(ScrollWidget* sc):sc(sc){}

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
    sc->layout().applyLayout();
    }
  ScrollWidget* sc;
  };

struct ScrollWidget::MainLayout : public Tempest::Layout {
  MainLayout(ScrollWidget* sc):sc(sc){}

  void applyLayout(){
    if(busy)
      return;

    busy=true;
    sc->helper.setGeometry(0,0,sc->w(),sc->h());
    static const int tryCound=3;
    for(int i=1;i<=tryCound;++i)
      if(sc->updateScrolls(i==tryCound))
        break;
    busy=false;
    }

  ScrollWidget* sc;
  bool          busy=false;
  };


ScrollWidget::ScrollWidget()
  : sbH(nullptr), sbV(nullptr), vert(AsNeed), hor(AsNeed) {
  cen = new Widget();

  const Style::UIIntefaceCategory cat=style().idiom().category;
  if( cat==Style::UIIntefacePhone || cat==Style::UIIntefacePC ){
    vert=AlwaysOff;
    hor =AlwaysOff;
    }

  sbH = new ScrollBar(Horizontal);
  sbV = new ScrollBar(Vertical  );

  sbH->onValueChanged.bind(this, &ScrollWidget::scrollH);
  sbV->onValueChanged.bind(this, &ScrollWidget::scrollV);

  setHscrollViewMode(this->hor );
  setVscrollViewMode(this->vert);

  helper.layout().add(cen);
  layout().add(&helper);
  layout().add(sbH);
  layout().add(sbV);

  cen   ->setLayout(new BoxLayout(this,orient));
  helper. setLayout(new HelperLayout(this));
  Widget::setLayout(new MainLayout(this));
  }

void ScrollWidget::initializeList() {
  if(list!=nullptr)
    return;

  delete cen;
  list = new ListView(orient);
  cen  = list;
  helper.layout().add(cen);
  list->onItemListChanged.bind(this,&ScrollWidget::recalcLayout);
  }

void ScrollWidget::recalcLayout() {
  layout().applyLayout();
  }

bool ScrollWidget::updateScrolls(bool noRetry) {
  using std::max;
  using std::min;

  const Margin& m     = margin();
  const Widget* first = findFirst();
  const Widget* last  = findLast();
  BoxLayout& cenLay   = *reinterpret_cast<BoxLayout*>(&cen->layout());

  Size content=cenLay.wrapContent(false);

  bool needScH = (hor ==AlwaysOn || content.w>helper.w());
  bool needScV = (vert==AlwaysOn || content.h>helper.h());
  bool hasScH  = needScH && (hor !=AlwaysOff);
  bool hasScV  = needScV && (vert!=AlwaysOff);

  emplace(helper,
          hasScH ? sbH : nullptr,
          hasScV ? sbV : nullptr,
          Rect(m.left,m.top,w()-m.xMargin(),h()-m.yMargin()));

  const Size hsize=helper.size();
  if( !noRetry ) {
    if(hasScH != (hor ==AlwaysOn || content.w>hsize.w) && (hor!=AlwaysOff))
      return false;
    if(hasScV != (vert==AlwaysOn || content.h>hsize.h) && (vert!=AlwaysOff))
      return false;
    }

  if( !needScH && !needScV ) {
    sbH->setValue(0);
    sbV->setValue(0);
    cen->setPosition(0,0);
    } else
  if( !needScH ) {
    sbH->setValue(0);
    cen->setPosition(0,cen->y());
    } else
  if( !needScV ) {
    sbV->setValue(0);
    cen->setPosition(cen->x(),0);
    }
  sbH->setVisible(hasScH);
  sbV->setVisible(hasScV);
  cen->layout().applyLayout();

  if(orient==Vertical) {
    int min=0;
    int max=std::max(content.h-hsize.h,0);

    if(scBeforeBeginV && first)
      min-=std::max(0,hsize.h-first->h());
    if(scAfterEndV && last)
      max+=std::max(0,hsize.h-last->h());
    sbV->setRange(min,max);
    sbH->setRange(0,cen->w());
    } else {
    int min=0;
    int max=std::max(content.w-hsize.w,0);

    if(scBeforeBeginH && first)
      min-=std::max(0,hsize.w-first->w());
    if(scAfterEndV && last)
      max+=std::max(0,hsize.w-last->w());
    sbH->setRange(min,max);
    sbV->setRange(cen->h(),0);
    }

  if(sbH->range()>0) {
    const double percent=hsize.w/double(sbH->range()+hsize.w);
    sbH->setCentralButtonSize(int(sbH->centralAreaSize()*percent));
    }
  if(sbV->range()>0) {
    const double percent=hsize.h/double(sbV->range()+hsize.h);
    sbV->setCentralButtonSize(int(sbV->centralAreaSize()*percent));
    }

  return true;
  }

void ScrollWidget::emplace(Widget &cen, Widget *scH, Widget *scV, const Rect& place) {
  int sp=spacing();
  int dx=scV==nullptr ? 0 : (scV->minSize().w);
  int dy=scH==nullptr ? 0 : (scH->minSize().h);

  if(scH)
    scH->setGeometry(place.x,place.y+place.h-dy,place.w-dx,dy);
  if(scV)
    scV->setGeometry(place.x+place.w-dx,place.y,dx,place.h-dy);

  if(dx>0)
    dx+=sp;
  if(dy>0)
    dy+=sp;
  cen.setGeometry(place.x,place.y,place.w-dx,place.h-dy);
  }

Widget *ScrollWidget::findFirst() {
  const std::vector<Widget*>& wx = cen->layout().widgets();
  for(size_t i=0;i<wx.size();i++)
    if( wx[i]->isVisible() )
      return wx[i];
  return nullptr;
  }

Widget *ScrollWidget::findLast() {
  const std::vector<Widget*>& wx = cen->layout().widgets();
  for(size_t i=wx.size();i>0;){
    i--;
    if( wx[i]->isVisible() )
      return wx[i];
    }
  return nullptr;
  }

ScrollWidget::~ScrollWidget() {
  if(MainLayout* m=dynamic_cast<MainLayout*>(&layout()))
    m->busy=true;

  if(list!=nullptr)
    list->onItemListChanged.ubind(this,&ScrollWidget::recalcLayout);
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

void ScrollWidget::setLayout(Layout *l) {
  if(LinearLayout* ll=dynamic_cast<LinearLayout*>(l)) {
    orient = ll->orientation();

    if(list!=nullptr)
      list->setOrientation(orient); else
      cen->setLayout(new BoxLayout(this,orient));
    }
  delete l;
  }

void ScrollWidget::hideScrollBars() {
  setScrollBarsVisible(false,false);
  }

void ScrollWidget::setScrollBarsVisible(bool vh, bool vv) {
  if( sbH==nullptr || sbV==nullptr )
    return;

  if( vh==sbH->isVisible() && vv==sbV->isVisible() )
    return;

  sbH->setVisible(vh);
  sbV->setVisible(vv);
  }

void ScrollWidget::setVscrollViewMode(scrollViewMode mode) {
  if(sbH==nullptr)
    return;

  vert = mode;
  layout().applyLayout();
  }

void ScrollWidget::setHscrollViewMode(scrollViewMode mode) {
  if(sbH==nullptr)
    return;

  hor = mode;
  layout().applyLayout();
  }

void ScrollWidget::scrollAfterEndH(bool s) {
  scAfterEndH = s;
  layout().applyLayout();
  }

bool ScrollWidget::hasScrollAfterEndH() const {
  return scAfterEndH;
  }

void ScrollWidget::scrollBeforeBeginH(bool s) {
  scBeforeBeginH = s;
  layout().applyLayout();
  }

bool ScrollWidget::hasScrollBeforeBeginH() const {
  return scBeforeBeginH;
  }

void ScrollWidget::scrollAfterEndV(bool s) {
  scAfterEndV = s;
  layout().applyLayout();
  }

bool ScrollWidget::hasScrollAfterEndV() const {
  return scAfterEndV;
  }

void ScrollWidget::scrollBeforeBeginV(bool s) {
  scBeforeBeginV = s;
  layout().applyLayout();
  }

bool ScrollWidget::hasScrollBeforeBeginV() const {
  return scBeforeBeginV;
  }

void ScrollWidget::mouseWheelEvent(Tempest::MouseEvent &e) {
  if(!isEnabled())
    return;

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

  if( e.gestureType()==Tempest::AbstractGestureEvent::gtDragGesture && helper.rect().contains(e.hotSpot()) ){
    Tempest::DragGesture &d = reinterpret_cast<Tempest::DragGesture&>(e);
    if(sbH!=nullptr && cen->w()>w() && style().idiom().touch){
      int v = sbH->value();
      int dpos = d.dpos.x;

      sbH->setValue(sbH->value() - dpos );
      if( v!=sbH->value() )
        e.accept();
      }
    if(sbV!=nullptr && cen->h()>h() && style().idiom().touch){
      int v = sbV->value();
      int dpos = d.dpos.y;

      sbV->setValue(sbV->value() - dpos );
      if( v!=sbV->value() )
        e.accept();
      }
    }
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

int ScrollWidget::scrollH() const {
  return -cen->x();
  }

int ScrollWidget::scrollV() const {
  return -cen->y();
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
