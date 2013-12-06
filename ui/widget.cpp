#include "widget.h"

#include <Tempest/Layout>
#include <Tempest/Painter>
#include <Tempest/Shortcut>
#include <Tempest/Assert>

#include <algorithm>
#include <functional>

using namespace Tempest;

struct Widget::DeleteGuard{
  DeleteGuard( Widget* w):w(w){
    if( w->deleteLaterFlag!=-1 )
      w->lockDelete();
    }

  ~DeleteGuard(){
    if( w->deleteLaterFlag!=-1 )
      w->unlockDelete();
    }

  Widget *w;
  };

size_t Widget::count = 0;

Widget::Widget(ResourceContext *context):
  fpolicy(BackgroundFocus), rcontext(context), deleteLaterFlagGuard(0){
  ++Widget::count;

  parentLay    = 0;
  focus        = false;
  chFocus      = false;
  uscissor     = true;
  nToUpdate    = false;
  multiPaint   = false;
  multiTouch   = false;

  deleteLaterFlag = false;

#ifdef __ANDROID__
  mouseReleseReciver.reserve(8);
#else
  mouseReleseReciver.reserve(1);
#endif
  wvisible = true;
  mlay = 0;
  setLayout( new Layout() );

  update();
  }

Widget::~Widget() {
  T_ASSERT_X( deleteLaterFlagGuard==0, "bad time to delete, use deleteLater");
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

  update();
  }

void Widget::setPosition(const Point &p) {
  setPosition( p.x, p.y );
  }

void Widget::resize(int w, int h) {
  int ow = wrect.w, oh = wrect.h;

  if( ow==w && oh==h )
    return;

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

  layout().applyLayout();

  if( ox!=r.x && oy!=r.y )
    onPositionChange(ox,oy);

  if( ow!=r.w && ow!=r.w )
    onResize(ow,oh);

  update();
  }

void Widget::setGeometry(int x, int y, int w, int h) {
  setGeometry( Rect(x,y,w,h) );
  }

void Widget::setMaximumSize(const Size &s) {
  sp.maxSize = s;

  if( owner() )
    owner()->layout().applyLayout();
  }

void Widget::setMinimumSize(const Size &s) {
  sp.minSize = s;

  if( owner() )
    owner()->layout().applyLayout();
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
  if( !hasMultiPassPaint() && pe.pass )
    return;

  nToUpdate = false;
  paintNested(pe);
  }

Widget* Widget::impl_mouseEvent( Tempest::MouseEvent & e,
                                 Widget* root,
                                 void (Widget::*f)(Tempest::MouseEvent &),
                                 bool focus,
                                 bool mpress ){
  DeleteGuard guard(root);
  (void)guard;

  if( !root->isVisible() )
    return 0;

  for( size_t i=root->layout().widgets().size(); i>=1; --i ){
    Widget* w = root->layout().widgets()[i-1];

    if( !w->isScissorUsed() ||
        w->rect().contains( e.x, e.y, true ) ){
      MouseEvent et( e.x - w->x(),
                     e.y - w->y(),
                     e.button,
                     e.delta,
                     e.mouseID );
      et.ignore();

      Widget* deep = impl_mouseEvent(et, w, f, focus, mpress);
      if( et.isAccepted() ){
        e.accept();
        if( mpress ){
          if( w->mouseReleseReciver.size() < size_t(et.mouseID+1) )
            w->mouseReleseReciver.resize( et.mouseID+1 );
          w->mouseReleseReciver[et.mouseID] = deep;
          }
        return w;
        }

      if( w->isVisible() && (et.mouseID==0 || root->multiTouch) ){
        et.accept();
        (w->*f)( et );
        }

      if( et.isAccepted() ){
        e.accept();
        if( mpress ){
          if( root->mouseReleseReciver.size() < size_t(et.mouseID+1) )
            root->mouseReleseReciver.resize( et.mouseID+1 );
          root->mouseReleseReciver[et.mouseID] = w;
          }

        if( focus && w->focusPolicy()==ClickFocus )
          w->setFocus(1);

        return w;
        }
      }
    }

  e.ignore();
  return 0;
  }

void Widget::lockDelete() {
  ++deleteLaterFlagGuard;
  }

void Widget::unlockDelete() {
  --deleteLaterFlagGuard;

  if( deleteLaterFlagGuard==0 && deleteLaterFlag )
    delete this;
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

void Widget::mouseWheelEvent(MouseEvent &e){
  e.ignore();
  }

void Widget::keyDownEvent(KeyEvent &e){
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

void Widget::update() {
  if( nToUpdate || wvisible==false )
    return;

  nToUpdate = true;
  intentToUpdate();

  if( owner() )
    owner()->update();
  }

bool Widget::needToUpdate() const {
  return nToUpdate;
  }

void Widget::setMultiPassPaint(bool a) {
  multiPaint = a;
  }

bool Widget::hasMultiPassPaint() const {
  return multiPaint;
  }

void Widget::impl_keyPressEvent( Widget *wd, KeyEvent &e,
                                 void (Widget::*f)(Tempest::KeyEvent &) ) {
  DeleteGuard guard(wd);
  (void)guard;

  const std::vector<Widget*> & w = wd->layout().widgets();

  for( size_t i=w.size(); i>0; --i ){
    if( w[i-1]->hasFocus() ){
      e.accept();
      (w[i-1]->*f)(e);
      if( e.isAccepted() )
        return;
      }

    if( w[i-1]->chFocus ){
      impl_keyPressEvent(w[i-1], e, f);
      }
    }

  // e.ignore();
  }

void Widget::impl_customEvent( Widget*w, CustomEvent &e ) {
  DeleteGuard guard(w);
  (void)guard;

  size_t sz = w->layout().widgets().size();
  for( size_t i=0; i<sz; ++i ){
    Widget *wx = w->layout().widgets()[sz-i-1];
    wx->customEvent(e);
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
  w->closeEvent(e);
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
    w->gestureEvent(e);
    }
  }

void Widget::paintNested( PaintEvent &p ){
  Rect scissor = p.painter.scissor();

  const std::vector<Widget*> & w = layout().widgets();
  Rect s;

  for( size_t i=0; i<w.size(); ++i ){
    if( w[i]->wvisible &&
        !(w[i]->uscissor &&
          p.painter.scissor().intersected(w[i]->rect()).isEmpty()) &&
        !(!w[i]->hasMultiPassPaint() && p.pass) ){
      Tempest::Point pt = w[i]->pos();

      bool uscis = w[i]->uscissor;
      if( uscis ){
        s = scissor.intersected( w[i]->wrect );
        p.painter.setScissor( s );
        }

      w[i]->nToUpdate = false;
      if( !( uscis && s.isEmpty() ) ){
        p.painter.translate(  pt.x,  pt.y );
        w[i]->paintEvent(p);
        p.painter.translate( -pt.x, -pt.y );
        }

      if( uscis ){
        p.painter.setScissor( scissor );
        }
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
                                                   &Widget::mouseDownEvent,
                                                   true,
                                                   true );

  if( !e.isAccepted() && (e.mouseID==0 || hasMultitouch()))
    this->mouseDownEvent(e);
  }

void Widget::rootMouseDragEvent(MouseEvent &e) {
  DeleteGuard guard(this);
  (void)guard;

  e.ignore();

  if( size_t(e.mouseID) < mouseReleseReciver.size() && mouseReleseReciver[e.mouseID] )
    impl_mouseDragEvent( this, e );

  if( !e.isAccepted() && (e.mouseID==0 || hasMultitouch()) ){
    e.accept();
    this->mouseDragEvent(e);
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
    this->mouseUpEvent(e);
    }
  }

void Widget::impl_mouseDragEvent( Widget* w, Tempest::MouseEvent & e ){
  DeleteGuard guard(w);
  (void)guard;

  if( !( size_t(e.mouseID) < w->mouseReleseReciver.size() && w->mouseReleseReciver[e.mouseID]) ){
    w->mouseDragEvent(e);
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
    Tempest::MouseEvent ex( e.x - r->x(), e.y - r->y(), e.button, e.delta, e.mouseID );

    impl_mouseDragEvent( r, ex);
    }
  }

void Widget::rootMouseMoveEvent(MouseEvent &e) {
  e.ignore();
  impl_mouseEvent( e, this, &Widget::mouseMoveEvent, false, false );

  if( !e.isAccepted() && (e.mouseID==0 || hasMultitouch()) ){
    e.accept();
    this->mouseMoveEvent(e);
    }
  }

void Widget::rootMouseWheelEvent(MouseEvent &e) {
  e.ignore();
  impl_mouseEvent( e, this, &Widget::mouseWheelEvent, true, true );

  if( !e.isAccepted() && (e.mouseID==0 || hasMultitouch()) ){
    e.accept();
    this->mouseWheelEvent(e);
    }
  }

void Widget::rootKeyDownEvent(KeyEvent &e) {
  if( chFocus && !focus ){
    e.accept();
    impl_keyPressEvent( this, e, &Widget::keyDownEvent );
    } else {
    e.ignore();
    }

  if( !e.isAccepted() ){
    e.accept();
    this->keyDownEvent(e);
    }
  }

void Widget::rootKeyUpEvent(KeyEvent &e) {
  if( chFocus && !focus ){
    e.accept();
    impl_keyPressEvent( this, e, &Widget::keyUpEvent );
    } else {
    e.ignore();
    }

  if( !e.isAccepted() ){
    e.accept();
    this->keyUpEvent(e);
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

void Widget::impl_mouseUpEvent( Widget* w, Tempest::MouseEvent & e ){
  DeleteGuard guard(w);
  (void)guard;

  if( !( size_t(e.mouseID) < w->mouseReleseReciver.size() && w->mouseReleseReciver[e.mouseID]) ){
    w->mouseUpEvent(e);
    return;
    }

  Widget * rec = 0;
  if( size_t(e.mouseID) < w->mouseReleseReciver.size() )
    rec = w->mouseReleseReciver[e.mouseID];

  if( rec ){
    Widget * r = rec;
    Tempest::MouseEvent ex( e.x - r->x(), e.y - r->y(), e.button, e.delta );

    impl_mouseUpEvent( r, ex );

    if( ex.isAccepted() )
      e.accept(); else
      e.ignore();

    if( size_t(e.mouseID) < w->mouseReleseReciver.size() )
      w->mouseReleseReciver[e.mouseID] = 0;
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

  update();

  wvisible = v;
  if( !wvisible )
    nToUpdate = false;

  if( owner() )
    owner()->layout().applyLayout();

  update();
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

void Widget::setFocus(bool f) {
  DeleteGuard g(this);
  (void)g;

  if( focus!=f ){
    if( f ){
      unsetChFocus( this, this );

      Widget * root = this, *proot = this;

      while( root && !root->chFocus ){
        if( root->hasFocus() ){
          root->focus = 0;
          root->onFocusChange( 0 );
          }

        if( !root->chFocus && root!=this ){
          root->chFocus = 1;
          root->onChildFocusChange(1);
          }

        if( root->owner() ){
          proot = root;
          root  = root->owner();
          }
        }

      if( root->chFocus ){
        DeleteGuard g(root);
        (void)g;

        for( size_t i=0; i<root->layout().widgets().size(); ++i ){
          if( root->layout().widgets()[i]!=proot )
            unsetChFocus( root->layout().widgets()[i], root );
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
    onFocusChange(f);
    }
  }

void Widget::unsetChFocus( Widget* root, Widget* emiter ){  
  DeleteGuard g(root);
  (void)g;

  if( root!=emiter && root->focus ){
    root->focus = 0;
    root->onFocusChange(0);
    }

  if( root->chFocus ){
    root->chFocus = 0;
    root->onChildFocusChange(0);
    }

  const std::vector<Widget*>& lx = root->layout().widgets();
  for( size_t i=0; i<lx.size(); ++i )
    unsetChFocus( lx[i], emiter );
  }

void Widget::deleteLater() {
  deleteLaterFlag = true;

  if( deleteLaterFlagGuard==0 )
    delete this;
  }

void Widget::shortcutEvent(KeyEvent &e) {
  if( !isVisible() ){
    e.ignore();
    return;
    }

  size_t sz = layout().widgets().size()-1;
  for( size_t i=0; i<layout().widgets().size(); ++i ){
    layout().widgets()[sz-i]->shortcutEvent(e);
    if( e.isAccepted() )
      return;
    }

  for( size_t i=0; i<skuts.size(); ++i )
    if( (skuts[i]->key()==e.key &&
         e.key != KeyEvent::K_NoKey ) ||
        (e.key == KeyEvent::K_NoKey &&
         skuts[i]->lkey()==e.u16 &&
         e.u16!=0 )){
      e.accept();
      skuts[i]->activated();
      return;
      }

  e.ignore();
  }

void Widget::resizeEvent( SizeEvent &e ) {
  e.ignore();
  }

void Widget::gestureEvent(AbstractGestureEvent &e) {
  e.ignore();
  }



