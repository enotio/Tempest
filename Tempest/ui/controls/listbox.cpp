#include "listbox.h"

#include <Tempest/Panel>
#include <Tempest/ScroolWidget>
#include <Tempest/Layout>
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

ListBox::ListBox() : AbstractListBox() {
  selected = 0;
  }

void ListBox::setItemList( const std::vector<std::string> &list ) {
  data.resize( list.size() );
  for(size_t i = 0; i < list.size(); ++i)
    data[i].assign( list[i].begin(), list[i].end() );

  if (data.size())
    setText( data[0] ); else
    setText( "" );

  selected = 0;
  onItem( 0 );
  }

void ListBox::setItemList(const std::vector<std::u16string> &list) {
  data = list;

  if( data.size() )
    setText( data[0] ); else
    setText( "" );

  selected = 0;
  onItem(0);
  }

const std::vector<std::u16string> &ListBox::items() const {
  return data;
  }

void ListBox::setCurrentItem(size_t i) {
  if( data.size()==0 )
    return;

  i = std::min( data.size()-1, i );
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

  if( data.size() ){
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
  box->resize(160*ui.uiScale, 200*ui.uiScale);

  ScroolWidget *sw = new ScroolWidget();

  int h = box->layout().margin().yMargin();
  for( size_t i=0; i<data.size(); ++i ){
    ItemBtn * b = new ItemBtn(i);
    b->setText( data[i] );
    b->clickedEx.bind( *this, &ListBox::onItem );

    sw->centralWidget().layout().add( b );
    h += b->sizePolicy().maxSize.h;
    }

  h += data.size()*sw->centralWidget().layout().spacing();

  if( h<box->h() ){
    sw->setScroolBarVisible(0);
    box->resize( box->w(), h );
    }

  box->layout().add(sw);
  return box;
  }

void ListBox::onItem(size_t id) {
  setText( data[id] );

  onItemSelected(id);
  onItemSelectedW( data[id] );
  selected = id;
  close();
  }
