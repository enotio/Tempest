#include "widget.h"

#include <Tempest/Layout>
#include <Tempest/Painter>
#include <Tempest/Shortcut>
#include <Tempest/Assert>

#include <algorithm>
#include <functional>

using namespace Tempest;

/// \cond HIDDEN_SYMBOLS
struct Widget::DeleteGuard{
  DeleteGuard( Widget* w):w(w){
    w->lockDelete();
    }

  ~DeleteGuard(){
    w->unlockDelete();
    }

  Widget *w;
  };
/// \endcond

size_t Widget::count = 0;

Widget::Widget(ResourceContext *context):
  fpolicy(NoFocus), rcontext(context), deleteLaterFlagGuard(0){
  ++Widget::count;

  parentLay    = 0;
  focus        = false;
  chFocus      = false;
  uscissor     = true;
  nToUpdate    = false;
  multiTouch   = false;

  deleteLaterFlag = false;

#ifdef __MOBILE_PLATFORM__
  mouseReleseReciver.reserve(8);
#else
  mouseReleseReciver.reserve(1);
#endif
  mouseLeaveReciver.reserve(1);
  wvisible = true;
  mlay = 0;
  setLayout( new Layout() );

  update();
  }

Widget::~Widget() {
  T_ASSERT_X( deleteLaterFlagGuard<=0, "bad time to delete, use deleteLater");
  deleteLaterFlagGuard = -1;

  if( parentLayout() )
    parentLayout()->take(this);

  for( size_t i=0; i<skuts.size(); ++i ){
    Shortcut *s = skuts[i];
    s->m.owner = 0;
    delete s;
    }

  onDestroy( this );
  delete mlay;

  --Widget::count;
  }

int Widget::x() const {
  return wrect.x;
  }

int Widget::y() const {
  return wrect.y;
  }

int Widget::w() const {
  return wrect.w;
  }

int Widget::h() const {
  return wrect.h;
  }

Point Widget::pos() const {
  return wrect.pos();
  }

Rect Widget::rect() const {
  return wrect;
  }

Size Widget::size() const {
  return wrect.size();
  }

void Widget::setPosition(int x, int y) {
  int ox = wrect.x, oy = wrect.y;

  if( ox==x && oy==y )
    return;

  wrect.x = x;
  wrect.y = y;
  onPositionChange(ox,oy);

  nToUpdate = false;
  update();
  }

void Widget::setPosition(const Point &p) {
  setPosition( p.x, p.y );
  }

void Widget::resize(int w, int h) {
  if(w<0)
    w=0;
  if(h<0)
    h=0;

  int ow = wrect.w, oh = wrect.h;

  if( ow==w && oh==h )
    return;

  nToUpdate &= wrect.w>0 && wrect.h>0;
  wrect.w = w;
  wrect.h = h;

  layout().applyLayout();
  onResize(ow,oh);

  SizeEvent e(w,h);
  resizeEvent( e );

  update();
  }

void Widget::resize(const Size &s) {
  resize( s.w, s.h );
  }

void Widget::setGeometry(const Rect &r) {
  if( r==wrect )
    return;

  int ox = wrect.x, oy = wrect.y, ow = wrect.w, oh = wrect.h;
  wrect = r;
  if(wrect.w<0)
    wrect.w = 0;
  if(wrect.h<0)
    wrect.h = 0;

  if( ox!=r.x && oy!=r.y )
    onPositionChange(ox,oy);

  if( ow!=r.w || oh!=r.h ){
    layout().applyLayout();
    onResize(ow,oh);
    SizeEvent e(w(),h());
    resizeEvent( e );
    }

  update();
  }

void Widget::setGeometry(int x, int y, int w, int h) {
  setGeometry( Rect(x,y,w,h) );
  }

void Widget::setMaximumSize(const Size &s) {
  if(sp.maxSize==s)
    return;

  sp.maxSize = s;

  if( owner() )
    owner()->layout().applyLayout();

  if( wrect.w > s.w || wrect.h > s.h )
    setGeometry( wrect.x, wrect.y, std::min(s.w, wrect.w), std::min(s.h, wrect.h) );
  }

void Widget::setMinimumSize(const Size &s) {
  if(sp.minSize==s)
    return;

  sp.minSize = s;
  if( owner() )
    owner()->layout().applyLayout();

  if( wrect.w < s.w || wrect.h < s.h )
    setGeometry( wrect.x, wrect.y, std::max(s.w, wrect.w), std::max(s.h, wrect.h) );
  }

void Widget::setMaximumSize(int w, int h) {
  setMaximumSize( Size(w,h) );
  }

void Widget::setMinimumSize(int w, int h) {
  setMinimumSize( Size(w,h) );
  }

Size Widget::minSize() const {
  return sizePolicy().minSize;
  }

Size Widget::maxSize() const {
  return sizePolicy().maxSize;
  }

const SizePolicy Widget::sizePolicy() const {
  return sp;
  }

void Widget::setSizePolicy(const SizePolicy &s) {
  sp = s;

  if( owner() )
    owner()->layout().applyLayout();
  }

void Widget::setSizePolicy(SizePolicyType f) {
  setSizePolicy(f,f);
  }

void Widget::setSizePolicy(SizePolicyType f0, SizePolicyType f1) {
  sp.typeH = f0;
  sp.typeV = f1;

  if( owner() )
    owner()->layout().applyLayout();
  }

void Widget::setSpacing(int s) {
  layout().setSpacing(s);
  }

int Widget::spacing() const {
  return layout().spacing();
  }

void Widget::setMargin(const Margin &m) {
  return layout().setMargin(m);
  }

void Widget::setMargin(int l, int r, int t, int b) {
  return layout().setMargin(l,r,t,b);
  }

const Margin &Widget::margin() const {
  return layout().margin();
  }

FocusPolicy Widget::focusPolicy() const {
  return fpolicy;
  }

void Widget::setFocusPolicy( FocusPolicy f ) {
  fpolicy = f;
  }

void Widget::setLayout(Orientation ori) {
  setLayout( new LinearLayout(ori) );
  }

void Widget::setLayout(Layout *l) {
  l->swap( mlay );

  delete mlay;
  mlay = l;
  l->rebind(this);

  l->applyLayout();
  }

Layout &Widget::layout() {
  return *mlay;
  }

const Layout &Widget::layout() const {
  return *mlay;
  }

Layout *Widget::parentLayout() {
  return parentLay;
  }

const Layout *Widget::parentLayout() const {
  return parentLay;
  }

Widget *Widget::owner() {
  if( parentLay )
    return parentLay->owner(); else
    return 0;
  }

const Widget *Widget::owner() const {
  if( parentLay )
    return parentLay->owner(); else
    return 0;
  }

bool Widget::hasFocus() const {
  return focus;
  }

bool Widget::hasChildFocus() const {
  return chFocus;
  }

void Widget::useScissor(bool u) {
  uscissor = u;
  }

bool Widget::isScissorUsed() const {
  return uscissor;
  }

Point Widget::mapToRoot(const Point &p) const {
  if( owner()==0 )
    return p;

  return owner()->mapToRoot( p + pos() );
  }

void Widget::paintEvent( PaintEvent &pe ) {
  if( pe.pass )
    return;

  nToUpdate = false;
  paintNested(pe);
  }

void Widget::multiPaintEvent(PaintEvent &pe) {
  paintNested(pe);
  }

Widget* Widget::impl_mouseMoveEvent(Widget *root, MouseEvent &e) {
  DeleteGuard guard(root);
  (void)guard;

  if( !root->isVisible() )
    return 0;

  for( size_t i=root->layout().widgets().size(); i>=1; --i ){
    Widget* w = root->layout().widgets()[i-1];
    DeleteGuard guard(w);
    (void)guard;

    if( !w->isScissorUsed() ||
        w->rect().contains( e.x, e.y, true ) ){
      MouseEvent et( e.x - w->x(),
                     e.y - w->y(),
                     e.button,
                     e.delta,
                     e.mouseID,
                     e.type() );
      et.ignore();

      impl_mouseMoveEvent(w,et);

      if( et.isAccepted() ){
        e.accept();
        if(w->deleteLaterFlag)
          return nullptr; else
          return w;
        }

      if( w->isVisible() && (et.mouseID==0 || w->multiTouch) ){
        et.accept();
        w->event(et);
        }

      if( et.isAccepted() ){
        e.accept();
        impl_enterLeaveEvent(w,et,true);

        if(w->deleteLaterFlag)
          return nullptr; else
          return w;
        }
      }
    }

  e.ignore();
  return 0;
  }

void Widget::impl_enterLeaveEvent(Widget *w, MouseEvent &et, bool enter) {
  if( !w )
    return;

  Widget* r  = w->findRoot();
  size_t  id = et.mouseID;
  Point   pw = w->mapToRoot(Point());

  if( r ){
    while( id < r->mouseLeaveReciver.size() &&
           r->mouseLeaveReciver[id]!=nullptr ){
      r = r->mouseLeaveReciver[id];
      }

    if( w==r )
      return;

    Point proot = r->mapToRoot(Point());

    // leave event
    MouseEvent l(et.x-pw.x+proot.x,
                 et.y-pw.y+proot.y,
                 et.button,
                 et.delta,
                 et.mouseID,
                 Event::MouseLeave);
    r->event(l);
    r = r->owner();

    while( r ){
      if( id<r->mouseLeaveReciver.size() )
        r->mouseLeaveReciver[id] = nullptr;
      r = r->owner();
      }
    }

  // enter event
  if( !w->deleteLaterFlag && enter ){
    MouseEvent e(et.x,
                 et.y,
                 et.button,
                 et.delta,
                 et.mouseID,
                 Event::MouseEnter);
    w->event(e);

    Widget* pr = w;
    r = w->owner();
    while( r ){
      if( r->mouseLeaveReciver.size()<=id )
        r->mouseLeaveReciver.resize(id+1);
      r->mouseLeaveReciver[id] = pr;
      pr = r;
      r = r->owner();
      }
    }
  }

Widget* Widget::impl_mouseEvent( Tempest::MouseEvent & e,
                                 Widget* root,
                                 bool focus,
                                 bool mpress ){
  DeleteGuard guard(root);
  (void)guard;

  if( !root->isVisible() )
    return 0;

  for( size_t i=root->layout().widgets().size(); i>=1; --i ){
    Widget* w = root->layout().widgets()[i-1];
    DeleteGuard guard(w);
    (void)guard;

    if( !w->isScissorUsed() ||
        w->rect().contains( e.x, e.y, true ) ){
      MouseEvent et( e.x - w->x(),
                     e.y - w->y(),
                     e.button,
                     e.delta,
                     e.mouseID,
                     e.type() );
      et.ignore();

      impl_mouseEvent(et, w, focus, mpress);
      if( et.isAccepted() ){
        e.accept();
        if(w->deleteLaterFlag)
          return nullptr; else
          return w;
        }

      if( w->isVisible() && (et.mouseID==0 || w->multiTouch) ){
        et.accept();
        w->event(et);
        }

      if( et.isAccepted() ){
        e.accept();
        if( mpress )
          w->setupMouseReleasePtr(et.mouseID);

        if( focus && (w->focusPolicy()&ClickFocus) )
          w->impl_setFocus(true,Event::ClickReason);

        if(w->deleteLaterFlag)
          return nullptr; else
          return w;
        }
      }
    }

  e.ignore();
  return 0;
  }

void Widget::setupMouseReleasePtr(size_t mouseID) {
  Widget* w = this;
  while(w){
    if(w->deleteLaterFlag)
      return;
    Widget* root = w->owner();
    if(root){
      if( root->mouseReleseReciver.size() < size_t(mouseID+1) )
        root->mouseReleseReciver.resize( mouseID+1 );
      root->mouseReleseReciver[mouseID] = w;
      }
    w = root;
    }
  }

void Widget::detachMouseReleasePtr() {
  Widget* widget = this;

  while( widget->owner() ){
    bool hasNext = false;
    std::vector<Widget*> & mr = widget->owner()->mouseReleseReciver;
    for( size_t i=0; i<mr.size(); ++i )
      if( mr[i]==widget ){
        mr[i] = nullptr;
        hasNext = true;
        }
    if(!hasNext)
      return;
    widget = widget->owner();
    }
  }

void Widget::detachMouseLeavePtr() {
  Widget* widget = this;

  while( widget->owner() ){
    bool hasNext = false;
    std::vector<Widget*> & mr = widget->owner()->mouseLeaveReciver;
    for( size_t i=0; i<mr.size(); ++i )
      if( mr[i]==widget ){
        mr[i] = nullptr;
        hasNext = true;
        }
    if(!hasNext)
      return;
    widget = widget->owner();
    }
  }

void Widget::detach() {
  detachMouseReleasePtr();
  detachMouseLeavePtr();

  if(Widget* w = owner())
    impl_disableSum(this,-w->disableSum);
  }

void Widget::lockDelete() {
  if(deleteLaterFlagGuard>=0)
    ++deleteLaterFlagGuard;
  }

void Widget::unlockDelete() {
  if(deleteLaterFlagGuard>0)
    --deleteLaterFlagGuard;

  if( deleteLaterFlagGuard==0 && deleteLaterFlag ){
    deleteLaterFlagGuard = -1;
    delete this;
    }
  }

void Widget::mouseMoveEvent(MouseEvent &e){
  e.ignore();
  }

void Widget::mouseDragEvent(MouseEvent &e) {
  e.ignore();
  }

void Widget::mouseDownEvent(MouseEvent &e){
  e.ignore();
  }

void Widget::mouseUpEvent(MouseEvent &e) {
  e.ignore();
  }

void Widget::mouseEnterEvent(MouseEvent &e) {
  e.ignore();
  }

void Widget::mouseLeaveEvent(MouseEvent &e) {
  e.ignore();
  }

void Widget::mouseWheelEvent(MouseEvent &e){
  e.ignore();
  }

void Widget::keyDownEvent(KeyEvent &e){
  if( e.key==Event::K_Tab ){
    focusTraverse( !SystemAPI::isKeyPressed(Event::K_Shift) );
    return;
    }

  e.ignore();
  }

void Widget::keyUpEvent(KeyEvent &e) {
  e.ignore();
  }

void Widget::customEvent( CustomEvent &e ) {
  e.ignore();
  }

void Widget::closeEvent(CloseEvent &e) {
  e.ignore();
  }

void Widget::focusEvent(FocusEvent &e) {
  e.ignore();
  }

void Widget::update() {
  if( nToUpdate || wvisible==false )
    return;

  nToUpdate = wrect.w>0 && wrect.h>0;
  intentToUpdate();

  if( owner() )
    owner()->update();
  }

bool Widget::needToUpdate() const {
  return nToUpdate;
  }

void Widget::impl_keyPressEvent(Widget *wd, KeyEvent &e) {
  DeleteGuard guard(wd);
  (void)guard;

  const std::vector<Widget*>& w = wd->layout().widgets();

  for( size_t i=w.size(); i>0; --i ){
    Widget* wx=w[i-1];
    DeleteGuard g(wx);(void)g;

    const bool focus   = wx->hasFocus();
    const bool chFocus = wx->hasChildFocus();

    if( focus ){
      e.accept();
      wx->event(e);
      if( e.isAccepted() )
        return;
      }

    if( chFocus ){
      impl_keyPressEvent(wx, e);
      if( e.isAccepted() )
        return;
      }
    }
  }

void Widget::impl_customEvent( Widget*w, CustomEvent &e ) {
  DeleteGuard guard(w);
  (void)guard;

  size_t sz = w->layout().widgets().size();
  for( size_t i=0; i<sz; ++i ){
    Widget *wx = w->layout().widgets()[sz-i-1];
    wx->event(e);
    impl_customEvent( wx, e );
    }
  }

void Widget::impl_closeEvent(Widget *w, CloseEvent &e) {
  DeleteGuard guard(w);
  (void)guard;

  size_t sz = w->layout().widgets().size();
  for( size_t i=0; i<sz; ++i ){
    Widget *wx = w->layout().widgets()[sz-i-1];

    impl_closeEvent( wx, e );
    if( e.isAccepted() )
      return;
    }

  e.accept();
  w->event(e);
  }

void Widget::impl_gestureEvent(Widget *w, AbstractGestureEvent &e) {
  DeleteGuard guard(w);
  (void)guard;

  size_t sz = w->layout().widgets().size();
  for( size_t i=0; i<sz; ++i ){
    Widget *wx = w->layout().widgets()[sz-i-1];

    if( wx->isVisible() &&
        (!w->isScissorUsed() || wx->rect().contains(e.hotSpot())) ){
      Point h = e.hotSpot();
      e.setHotSpot( h - wx->pos() );
      impl_gestureEvent( wx, e );
      e.setHotSpot( h );

      if( e.isAccepted() )
        return;
      }
    }

  if( !e.isAccepted() ){
    e.accept();
    w->event(e);
    }
  }

void Widget::paintNested( PaintEvent &p ){
  Rect scissor = p.painter.scissor();

  const std::vector<Widget*> & w = layout().widgets();
  Rect s;

  for( size_t i=0; i<w.size(); ++i ){
    Widget *wi = w[i];
    if( wi->wvisible &&
        !(wi->uscissor &&
          p.painter.scissor().intersected(wi->rect()).isEmpty()) ){
      Tempest::Point pt = wi->pos();

      bool uscis = wi->uscissor;
      if( uscis ){
        s = scissor.intersected( wi->wrect );
        p.painter.setScissor( s );
        }

      wi->nToUpdate = false;
      if( !( uscis && s.isEmpty() ) ){
        p.painter.translate(  pt.x,  pt.y );
        wi->event(p);
        p.painter.translate( -pt.x, -pt.y );
        }

      if( uscis ){
        p.painter.setScissor( scissor );
        }
      }
    }
  }

static size_t indexof(Widget* owner,Widget* dest){
  if(owner==nullptr)
    return 0;

  auto w = owner->layout().widgets();
  for(size_t i=0; i<w.size(); ++i)
    if( w[i]==dest )
      return i;
  return 0;
  }

static Widget* nextWidget(Widget* w) {
  if( w->layout().widgets().size()>0 && w->isVisible() ) {
    return w->layout().widgets()[0];
    }

  Widget* prev = w;
  Widget* ow   = w->owner();
  while( ow ) {
    size_t id = indexof(ow,prev)+1;
    if(id<ow->layout().widgets().size()){
      Widget* target=ow->layout().widgets()[id];
      return target;
      }
    prev = ow;
    ow   = ow->owner();
    }

  return prev;
  }

static Widget* prevWidget(Widget* w) {
  if( w->layout().widgets().size()>0 && w->isVisible() ) {
    return w->layout().widgets().back();
    }

  Widget* prev = w;
  Widget* ow   = w->owner();
  while( ow ) {
    size_t id = indexof(ow,prev);
    if( id>0 ){
      Widget* target=ow->layout().widgets()[id-1];
      return target;
      }
    prev = ow;
    ow   = ow->owner();
    }

  return prev;
  }

void Widget::focusNext() {
  return focusTraverse(true);
  }

void Widget::focusPrevious() {
  return focusTraverse(false);
  }

void Widget::focusTraverse(bool forward) {
  DeleteGuard g(this);(void)g;

  Widget* w = this;
  while( w ) {
    w = forward ? nextWidget(w) : prevWidget(w);
    if( w && (w->focusPolicy()&TabFocus) && w->isEnabled() ) {
      w->impl_setFocus(true,Event::TabReason);
      return;
      }
    }
  }

void Widget::rootMouseDownEvent(MouseEvent &e) {
  DeleteGuard guard(this);
  (void)guard;

  e.ignore();

  if( mouseReleseReciver.size() < size_t(e.mouseID+1) )
    mouseReleseReciver.resize( e.mouseID+1 );

  mouseReleseReciver[e.mouseID] = impl_mouseEvent( e,
                                                   this,
                                                   true,
                                                   true );

  if( !e.isAccepted() && (e.mouseID==0 || hasMultitouch())){
    e.accept();
    this->event(e);
    if(e.isAccepted() && (this->focusPolicy()&ClickFocus) )
      this->impl_setFocus(true,Event::ClickReason);
    }
  }

void Widget::rootMouseDragEvent(MouseEvent &e) {
  DeleteGuard guard(this);
  (void)guard;

  e.ignore();

  if( size_t(e.mouseID) < mouseReleseReciver.size() && mouseReleseReciver[e.mouseID] )
    impl_mouseDragEvent( this, e );

  if( !e.isAccepted() && (e.mouseID==0 || hasMultitouch()) ){
    e.accept();
    this->event(e);
    }
  }

void Widget::rootMouseUpEvent(MouseEvent &e) {
  DeleteGuard guard(this);
  (void)guard;

  e.ignore();

  if( size_t(e.mouseID) < mouseReleseReciver.size() && mouseReleseReciver[e.mouseID] )
    impl_mouseUpEvent( this, e );

  if( !e.isAccepted() && (e.mouseID==0 || hasMultitouch()) ){
    e.accept();
    this->event(e);
    }
  }

void Widget::impl_mouseDragEvent( Widget* w, Tempest::MouseEvent & e ){
  DeleteGuard guard(w);
  (void)guard;

  if( !( size_t(e.mouseID) < w->mouseReleseReciver.size() && w->mouseReleseReciver[e.mouseID]) ){
    w->event(e);
    return;
    }

  Widget *rec = 0;
  if( size_t(e.mouseID) < w->mouseReleseReciver.size() )
    rec = w->mouseReleseReciver[e.mouseID];

  if( std::find( w->layout().widgets().begin(),
                 w->layout().widgets().end(),
                 rec )
      !=w->layout().widgets().end() ){
    Widget * r = rec;
    Tempest::MouseEvent ex( e.x - r->x(), e.y - r->y(),
                            e.button, e.delta, e.mouseID,
                            e.type() );

    impl_mouseDragEvent( r, ex);

    if( ex.isAccepted() )
      e.accept();
    }
  }

void Widget::rootMouseMoveEvent(MouseEvent &e) {
  DeleteGuard guard(this);
  (void)guard;

  e.ignore();  
  if( mouseLeaveReciver.size() < size_t(e.mouseID+1) )
    mouseLeaveReciver.resize( e.mouseID+1 );
  impl_mouseMoveEvent( this, e );

  if( !e.isAccepted() && (e.mouseID==0 || hasMultitouch()) ){
    e.accept();
    this->event(e);
    impl_enterLeaveEvent(this,e,e.isAccepted());
    }
  }

void Widget::rootMouseWheelEvent(MouseEvent &e) {
  DeleteGuard guard(this);
  (void)guard;

  e.ignore();
  impl_mouseEvent( e, this, true, false );

  if( !e.isAccepted() && (e.mouseID==0 || hasMultitouch()) ){
    e.accept();
    this->event(e);
    if(e.isAccepted() && (this->focusPolicy()&WheelFocus))
      this->impl_setFocus(true,Event::WheelReason);
    }
  }

void Widget::rootKeyDownEvent(KeyEvent &e) {
  DeleteGuard guard(this);
  (void)guard;

  if( chFocus && !focus ){
    e.accept();
    impl_keyPressEvent( this, e );
    } else {
    e.ignore();
    }

  if( !e.isAccepted() ){
    e.accept();
    this->event(e);
    }
  }

void Widget::rootKeyUpEvent(KeyEvent &e) {
  DeleteGuard guard(this);
  (void)guard;

  if( chFocus && !focus ){
    e.accept();
    impl_keyPressEvent( this, e );
    } else {
    e.ignore();
    }

  if( !e.isAccepted() ){
    e.accept();
    this->event(e);
    }
  }

void Widget::rootShortcutEvent(KeyEvent &e) {
  e.ignore();
  this->shortcutEvent(e);
  }

void Widget::rootCloseEvent(CloseEvent &e) {
  impl_closeEvent(this, e);
  }

void Widget::rootGestureEvent(AbstractGestureEvent &e) {
  e.ignore();
  impl_gestureEvent(this, e);
  }

void Widget::rootCustomEvent(CustomEvent &e) {
  e.ignore();
  impl_customEvent(this, e);
  }

void Widget::impl_mouseUpEvent( Widget* w, Tempest::MouseEvent & e ){
  DeleteGuard guard(w);
  (void)guard;

  if( !( size_t(e.mouseID) < w->mouseReleseReciver.size() &&
         w->mouseReleseReciver[e.mouseID]) ){
    w->mouseUpEvent(e);
    return;
    }

  Widget * rec = 0;
  if( size_t(e.mouseID) < w->mouseReleseReciver.size() ){
    rec = w->mouseReleseReciver[e.mouseID];
    if( rec )
      w->mouseReleseReciver[e.mouseID] = 0;
    }

  if( rec ){
    Widget * r = rec;
    Tempest::MouseEvent ex( e.x - r->x(), e.y - r->y(), e.button, e.delta, e.mouseID, e.type() );

    impl_mouseUpEvent( r, ex );

    if( ex.isAccepted() )
      e.accept(); else
      e.ignore();
    }
  }

Widget* Widget::findRoot(){
  Widget* root = this;

  while( root->owner() )
    root = root->owner();

  return root;
  }

bool Widget::isVisible() const {
  return wvisible;
  }

void Widget::setVisible(bool v) {
  if( wvisible==v )
    return;

  wvisible = v;
  if( !wvisible )
    nToUpdate = false;

  if( owner() ){
    owner()->layout().applyLayout();
    owner()->update();
    }
  }

ResourceContext *Widget::context() const {
  return rcontext;
  }

void Widget::setContext(ResourceContext *context) {
  rcontext = context;

  if( mlay )
    mlay->rebind(this);
  }

bool Widget::hasMultitouch() const {
  return multiTouch;
  }

void Widget::setMultiTouchTracking( bool m ) {
  multiTouch = m;
  }

void Widget::impl_disableSum(Widget *root,int diff) {
  root->disableSum+=diff;

  const std::vector<Widget*> & w = root->layout().widgets();

  for( Widget* wx:w )
    impl_disableSum(wx,diff);
  }

void Widget::setEnabled(bool e) {
  if(e!=disabled)
    return;
  if(disabled) {
    disabled=false;
    impl_disableSum(this,-1);
    } else {
    disabled=true;
    impl_disableSum(this,+1);
    }
  update();
  }

bool Widget::isEnabled() const {
  return disableSum==0;
  }

bool Widget::isEnabledTo(const Widget *ancestor) const {
  const Widget* w = this;
  while( w ){
    if(w->disabled)
       return false;
    if( w==ancestor )
       return true;
    w = w->owner();
    }
  return true;
  }

void Widget::setFocus(bool f) {
  impl_setFocus(f,Event::UnknownReason);
  }

void Widget::impl_setFocus(bool f, Event::FocusReason reason) {
  DeleteGuard g(this);
  (void)g;

  FocusEvent inEv  (true,reason);
  FocusEvent outEv(false,reason);

  if( focus!=f ){
    if( f ){
      unsetChFocus( this, this, reason );

      Widget * root = this, *proot = this;

      while( root && !root->chFocus ){
        if( root->hasFocus() ){
          root->focus = false;
          root->focusEvent(outEv);
          root->onFocusChange( false );
          }

        if( !root->chFocus && root!=this ){
          root->chFocus = true;
          root->onChildFocusChange(true);
          }

        if( root->owner() ){
          proot = root;
          root  = root->owner();
          } else {
          break;
          }
        }

      if( root && root->chFocus ){
        DeleteGuard g(root);
        (void)g;

        for( size_t i=0; i<root->layout().widgets().size(); ++i ){
          if( root->layout().widgets()[i]!=proot )
            unsetChFocus( root->layout().widgets()[i], root, reason );
          }
        }

      } else {
      Widget * root = this, *root_owner = root->owner();

      while( root_owner && root_owner->chFocus ){
        root = root_owner;

        DeleteGuard g(root);
        (void)g;

        root->chFocus = 0;
        root->onChildFocusChange(0);

        root_owner = root->owner();
        }
      }

    focus = f;
    focusEvent(f ? inEv : outEv);
    onFocusChange(f);
    }
  }

void Widget::unsetChFocus( Widget* root, Widget* emiter, Event::FocusReason reason ){
  DeleteGuard g(root);
  (void)g;

  if( root!=emiter && root->focus ){
    FocusEvent outEv(false,reason);
    root->focus = false;
    root->focusEvent(outEv);
    root->onFocusChange(false);
    }

  if( root->chFocus ){
    root->chFocus = false;
    root->onChildFocusChange(false);
    }

  const std::vector<Widget*>& lx = root->layout().widgets();
  for( size_t i=0; i<lx.size(); ++i )
    unsetChFocus( lx[i], emiter, reason );
  }

void Widget::deleteLater() {
  if( deleteLaterFlagGuard==0 )
    delete this;
  else
    deleteLaterFlag = true;
  }

void Widget::shortcutEvent(KeyEvent &e) {
  if( !isVisible() ){
    e.ignore();
    return;
    }

  DeleteGuard g(this);
  (void)g;

  size_t sz = layout().widgets().size()-1;
  for( size_t i=0; i<layout().widgets().size(); ++i ){
    layout().widgets()[sz-i]->event(e);
    if( e.isAccepted() )
      return;
    }

  for( Shortcut* sc:skuts ){
    if( sc->modifier()!=KeyEvent::K_NoKey && !SystemAPI::isKeyPressed(sc->modifier()) )
      continue;

    if( (SystemAPI::isKeyPressed(sc->key()) && e.key != KeyEvent::K_NoKey ) ||
        (e.key == KeyEvent::K_NoKey && sc->lkey()==e.u16 && e.u16!=0 ) ){
      e.accept();
      sc->activated();
      return;
      }
    }

  e.ignore();
  }

void Widget::resizeEvent( SizeEvent &e ) {
  e.ignore();
  }

void Widget::gestureEvent(AbstractGestureEvent &e) {
  e.ignore();
  }

void Widget::event( Event &e ) {
  switch ( e.type() ) {
    case Event::NoEvent:
      break;

    case Event::MouseDown:
      mouseDownEvent( (Tempest::MouseEvent&)e );
      break;

    case Event::MouseUp:
      mouseUpEvent( (Tempest::MouseEvent&)e );
      break;

    case Event::MouseMove:
      mouseMoveEvent( (Tempest::MouseEvent&)e );
      break;

    case Event::MouseDrag:
      mouseDragEvent( (Tempest::MouseEvent&)e );
      break;

    case Event::MouseWheel:
      mouseWheelEvent( (Tempest::MouseEvent&)e );
      break;

    case Event::MouseEnter:
      mouseEnterEvent( (Tempest::MouseEvent&)e );
      break;

    case Event::MouseLeave:
      mouseLeaveEvent( (Tempest::MouseEvent&)e );
      break;

    case Event::KeyDown:
      keyDownEvent( (Tempest::KeyEvent&)e );
      break;

    case Event::KeyUp:
      keyUpEvent( (Tempest::KeyEvent&)e );
      break;

    case Event::Resize:
      resizeEvent( (Tempest::SizeEvent&)e );
      break;

    case Event::Shortcut:
      shortcutEvent( (Tempest::KeyEvent&)e );
      break;

    case Event::Focus:
      focusEvent( (Tempest::FocusEvent&)e );
      break;

    case Event::Paint:{
      Tempest::PaintEvent& pe = (Tempest::PaintEvent&)e;
      if( pe.pass==0 )
        paintEvent( pe ); else
        multiPaintEvent( pe );
      }
      break;

    case Event::Close:
      closeEvent( (Tempest::CloseEvent&)e );
      break;

    case Event::Gesture:
      gestureEvent( (Tempest::AbstractGestureEvent&)e );
      break;

    case Event::Custom:
      customEvent( (Tempest::CustomEvent&)e );
      break;
    }
  }

