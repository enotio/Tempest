#include "listview.h"

using namespace Tempest;

ListView::ListView(Orientation ori)
  : orientation(ori), lay(nullptr) {
  if(orientation==Horizontal)
    setSizePolicy(FixedMin,Preferred);
  else
    setSizePolicy(Preferred,FixedMin);
  }

ListView::~ListView() {
  removeDelegate();
  }

void ListView::removeDelegate() {
  if(delegate){
    delegate->onItemSelected.ubind(onItemSelected);
    removeAll();
    lay = nullptr;
    setLayout(new Tempest::Layout());
    delegate.reset();
    }
  }

void ListView::invalidateView() {
  if(delegate)
    delegate->invalidateView();
  }

void ListView::updateView() {
  if(delegate)
    delegate->updateView();
  }

void ListView::setOrientation(Orientation ori) {
  orientation = ori;
  if(orientation==Horizontal)
    setSizePolicy(FixedMin,Preferred);
  else
    setSizePolicy(Preferred,FixedMin);

  if(delegate)
    setLayout(new Layout(*this,*delegate,ori));
  }

void ListView::removeAll() {
  if(lay)
    lay->removeAll(); else
    layout().removeAll();
  }


ListView::Layout::Layout(ListView &view, ListDelegate &delegate, Orientation ori)
  :LinearLayout(ori), view(view), delegate(delegate), busy(false) {
  delegate.invalidateView.bind(this,&Layout::invalidate);
  delegate.updateView    .bind(this,&Layout::update    );
  }

ListView::Layout::~Layout(){
  delegate.invalidateView.ubind(this,&Layout::invalidate);
  delegate.updateView    .ubind(this,&Layout::update    );
  removeAll();
  }

void ListView::Layout::applyLayout() {
  if(busy)
    return;
  busy = true;

  int w=margin().xMargin(), h=margin().yMargin();

  if(widgets().size()!=delegate.size()){
    removeAll();
    if(orientation()==Horizontal){
      for(size_t i=0; i<delegate.size(); ++i){
        Widget*       view=delegate.createView(i);
        const Size    sz  =view->minSize();
        w += sz.w;
        h = std::max(h,sz.h);
        add(view);
        }
      } else {
      for(size_t i=0; i<delegate.size(); ++i){
        Widget*       view=delegate.createView(i);
        const Size    sz  =view->minSize();
        w = std::max(w,sz.w);
        h += sz.h;
        add(view);
        }
      }
    } else {
    const std::vector<Widget*>& wx = widgets();
    if(orientation()==Horizontal){
      for(size_t i=0; i<wx.size(); ++i){
        const Size sz = wx[i]->minSize();
        w += sz.w;
        h = std::max(w,sz.h);
        }
      } else {
      for(size_t i=0; i<wx.size(); ++i){
        const Size sz = wx[i]->minSize();
        w = std::max(w,sz.w);
        h += sz.h;
        }
      }
    }

  const int sp = widgets().size()==0 ? 0 : (widgets().size()-1)*spacing();
  if(orientation()==Horizontal)
    owner()->setMinimumSize(w+sp, h); else
    owner()->setMinimumSize(w, h+sp);

  LinearLayout::applyLayout();
  busy = false;
  }

void ListView::Layout::invalidate(){
  removeAll();
  applyLayout();
  view.onItemListChanged();
  }

void ListView::Layout::update() {
  applyLayout();
  view.onItemListChanged();
  }

void ListView::Layout::removeAll() {
  bool b = busy;
  busy = true;

  while(widgets().size()){
    size_t id = widgets().size()-1;
    delegate.removeView(take(widgets().back()), id);
    }

  busy = b;
  }
