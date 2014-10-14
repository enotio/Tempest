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
      clicked.bind( this, &ItemBtn::onClick );
      }

    size_t id;
    void onClick(){
      clickedEx(id);
      }

    Tempest::signal<size_t> clickedEx;
  };

ListBox::ListBox(ListDelegate &delegate)
  : AbstractListBox(), delegate(delegate) {
  selected = 0;
  }

void ListBox::setCurrentItem(size_t i) {
  if( delegate.size()==0 )
    return;

  i = std::min( delegate.size()-1, i );
  if( selected!=i ){
    selected = i;
    onItem(selected);
    }
  }

void ListBox::mouseWheelEvent(Tempest::MouseEvent &e) {
  if( !rect().contains(e.x+x(), e.y+y()) ){
    e.ignore();
    return;
    }

  if( delegate.size() ){
    if( e.delta < 0 )
      setCurrentItem(selected+1); else
    if( e.delta > 0 && selected>0 )
      setCurrentItem(selected-1);
    }
  }

Tempest::Widget* ListBox::createDropList() {
  Panel *box = new Panel();

  const UiMetrics& ui = Application::uiMetrics();

  box->setLayout( Tempest::Horizontal );
  box->layout().setMargin(6);
  box->setPosition( mapToRoot( Tempest::Point(0,h()) ) );
  //box->resize(160*ui.uiScale, 200*ui.uiScale);

  ScroolWidget *sw = new ScroolWidget();
  ListView* list   = new ListView(delegate,Vertical);
  sw->centralWidget().layout().add(list);
  sw->resize(list->w(), std::min(list->h(),box->h()));
  box->resize(160+box->margin().xMargin(), list->h()+box->margin().yMargin());

  box->layout().add(sw);
  return box;
  }

void ListBox::onItem(size_t id) {
  //setText( data[id] );
  setText( "caption" );

  onItemSelected(id);
  //onItemSelectedW( data[id] );
  //onItemSelectedW( u"caption" );
  selected = id;
  close();
  }
