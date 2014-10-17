#include "abstractlistbox.h"

#include <Tempest/Application>
#include <Tempest/Window>
#include <Tempest/Layout>
#include <Tempest/ScroolWidget>
#include <Tempest/Panel>

using namespace Tempest;

AbstractListBox::AbstractListBox() : Button() {
  clicked.bind( *this, &AbstractListBox::showList );
  overlay    = 0;
  needToShow = 1;

  lastRM = Tempest::Application::tickCount();
  }

AbstractListBox::~AbstractListBox() {
  close();
  }

void AbstractListBox::showList() {
  if( !needToShow )
    return;

  if( overlay )
    close();

  Widget *box = createDropList();

  overlay = new Overlay();
  T_ASSERT( SystemAPI::instance().addOverlay( overlay )!=0 );

  overlay->owner = this;

  if( box->w() > overlay->w() )
    box->resize( overlay->w(), box->h() );

  if( box->h() > overlay->h() )
    box->resize( box->w(), overlay->h() );

  if( box->y()+box->h() > overlay->h() )
    box->setPosition( box->x(), overlay->h()-box->h() );
  if( box->x()+box->w() > overlay->w() )
    box->setPosition( overlay->w()-box->w(), box->y() );
  if( box->x()<0 )
    box->setPosition(0, box->y());
  if( box->y()<0 )
    box->setPosition(box->x(), 0);

  overlay->layout().add( box );

  box->setFocus(1);
  }

void AbstractListBox::close() {
  if( overlay ){
    overlay->deleteLater();
    overlay = 0;
    }
  }

void AbstractListBox::mouseDownEvent(Tempest::MouseEvent &e) {
  needToShow = Tempest::Application::tickCount()!=lastRM;
  Button::mouseDownEvent(e);
  }

void AbstractListBox::mouseUpEvent(Tempest::MouseEvent &e) {
  Button::mouseUpEvent(e);
  needToShow = true;
  }

AbstractListBox::Overlay::~Overlay() {
  owner->overlay = 0;
  }

void AbstractListBox::Overlay::mouseDownEvent(MouseEvent &e) {
  deleteLater();
  e.ignore();
  owner->lastRM = Tempest::Application::tickCount();
  }
