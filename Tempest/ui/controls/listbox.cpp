#include "listbox.h"

#include <Tempest/Panel>
#include <Tempest/ScrollWidget>
#include <Tempest/Layout>
#include <Tempest/ListDelegate>
#include <Tempest/ListView>
#include <Tempest/Application>

using namespace Tempest;

struct ListBox::ProxyDelegate : public ListDelegate {
  ProxyDelegate(const std::shared_ptr<ListDelegate>& d):delegate(d){
    d->invalidateView    .bind(invalidateView    );
    d->updateView        .bind(updateView        );
    d->onItemSelected    .bind(onItemSelected    );
    d->onItemViewSelected.bind(onItemViewSelected);
    }

  std::shared_ptr<ListDelegate> delegate;

  virtual size_t size() const{
    return delegate->size();
    }

  virtual Widget* createView(size_t position){
    return delegate->createView(position);
    }

  virtual void removeView(Widget* w, size_t position){
    return delegate->removeView(w,position);
    }
  };

class ListBox::ItemBtn:public Button{
  public:
    ItemBtn( size_t id ):id(id){
      onClicked.bind( this, &ItemBtn::onClick );
      }

    size_t id;
    void onClick(){
      clickedEx(id);
      }

    Tempest::signal<size_t> clickedEx;
  };

ListBox::ListBox() : view(0) {
  selected = 0;
  dropListEnabled = true;
  }

ListBox::~ListBox() {
  removeDelegate();
  }

void ListBox::removeDelegate() {
  if(view){
    layout().take(view);
    delegate->removeView(view,selected);
    view = 0;
    }

  if(delegate){
    delegate->onItemViewSelected.ubind(this, &ListBox::onItem);
    delegate->updateView.ubind(this,&ListBox::updateView);
    delegate.reset();
    }
  }

void ListBox::setCurrentItem(size_t i) {
  if( !delegate || delegate->size()==0 )
    return;

  i = std::min( delegate->size()-1, i );
  if( selected!=i ){
    auto tmp = dropListEnabled;
    dropListEnabled = false;
    onItem(i,nullptr);
    selected = i;
    dropListEnabled = tmp;
    }
  }

void ListBox::mouseWheelEvent(Tempest::MouseEvent &e) {
  if( !rect().contains(e.x+x(), e.y+y()) ){
    e.ignore();
    return;
    }

  dropListEnabled = false;
  if( delegate && delegate->size() ){
    Layout& l = layout();
    Widget* view = (l.widgets().size()==0 ? nullptr : l.widgets()[0]);
    if( e.delta < 0 && selected+1<delegate->size() )
      delegate->onItemViewSelected(selected+1,view);
      else
    if( e.delta > 0 && selected>0 )
      delegate->onItemViewSelected(selected-1,view);
    }
  dropListEnabled = true;
  }

void ListBox::setupView(size_t oldSelected) {
  if(!delegate)
    return;

  if(view){
    layout().take(view);
    delegate->removeView(view,oldSelected);
    view = 0;
    }

  if(delegate->size()){
    view = delegate->createView(selected,ListDelegate::R_ListBoxView);
    layout().add(view);
    }
  }

void ListBox::updateView() {
  setupView(selected);
  }

Tempest::Widget* ListBox::createDropList() {
  if(!delegate)
    return nullptr;

  Panel *box = new Panel();

  box->setLayout( Tempest::Horizontal );
  box->layout().setMargin(6);
  box->setPosition( mapToRoot( Tempest::Point(0,h()) ) );

  ScrollWidget *sw = new ScrollWidget();
  ListView* list   = new ListView(Vertical);
  list->setDelegate(ProxyDelegate(delegate));

  sw->centralWidget().layout().add(list);
  int wx = std::max(w(), list->minSize().w+box->margin().xMargin());
  box->resize(wx, list->h()+box->margin().yMargin());
  box->layout().add(sw);

  return box;
  }

void ListBox::onItem(size_t id,Widget* item) {
  Layout& l    = layout();
  Widget* view = (l.widgets().size()==0 ? nullptr : l.widgets()[0]);

  if(dropListLayer()!=nullptr || !dropListEnabled ){
    selectItem(id);
    } else {
    if(view==item)
      showList();
    }
  }

void ListBox::selectItem(size_t id) {
  size_t old = selected;
  selected = id;
  if(old!=id)
    onItemSelected(id);
  setupView(old);
  close();
  }

void ListBox::setupDelegateCallback() {
  delegate->onItemViewSelected.bind(this, &ListBox::onItem);
  delegate->updateView.bind(this, &ListBox::updateView);
  }

size_t Tempest::ListBox::currentItem() const {
  return selected;
  }
