#include "painter.h"

#include <cstdlib>
#include <algorithm>
#include <Tempest/Event>

#include <Tempest/SizePolicy>

using namespace Tempest;

PainterDevice::PainterDevice() {
  State::ScissorRect & scissor = rstate.scissor;

  scissor.x  = 0 + rstate.orign.x;
  scissor.y  = 0 + rstate.orign.y;
  scissor.x1 = scissor.x-1;
  scissor.y1 = scissor.y-1;
  }

PainterDevice::~PainterDevice() {
  }

void PainterDevice::setScissor( const Rect &r ) {
  setScissor( r.x, r.y, r.w, r.h );
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
      drawRect( x+i, y+h-hl, tw, hl, tx, ty );

  if( wl>0 )
    for( int r=0; r+hl<h; r+=th )
      drawRect( x+w-wl, y+r, wl, th, tx, ty );

  if( wl>0 && hl>0 )
    drawRect( x+w-wl, y+h-hl, wl, hl, tx, ty );
  }

void PainterDevice::drawLine( int x, int y, int x1, int y1 ) {
  if( cropLine(x,y,x1,y1,x,y,x1,y1) )
    line(x,y,x1,y1);
  }

void PainterDevice::drawTriangle( int x0, int y0, int u0, int v0,
                                  int x1, int y1, int u1, int v1,
                                  int x2, int y2, int u2, int v2 ) {
  drawTrigImpl( x0, y0, u0, v0,
                x1, y1, u1, v1,
                x2, y2, u2, v2,
                trigBuf, 0 );
  }

void PainterDevice::drawTrigImpl( float x0, float y0, float u0, float v0,
                                  float x1, float y1, float u1, float v1,
                                  float x2, float y2, float u2, float v2,
                                  FPoint* out,
                                  int stage ) {
  State::ScissorRect& s = rstate.scissor;
  FPoint*             r = out;

  float x[4] = {x0,x1,x2,x0};
  float y[4] = {y0,y1,y2,y0};
  float u[4] = {u0,u1,u2,u0};
  float v[4] = {v0,v1,v2,v0};

  float mid;
  float sx;
  bool  cs, ns;

  switch( stage ) {
    case 0: {
      Point& orign = rstate.orign;
      sx = s.x;
      for(int i=0;i<4;++i){
        x[i] += orign.x;
        y[i] += orign.y;
        }
      }
      break;
    case 1: sx = s.y;  break;
    case 2: sx = s.x1; break;
    case 3: sx = s.y1; break;
    }

  for(int i=0;i<3;++i){
    switch( stage ){
      case 0:
        cs = x[i]  >=sx;
        ns = x[i+1]>=sx;
        break;
      case 1:
        cs = y[i]  >=sx;
        ns = y[i+1]>=sx;
        break;
      case 2:
        cs = x[i]  <sx;
        ns = x[i+1]<sx;
        break;
      case 3:
        cs = y[i]  <sx;
        ns = y[i+1]<sx;
        break;
      }

    if(cs==ns){
      if( cs ){
        *out = FPoint{float(x[i+1]),float(y[i+1]),float(u[i+1]),float(v[i+1])}; ++out;
        }
      } else {
      float dx = (x[i+1]-x[i]);
      float dy = (y[i+1]-y[i]);
      float du = (u[i+1]-u[i]);
      float dv = (v[i+1]-v[i]);
      if( ns ){
        if(stage%2==0){
          mid = y[i] + (dy*(sx-x[i]))/dx;
          *out = FPoint{sx, mid,
                        u[i] + (du*(sx-x[i]))/dx,
                        v[i] + (dv*(sx-x[i]))/dx};   ++out;

          *out = FPoint{float(x[i+1]),float(y[i+1]),float(u[i+1]),float(v[i+1])}; ++out;
          } else {
          mid = x[i] + (dx*(sx-y[i]))/dy;
          *out = FPoint{mid, sx,
                        u[i] + (du*(sx-y[i]))/dy,
                        v[i] + (dv*(sx-y[i]))/dy};   ++out;

          *out = FPoint{float(x[i+1]),float(y[i+1]),float(u[i+1]),float(v[i+1])}; ++out;
          }
        } else {
        if(stage%2==0){
          mid = y[i] + (dy*(sx-x[i]))/dx;
          *out = FPoint{sx, mid,
                        u[i] + (du*(sx-x[i]))/dx,
                        v[i] + (dv*(sx-x[i]))/dx};   ++out;
          } else {
          mid = x[i] + (dx*(sx-y[i]))/dy;
          *out = FPoint{mid, sx,
                        u[i] + (du*(sx-y[i]))/dy,
                        v[i] + (dv*(sx-y[i]))/dy};   ++out;
          }
        }
      }
    cs = ns;
    }

  int count = out-r;
  count-=3;

  if( stage<3 ){
    for(int i=0;i<=count;++i){
      drawTrigImpl( r[  0].x, r[  0].y, r[  0].u, r[  0].v,
                    r[i+1].x, r[i+1].y, r[i+1].u, r[i+1].v,
                    r[i+2].x, r[i+2].y, r[i+2].u, r[i+2].v,
                    out, stage+1 );
      }
    } else {
    for(int i=0;i<=count;++i){
      triangle( r[  0].x, r[  0].y, r[  0].u, r[  0].v,
                r[i+1].x, r[i+1].y, r[i+1].u, r[i+1].v,
                r[i+2].x, r[i+2].y, r[i+2].u, r[i+2].v );
      }
    }
  }

void PainterDevice::translate(int dx, int dy) {
  rstate.orign += Point(dx,dy);
  }

void PainterDevice::setBlendMode(BlendMode ) {  
  }

BlendMode PainterDevice::blendMode() const {
  return noBlend;
  }

void PainterDevice::setState(const PainterDevice::State &s) {
  rstate = s;
  }

bool PainterDevice::cropLine(int x, int y, int x1, int y1,
                             int &nx, int &ny, int &nx1, int &ny1) {

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
      return false;

    if( x  < m_scissor.x &&
        x1 < m_scissor.x )
      return false;

    if( y  > m_scissor.y1 &&
        y1 > m_scissor.y1 )
      return false;

    if( y  < m_scissor.y &&
        y1 < m_scissor.y )
      return false;

    if( x<m_scissor.x ){
      if( x!=x1 )
        y += (m_scissor.x-x)*(y1-y)/(x1-x); else
        return false;

      x = m_scissor.x;
      }

    if( x1 > m_scissor.x1 ){
      if( x!=x1 )
        y1 += (m_scissor.x1-x1)*(y1-y)/(x1-x); else
        return false;

      x1 = m_scissor.x1;
      }

    if( y<m_scissor.y ){
      if( y!=y1 )
        x += (m_scissor.y-y)*(x1-x)/(y1-y); else
        return false;

      y = m_scissor.y;
      }

    if( y1<m_scissor.y ){
      if( y!=y1 )
        x1 += (m_scissor.y-y1)*(x1-x)/(y1-y); else
        return false;

      y1 = m_scissor.y;
      }

    if( y > m_scissor.y1 ){
      if( y!=y1 )
        x += (m_scissor.y1-y)*(x1-x)/(y1-y); else
        return false;

      y = m_scissor.y1;
      }

    if( y1 > m_scissor.y1 ){
      if( y!=y1 )
        x1 += (m_scissor.y1-y1)*(x1-x)/(y1-y); else
        return false;

      y1 = m_scissor.y1;
      }
    }

  nx  = x;
  ny  = y;
  nx1 = x1;
  ny1 = y1;
  return !(x==x1 && y==y1);
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

void Painter::setColor( const Color &c ) {
  dev.setColor( c.r(), c.g(), c.b(), c.a() );
  }

void Painter::setColor(float r, float g, float b, float a) {
  dev.setColor(r,g,b,a);
  }

Color Painter::color() const {
  return dev.color();
  }

void Painter::setFlip(bool h, bool v) {
  dev.setFlip(h,v);
  }

bool Painter::isHorizontalFliped() const {
  return dev.isHorizontalFliped();
  }

bool Painter::isVerticalFliped() const {
  return dev.isVerticalFliped();
  }

void Painter::setBlendMode( BlendMode m ) {
  dev.setBlendMode(m);
  }

BlendMode Painter::blendMode() const {
  return dev.blendMode();
  }

void Painter::setFont(const std::string &f, int sz) {
  dev.textEngine().setFont( f, sz );
  }

void Painter::setFont(const Font &f) {
  dev.textEngine().setFont(f);
  }

Font Painter::font() const {
  return dev.textEngine().font();
  }

const Font::Letter &Painter::letter(const Font &f, wchar_t c) {
  return dev.textEngine().letter(f,c);
  }

void Painter::drawText( int x, int y, int w, int h,
                        const char *str,
                        int flag) {
  dev.textEngine().drawText( x,
                             y,
                             w, h,
                             str,
                             flag);
  }

void Painter::drawText(int x, int y, const char16_t *t, int flg) {
  dev.textEngine().drawText( x, y,
                             SizePolicy::maxWidgetSize().w,
                             SizePolicy::maxWidgetSize().h,
                             t,
                             flg );
  }

void Painter::drawText(int x, int y, int w, int h, const char16_t *str, int flg) {
  dev.textEngine().drawText( x, y, w,  h,
                             str,
                             flg );
  }

void Painter::drawText( int x, int y, int w, int h,
                        const std::string &str,
                        int flag ) {
  dev.textEngine().drawText( x,
                             y,
                             w, h,
                             str.c_str(),
                             flag);
  }

void Painter::drawText(int x, int y, const std::string &str, int flg ) {
  dev.textEngine().drawText( x, y,
                             SizePolicy::maxWidgetSize().w,
                             SizePolicy::maxWidgetSize().h,
                             str.c_str(),
                             flg );
  }

void Painter::drawText(int x, int y, const char *str, int flg) {
  dev.textEngine().drawText( x, y,
                             SizePolicy::maxWidgetSize().w,
                             SizePolicy::maxWidgetSize().h,
                             str,
                             flg );
  }

void Painter::drawText( int x, int y,
                        int w, int h,
                        const std::u16string &str,
                        int flg ) {
  dev.textEngine().drawText( x, y, w, h,
                             str.c_str(),
                             flg);
  }

void Painter::drawText(int x, int y, const std::u16string &str, int flg ) {
  drawText( x, y,
            SizePolicy::maxWidgetSize().w,
            SizePolicy::maxWidgetSize().h, str,
            flg );
  }

void Painter::drawLine(const Point &p, const Point &p1) {
  drawLine( p.x, p.y, p1.x, p1.y );
  }

void Painter::drawTriangle( int x0, int y0, int u0, int v0,
                            int x1, int y1, int u1, int v1,
                            int x2, int y2, int u2, int v2 ) {
  dev.drawTriangle(x0,y0,u0,v0,x1,y1,u1,v1,x2,y2,u2,v2);
  }

void Painter::drawTriangle(int x0, int y0, int x1, int y1, int x2, int y2) {
  dev.drawTriangle(x0,y0,0,0,x1,y1,x1-x0,y1-y0,x2,y2,x2-x0,y2-y0);
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
