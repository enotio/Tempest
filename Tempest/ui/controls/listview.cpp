#include "listview.h"

using namespace Tempest;

ListView::ListView(Tempest::ListDelegate &delegate) {
  setSizePolicy(Preferred,FixedMin);
  setLayout(new Layout(delegate,Vertical));
  }

ListView::Layout::Layout(ListDelegate &delegate, Orientation ori)
  :ori(ori), placedStart(0), delegate(delegate), busy(false) {
  placed.reserve(16);
  delegate.invalidateView.bind(this,&Layout::invalidate);
  }

ListView::Layout::~Layout(){
  }

void ListView::Layout::applyLayout() {
  if(busy)
    return;
  busy = true;

  Widget* ow   = owner();
  Widget* wx   = ow->owner();
  Rect r       = (wx!=nullptr) ? wx->rect().intersected(ow->rect()) : ow->rect();
  size_t count = delegate.size();

  int curH = placed.size()==0 ? 0:placed.back()->y()+placed.back()->h();
  for(size_t i=placed.size(); i<count && curH<r.h; ++i){
    Widget* w = delegate.createView(i);
    const int elementHeight = w->h();
    placeIn(w,Rect(0,i*elementHeight,ow->w(),elementHeight));
    curH += elementHeight;
    add(w);
    placed.push_back(w);
    }

  for(size_t i=placed.size(); i>0 && curH>r.y; ){
    --i;
    Widget* w = placed[i];
    const int elementHeight = w->h();
    placeIn(w,Rect(0,i*elementHeight,ow->w(),elementHeight));
    curH -= elementHeight;
    }

  busy = false;
  ow->setMinimumSize(ow->minSize().w,delegate.viewLengthTotal());
  }

Widget *ListView::Layout::widget(size_t i) {
  return delegate.createView(i);
  }

void ListView::Layout::invalidate() {
  for(size_t i=placed.size(); i>0; ){
    --i;
    Widget* w = placed[i];
    take(w);
    delegate.removeView(placed[i],i);
    }
  placed.clear();

  applyLayout();
  }
