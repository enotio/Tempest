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

class ListBox::ItemBtn:public Button {
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

class ListBox::BasicButton:public Button {
  public:
    BasicButton(size_t id):id(id){}

    void emitClick() {
      onItemSelected    (id);
      onItemViewSelected(id,this);
      }

    size_t                          id;
    Tempest::signal<size_t>         onItemSelected;
    Tempest::signal<size_t,Widget*> onItemViewSelected;
  };

template<class Array>
class ListBox::BasicDelegate : public ListDelegate {
  public:
    BasicDelegate(ListBox& box,const Array& a):box(box),items(a){}

    size_t size() const { return items.size(); }

    Widget* createView(size_t position){
      return createView(position,R_Default);
      }

    Widget* createView(size_t position, Role role){
      Button* b=box.createItemButton(*this,position,role);
      initializeItem(b,items[position]);
      return b;
      }

    Widget* update( Widget* w, size_t position ){
      Button* b=dynamic_cast<Button*>(w);
      initializeItem(b,items[position]);
      return b;
      }

    template<class T>
    void initializeItem(Button* c, const T& data){
      SizePolicy p = c->sizePolicy();
      p.typeH      = Preferred;
      p.maxSize.w  = SizePolicy::maxWidgetSize().w;
      c->setSizePolicy(p);

      c->setText(data);
      c->setMinimumSize( c->font().textSize(c->text()).w + c->margin().xMargin(),
                         c->minSize().h );
      }

    ListBox& box;
    Array    items;
  };


ListBox::ListBox() : view(0) {
  selected = 0;
  dropListEnabled = true;
  }

ListBox::~ListBox() {
  removeDelegate();
  }

void ListBox::setItems(const std::vector<std::u16string> &items) {
  setDelegate(BasicDelegate<std::vector<std::u16string>>(*this,items));
  }

void ListBox::setItems(const std::vector<std::string> &items) {
  setDelegate(BasicDelegate<std::vector<std::string>>   (*this,items));
  }

void ListBox::removeDelegate() {
  if(view){
    layout().take(view);
    listDelegate->removeView(view,selected);
    view = 0;
    }

  if(listDelegate){
    listDelegate->onItemViewSelected.ubind(this, &ListBox::onItem);
    listDelegate->updateView.ubind(this,&ListBox::updateSelectedView);
    listDelegate.reset();
    }
  }

void ListBox::invalidateView() {
  if(listDelegate)
    listDelegate->invalidateView();
  }

void ListBox::updateView() {
  if(listDelegate)
    listDelegate->updateView();
  }

void ListBox::setCurrentItem(size_t i) {
  if( !listDelegate || listDelegate->size()==0 )
    return;

  i = std::min( listDelegate->size()-1, i );
  if( selected!=i ){
    auto tmp = dropListEnabled;
    dropListEnabled = false;
    onItem(i,nullptr);
    selected = i;
    dropListEnabled = tmp;
    }
  }

void ListBox::mouseWheelEvent(Tempest::MouseEvent &e) {
  if( !rect().contains(e.x+x(), e.y+y()) || !isEnabled() ){
    e.ignore();
    return;
    }

  dropListEnabled = false;
  std::shared_ptr<ListDelegate> d = listDelegate;
  if( d && d->size() ){
    Layout& l = layout();
    Widget* view = (l.widgets().size()==0 ? nullptr : l.widgets()[0]);
    if( e.delta < 0 ){
      if( selected+1<d->size() )
        d->onItemViewSelected(selected+1,view); else
        e.ignore();
      }
      else
    if( e.delta > 0 ){
      if( selected>0 )
        d->onItemViewSelected(selected-1,view); else
        e.ignore();
      }
    }
  dropListEnabled = true;
  }

void ListBox::setupView(size_t oldSelected) {
  bool focused=false;
  if(!listDelegate)
    return;

  if(view){
    focused = view->hasFocus();
    layout().take(view);
    listDelegate->removeView(view,oldSelected);
    view = 0;
    }

  if(listDelegate->size()){
    selected = std::min(selected,listDelegate->size()-1);
    view     = listDelegate->createView(selected,ListDelegate::R_ListBoxView);
    layout().add(view);
    view->setFocus(focused);
    } else {
    selected = 0;
    }
  }

void ListBox::updateSelectedView() {
  setupView(selected);
  }

const std::shared_ptr<ListDelegate> &ListBox::delegate() {
  return listDelegate;
  }

Tempest::Widget* ListBox::createDropList() {
  if(!listDelegate)
    return nullptr;

  Panel *box = new Panel();

  box->setLayout( Tempest::Horizontal );
  box->layout().setMargin(6);
  box->setPosition( mapToRoot( Tempest::Point(0,h()) ) );

  ScrollWidget *sw = new ScrollWidget();
  ListView* list   = new ListView(Vertical);
  list->setDefaultItemRole(ListDelegate::R_ListBoxItem);
  list->setDelegate(ProxyDelegate(listDelegate));

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
  listDelegate->onItemViewSelected.bind(this, &ListBox::onItem);
  listDelegate->updateView.bind(this, &ListBox::updateSelectedView);
  }

size_t Tempest::ListBox::currentItem() const {
  return selected;
  }

size_t ListBox::itemsCount() const {
  return listDelegate==nullptr ? 0 : listDelegate->size();
  }

Tempest::Button *Tempest::ListBox::createItemButton(Tempest::ListDelegate& owner,size_t pos,Tempest::ListDelegate::Role /*r*/) {
  auto b = new BasicButton(pos);
  b->onItemSelected    .bind(owner.onItemSelected);
  b->onItemViewSelected.bind(owner.onItemViewSelected);
  return b;
  }
