#include "painter.h"

#include <cstdlib>
#include <algorithm>
#include <Tempest/Event>

#include <Tempest/SizePolicy>

using namespace Tempest;

PainterDevice::PainterDevice() {
  State::ScissorRect & m_scissor = rstate.scissor;

  m_scissor.x  = 0 + rstate.orign.x;
  m_scissor.y  = 0 + rstate.orign.y;
  m_scissor.x1 = m_scissor.x-1;
  m_scissor.y1 = m_scissor.y-1;
  }

PainterDevice::~PainterDevice() {
  }

void PainterDevice::setScissor(int x, int y, int w, int h) {
  State::ScissorRect & m_scissor = rstate.scissor;

  m_scissor.x  = x + rstate.orign.x;
  m_scissor.y  = y + rstate.orign.y;
  m_scissor.x1 = m_scissor.x+w;
  m_scissor.y1 = m_scissor.y+h;

  textEngine().setScissor( x + rstate.orign.x,
                           y + rstate.orign.y,
                           w, h );
  }

void PainterDevice::setScissor( const Rect &r ) {
  setScissor( r.x, r.y, r.w, r.h );
  }

Rect PainterDevice::scissor() const {
  const State::ScissorRect & m_scissor = rstate.scissor;
  const Point & orign = rstate.orign;

  return Rect( m_scissor.x  - orign.x,
               m_scissor.y  - orign.y,
               m_scissor.x1 - m_scissor.x,
               m_scissor.y1 - m_scissor.y );
  }

void PainterDevice::drawRect(int x, int y, int w, int h, int tx, int ty) {
  drawRect( x,y,w,h, tx,ty,w,h );
  }

void PainterDevice::drawRect( int x, int y, int w, int h,
                              int tx, int ty, int tw, int th ) {
  State::ScissorRect & m_scissor = rstate.scissor;
  Point & orign = rstate.orign;

  x += orign.x;
  y += orign.y;

  if( m_scissor.x1 >= m_scissor.x &&
      m_scissor.y1 >= m_scissor.y ){
    if( x<m_scissor.x ){
      if( w )
        tx += (tw*(m_scissor.x - x))/w;
      if( w )
        tw -= (tw*(m_scissor.x - x))/w;

      w  -= (m_scissor.x - x);
      x = m_scissor.x;
      }

    if( y<m_scissor.y ){
      if( h )
        ty += (th*(m_scissor.y - y))/h;
      if( h )
        th -= (th*(m_scissor.y - y))/h;
      //th -= (m_scissor.y - y);
      //ty += (m_scissor.y - y);

      h  -= (m_scissor.y - y);
      y = m_scissor.y;
      }

    if( x+w > m_scissor.x1 ){
      if( w )
        tw += (tw*(m_scissor.x1 - x-w ))/w;
      w = m_scissor.x1-x;
      }

    if( y+h > m_scissor.y1 ){
      if( h )
        th += (th*(m_scissor.y1 - y-h ))/h;
      h = m_scissor.y1-y;
      }
    }

  if( w>0 && h>0 )
    quad(x,y,w,h, tx, ty, tw, th);
  }

void PainterDevice::drawRectTailed(int x, int y, int w, int h, int tx, int ty) {
  drawRectTailed( x,y,w,h, tx,ty,w,h );
  }


void Painter::drawRect(const Rect &r, const Rect &t) {
  drawRect( r.x, r.y, r.w, r.h,
            t.x, t.y, t.w, t.h );
  }

void Painter::drawRect(const Rect &r) {
  drawRect( r.x, r.y, r.w, r.h, 0, 0 );
  }

void Painter::drawRect( int x, int y, int w, int h ) {
  drawRect(x,y,w,h,0,0);
  }

void Painter::drawRect(const Rect &r, int tx, int ty) {
  dev.drawRect( r.x, r.y, r.w, r.h, tx, ty );
  }

void Painter::drawRect( int x, int y, int w, int h,
                        int tx, int ty, int tw, int th ) {
  dev.drawRect(x,y,w,h,tx,ty,tw,th);
  }

void Painter::drawRect( int x, int y, int w, int h,
                        int tx, int ty ) {
  drawRect( x, y, w, h, tx, ty, w, h );
  }

void Painter::drawRectTailed( int x, int y, int w, int h,
                              int tx, int ty, int tw, int th) {
  dev.drawRectTailed( x,y,w,h, tx,ty,tw,th );
  }

void Painter::drawLine(int x, int y, int x1, int y1) {
  dev.drawLine(x,y,x1,y1);
  }

void PainterDevice::drawRectTailed( int x, int y, int w, int h,
                                    int tx, int ty, int tw, int th) {
  if( w<=0 || h<=0)
    return;

  tw = std::max(1, tw);
  th = std::max(1, th);

  tw = std::min( tw, w );
  th = std::min( th, h );

  int wl = w%tw, hl = h%th;

  for( int i=0; i+wl<w; i+=tw )
    for( int r=0; r+hl<h; r+=th )
      drawRect( x+i, y+r, tw, th, tx, ty );

  if( hl>0 )
    for( int i=0; i+wl<w; i+=tw )
      drawRect( x+i, y+h-hl-1, tw, hl, tx, ty );

  if( wl>0 )
    for( int r=0; r+hl<h; r+=th )
      drawRect( x+w-wl-1, y+r, wl, th, tx, ty );

  if( wl>0 && hl>0 )
    drawRect( x+w-wl-1, y+h-hl-1, wl, hl, tx, ty );
  }

void PainterDevice::drawLine( int x, int y, int x1, int y1 ) {
  State::ScissorRect & m_scissor = rstate.scissor;
  Point & orign = rstate.orign;

  x  += orign.x;
  x1 += orign.x;

  y  += orign.y;
  y1 += orign.y;

  if( x>x1 ){
    std::swap(x, x1);
    std::swap(y, y1);
    }

  if( m_scissor.x <= m_scissor.x1 &&
      m_scissor.y <= m_scissor.y1 ){
    if( x  > m_scissor.x1 &&
        x1 > m_scissor.x1 )
      return;

    if( x  < m_scissor.x &&
        x1 < m_scissor.x )
      return;

    if( y  > m_scissor.y1 &&
        y1 > m_scissor.y1 )
      return;

    if( y  < m_scissor.y &&
        y1 < m_scissor.y )
      return;

    if( x<m_scissor.x ){
      if( x!=x1 )
        y += (m_scissor.x-x)*(y1-y)/(x1-x); else
        return;

      x = m_scissor.x;
      }

    if( x1 > m_scissor.x1 ){
      if( x!=x1 )
        y1 += (m_scissor.x1-x1)*(y1-y)/(x1-x); else
        return;

      x1 = m_scissor.x1;
      }

    if( y<m_scissor.y ){
      if( y!=y1 )
        x += (m_scissor.y-y)*(x1-x)/(y1-y); else
        return;

      y = m_scissor.y;
      }

    if( y1<m_scissor.y ){
      if( y!=y1 )
        x1 += (m_scissor.y-y1)*(x1-x)/(y1-y); else
        return;

      y1 = m_scissor.y;
      }

    if( y > m_scissor.y1 ){
      if( y!=y1 )
        x += (m_scissor.y1-y)*(x1-x)/(y1-y); else
        return;

      y = m_scissor.y1;
      }

    if( y1 > m_scissor.y1 ){
      if( y!=y1 )
        x1 += (m_scissor.y1-y1)*(x1-x)/(y1-y); else
        return;

      y1 = m_scissor.y1;
      }
    }

  if( !(x==x1 && y==y1) )
    line(x,y,x1,y1);
  }

void PainterDevice::translate(int dx, int dy) {
  rstate.orign += Point(dx,dy);
  }

void PainterDevice::setBlendMode(BlendMode ) {

  }

void PainterDevice::setState(const PainterDevice::State &s) {
  rstate = s;
  }


Painter::Painter(PaintEvent &e) : dev( e.painter ){
  oldState = dev.rstate;
  dev.pushState();
  dev.setNullState();
  }

Painter::Painter( PainterDevice &d ) :dev(d) {
  oldState = dev.rstate;
  dev.pushState();
  dev.setNullState();
  }

Painter::~Painter() {
  dev.popState();
  dev.setState( oldState );
  }

void Painter::setScissor(int x, int y, int w, int h) {
  dev.setScissor(x,y,w,h);
  }

void Painter::setScissor(const Rect &r) {
  dev.setScissor( r.x, r.y, r.w, r.h );
  }

Rect Painter::scissor() const {
  return dev.scissor();
  }

void Painter::setTexture( const Texture2d &t ) {
  dev.setTexture(t);
  }

void Painter::setTexture( const Tempest::Sprite &t ) {
  dev.setTexture(t);
  }

void Painter::unsetTexture() {
  dev.unsetTexture();
  }

void Painter::setColor(float r, float g, float b, float a) {
  dev.setColor(r,g,b,a);
  }

void Painter::setFlip(bool h, bool v) {
  dev.setFlip(h,v);
  }

void Painter::setBlendMode( BlendMode m ) {
  dev.setBlendMode(m);
  }

void Painter::setFont(const std::string &f, int sz) {
  dev.textEngine().setFont( f, sz );
  }

void Painter::setFont(const Font &f) {
  dev.textEngine().setFont(f);
  }

const Font::Letter &Painter::letter(const Font &f, wchar_t c) {
  return dev.textEngine().letter(f,c);
  }

void Painter::drawText( int x, int y, int w, int h, const std::string &str,
                        int flag ) {
  dev.textEngine().drawText( x,
                             y,
                             w, h, str, flag);
  }

void Painter::drawText(int x, int y, const std::string &str, int flg ) {
  drawText( x, y,
            SizePolicy::maxWidgetSize().w,
            SizePolicy::maxWidgetSize().h, str, flg );
  }

void Painter::drawText( int x, int y, int w, int h, const std::wstring &str,
                        int flg ) {
  dev.textEngine().drawText( x,//dev.rstate.orign.x+x,
                             y,//dev.rstate.orign.y+y,
                             w, h, str, flg);
  }

void Painter::drawText(int x, int y, const std::wstring &str, int flg ) {
  drawText( x, y,
            SizePolicy::maxWidgetSize().w,
            SizePolicy::maxWidgetSize().h, str,
            flg );
  }

void Painter::drawLine(const Point &p, const Point &p1) {
  drawLine( p.x, p.y, p1.x, p1.y );
  }

void Painter::translate( int px, int py ) {
  dev.translate(px, py);
  }

void Painter::translate(const Point &p) {
  dev.translate(p.x, p.y);
  }

Tempest::PainterDevice& Tempest::Painter::device() {
  return dev;
  }
