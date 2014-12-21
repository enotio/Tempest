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
  if( !dragable ){
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

void Panel::mouseMoveEvent(Tempest::MouseEvent &) {
  if( !dragable ){
    //e.ignore();
    }
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
  p.setBlendMode(alphaBlend);
  drawBack(p);
  drawFrame(p);
  paintNested(e);
  }

void Panel::drawFrame( Tempest::Painter &p ){
  auto vRect = Rect(0,0,w()-1,h()-1);
  auto c     = p.color();
  p.setColor(Color(0.25,0.25,0.25,1));

  p.drawLine(vRect.x,vRect.y,        vRect.x+vRect.w,vRect.y);
  p.drawLine(vRect.x,vRect.y+vRect.h,vRect.x+vRect.w,vRect.y+vRect.h);

  p.drawLine(vRect.x,        vRect.y,vRect.x,        vRect.y+vRect.h);
  p.drawLine(vRect.x+vRect.w,vRect.y,vRect.x+vRect.w,vRect.y+vRect.h);
  p.setColor(c);
  }

void Panel::drawBack(Painter &p) {
  auto c = p.color();
  p.setColor(Color(0.8f,0.8f,0.85f,0.75f));
  p.drawRect(0,0,w()-1,h()-1);
  p.setColor(c);
  }
