#include "panel.h"

using namespace Tempest;

Panel::Panel() {
  resize(200, 400);
  mouseTracking = false;
  dragable      = false;

  setMargin(8);
  setLayout( Tempest::Horizontal );
  }

void Panel::setDragable(bool d) {
  dragable = d;
  }

bool Panel::isDragable() {
  return dragable;
  }

void Panel::mouseDownEvent(Tempest::MouseEvent &e) {
  if( !dragable || !isEnabled() ){
    return;
    }

  if( e.button==Tempest::MouseEvent::ButtonLeft ){
    mouseTracking = true;
    oldPos = pos();
    mpos   = mapToRoot( e.pos() );
    }
  }

void Panel::mouseDragEvent(Tempest::MouseEvent &e) {
  if( !dragable ){
    e.ignore();
    return;
    }

  if( mouseTracking )
    setPosition( oldPos - (mpos - mapToRoot(e.pos() )) );
  }

void Panel::mouseMoveEvent(Tempest::MouseEvent &e) {
  e.accept();
  }

void Panel::mouseUpEvent(Tempest::MouseEvent &) {
  mouseTracking = false;
  }

void Panel::mouseWheelEvent(Tempest::MouseEvent &) {
  }

void Panel::gestureEvent(Tempest::AbstractGestureEvent &e) {
  if( !Tempest::Rect(0,0,w(),h()).contains(e.hotSpot()) )
    e.ignore();
  }

void Panel::paintEvent( Tempest::PaintEvent &e ) {
  Tempest::Painter p(e);
  style().draw(p,this,Style::E_Background,state(),Rect(0,0,w(),h()),Style::Extra(*this));

  paintNested(e);
  }
