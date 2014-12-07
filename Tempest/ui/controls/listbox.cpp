#include "listbox.h"

#include <Tempest/Panel>
#include <Tempest/ScroolWidget>
#include <Tempest/Layout>
#include <Tempest/ListDelegate>
#include <Tempest/ListView>
#include <Tempest/Application>

using namespace Tempest;

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

ListBox::ListBox(ListDelegate &delegate)
  : AbstractListBox(), delegate(delegate), view(0) {
  selected = 0;
  dropListEnabled = true;
  setupView(size_t(-1));
  delegate.onItemViewSelected.bind(this, &ListBox::onItem);
  }

void ListBox::setCurrentItem(size_t i) {
  if( delegate.size()==0 )
    return;

  i = std::min( delegate.size()-1, i );
  if( selected!=i ){
    selected = i;
    auto tmp = dropListEnabled;
    dropListEnabled = false;
    onItem(selected,nullptr);
    dropListEnabled = tmp;
    }
  }

void ListBox::mouseWheelEvent(Tempest::MouseEvent &e) {
  if( !rect().contains(e.x+x(), e.y+y()) ){
    e.ignore();
    return;
    }

  dropListEnabled = false;
  if( delegate.size() ){
    if( e.delta < 0 && selected+1<delegate.size() )
      delegate.onItemSelected(selected+1);
      else
    if( e.delta > 0 && selected>0 )
      delegate.onItemSelected(selected-1);
    }
  dropListEnabled = true;
  }

void ListBox::setupView(size_t oldSelected) {
  if(view){
    layout().take(view);
    delegate.removeView(view,oldSelected);
    view = 0;
    }

  if(delegate.size()){
    view = delegate.createView(selected);
    layout().add(view);
    }
  }

Tempest::Widget* ListBox::createDropList() {
  Panel *box = new Panel();

  box->setLayout( Tempest::Horizontal );
  box->layout().setMargin(6);
  box->setPosition( mapToRoot( Tempest::Point(0,h()) ) );

  ScroolWidget *sw = new ScroolWidget();
  ListView* list   = new ListView(delegate,Vertical);
  sw->centralWidget().layout().add(list);
  int wx = std::max(w(), list->minSize().w+box->margin().xMargin());
  box->resize(wx, list->h()+box->margin().yMargin());
  box->layout().add(sw);

  return box;
  }

void ListBox::onItem(size_t id,Widget* item) {
  Layout& l = layout();
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
  onItemSelected(id);
  selected = id;
  setupView(old);
  close();
  }
