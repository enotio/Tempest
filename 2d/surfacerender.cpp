#include "surfacerender.h"

#include <Tempest/Surface>
#include <Tempest/Sprite>
#include <Tempest/Font>

using namespace Tempest;

SurfaceRender::SurfaceRender( VertexShaderHolder &vs,
                              FragmentShaderHolder &fs )
  :vsH(vs), fsH(fs){
  cpuGm.reserve(8096);

  invW = 1;
  invH = 1;

  invTw = 1;
  invTh = 1;

  dpos = 0;

  //curTex   = 0;
  //curTex2d = 0;

  sstk.reserve(32);
  blocks.reserve(1024);

  rstate[1].setBlend(1);
  rstate[1].setBlendMode( RenderState::AlphaBlendMode::src_alpha,
                          RenderState::AlphaBlendMode::one_minus_src_alpha );
  rstate[2].setBlend(1);
  rstate[2].setBlendMode( RenderState::AlphaBlendMode::one,
                          RenderState::AlphaBlendMode::one );
  rstate[3].setBlend(1);
  rstate[3].setBlendMode( RenderState::AlphaBlendMode::dst_color,
                          RenderState::AlphaBlendMode::src_color );

  for( int i=0; i<4; ++i ){
    rstate[i].setZWriting(0);
    rstate[i].setZTest(0);
    }

  loadShader();
  }

void SurfaceRender::loadShader() {
  {
  AbstractShadingLang::UiShaderOpt opt;
  vs[0] = vsH.surfaceShader(opt, dpos);
  fs[0] = fsH.surfaceShader(opt);
  }

  {
  AbstractShadingLang::UiShaderOpt opt;
  opt.hasTexture = false;

  vs[1] = vsH.surfaceShader(opt);
  fs[1] = fsH.surfaceShader(opt);
  }
  }

void SurfaceRender::buildVbo( Tempest::Widget & surf,
                              VertexBufferHolder & vbHolder,
                              IndexBufferHolder  & ibHolder , SpritesHolder &sp) {
  buildVbo( surf, std::mem_fn(&Tempest::Widget::paintEvent),
            surf.w(), surf.h(), vbHolder, ibHolder, sp );
  }

void SurfaceRender::renderTo(Device &dev) {
  if( dpos ){
    float dp[2] = { -0.5f*invW, -0.5f*invH };
    dev.setUniform( vs[0], dp, 2, "dpos" );
    dev.setUniform( vs[1], dp, 2, "dpos" );
    }

  int sh = 0;
  for( size_t i=0; i<blocks.size(); ++i ){
    Block& b = blocks[i];
    dev.setRenderState( rstate[b.state.bm] );
    if(!b.curTex.pageRect().isEmpty() || b.curTex2d)
      sh = 0; else
      sh = 1;

    if( !b.curTex.pageRect().isEmpty() )
      dev.setUniform( fs[0], b.curTex.pageRawData(),   "texture" );
    if( b.curTex2d )
      dev.setUniform( fs[0], *b.curTex2d, "texture" );

    int sz = b.isLine? 2:3;
    dev.drawPrimitive( b.isLine? AbstractAPI::Lines : AbstractAPI::Triangle,
                       vs[sh], fs[sh],
                       vdecl,
                       vbo,
                       b.begin,
                       (b.end-b.begin)/sz );
    }
  }


void SurfaceRender::quad( int  x, int  y, int  w, int  h,
                          int tx, int ty, int tw, int th ) {
  Vertex v[4];
  v[0].x = x*invW;
  v[0].y = y*invH;
  v[0].u = tx*invTw;
  v[0].v = (ty+0)*invTh;

  v[1].x = (x+w)*invW;
  v[1].y = y*invH;
  v[1].u = (tx+tw)*invTw;
  v[1].v = (ty+0)*invTh;

  v[2].x = (x+w)*invW;
  v[2].y = (y+h)*invH;
  v[2].u = (tx+tw)*invTw;
  v[2].v = (ty+th)*invTh;

  v[3].x = x*invW;
  v[3].y = (y+h)*invH;
  v[3].u = tx*invTw;
  v[3].v = (ty+th)*invTh;

  for( int i=0; i<4; ++i ){
    v[i].color[0] = state.cl.r();
    v[i].color[1] = state.cl.g();
    v[i].color[2] = state.cl.b();
    v[i].color[3] = state.cl.a();

    v[i].x -= 1;
    v[i].y += 1;// - v[i].y;
    }

  if( state.flip[0] )
    for( int i=0; i<4; ++i ){
      v[i].u = 1.0f-v[i].u;
      }
  if( state.flip[1] )
    for( int i=0; i<4; ++i ){
      v[i].v = 1.0f-v[i].v;
      }

  if( !curTex.size().isEmpty() ){
    const Tempest::Size s = curTex.pageRawData().size();
    const Tempest::Rect r = curTex.pageRect();

    float dx = r.x/float(s.w),
          dy = r.y/float(s.h),
          dw = r.w/float(s.w),
          dh = r.h/float(s.h);

    for( int i=0; i<4; ++i ){
      v[i].u *= dw;
      v[i].v *= dh;

      v[i].u += dx;
      v[i].v += dy;
      }
    }

  //for( int i=0; i<4; ++i )
    //v[i].v = 1.0-v[i].v;

  cpuGm.push_back( v[0] );
  cpuGm.push_back( v[1] );
  cpuGm.push_back( v[2] );

  cpuGm.push_back( v[0] );
  cpuGm.push_back( v[2] );
  cpuGm.push_back( v[3] );
  updateBackBlock(false);
  }

void SurfaceRender::line(int x, int y, int x2, int y2) {
  Vertex v[2];
  v[0].x = x*invW;
  v[0].y = y*invH;

  v[1].x = x2*invW;
  v[1].y = y2*invH;

  for( int i=0; i<2; ++i ){
    v[i].color[0] = state.cl.r();
    v[i].color[1] = state.cl.g();
    v[i].color[2] = state.cl.b();
    v[i].color[3] = state.cl.a();

    v[i].u = 0;
    v[i].v = 0;

    v[i].x -= 1;
    v[i].y += 1;
    }

  if( state.flip[0] )
    for( int i=0; i<2; ++i ){
      v[i].u = 1.0f-v[i].u;
      }
  if( state.flip[1] )
    for( int i=0; i<2; ++i ){
      v[i].v = 1.0f-v[i].v;
      }

  cpuGm.push_back( v[0] );
  cpuGm.push_back( v[1] );

  updateBackBlock(true);
  }


void SurfaceRender::updateBackBlock( bool isLine ) {
  if( blocks.size()==0 ){
    blocks.resize(1);

    Block& b = blocks.back();
    b.begin    = 0;
    b.end      = 0;
    b.curTex    = curTex;
    b.curTex2d  = curTex2d;
    b.isLine    = isLine;
    b.state     = state;
    }

  while( blocks.size()>0 && blocks.back().end==blocks.back().begin )
    blocks.pop_back();

  if( blocks.back().state.bm != state.bm ||
      blocks.back().state.cl != state.cl ||
      blocks.back().curTex2d != curTex2d ||
      blocks.back().curTex.  handle() != curTex.  handle() ||
      blocks.back().isLine   != isLine ){
    size_t e = 0;
    if( blocks.size() )
      e = blocks.back().end;

    blocks.push_back( Block() );

    Block& b  = blocks.back();
    b.end       = e;
    b.begin     = b.end;
    b.curTex    = curTex;
    b.curTex2d  = curTex2d;
    b.isLine    = isLine;
    b.state     = state;
    }

  Block& b  = blocks.back();
  b.end     = cpuGm.size();
  }

const Tempest::VertexDeclaration::Declarator &SurfaceRender::decl() {
  static Tempest::VertexDeclaration::Declarator vdecl = declImpl();
  return vdecl;
  }

const VertexDeclaration::Declarator SurfaceRender::declImpl() {
  Tempest::VertexDeclaration::Declarator decl;
  decl.add( Tempest::Decl::float2, Tempest::Usage::Position )
      .add( Tempest::Decl::float2, Tempest::Usage::TexCoord, 0 )
      .add( Tempest::Decl::float4, Tempest::Usage::TexCoord, 1 );

  return decl;
  }

void SurfaceRender::PaintDev::quad( int x, int y, int w, int h,
                              int tx, int ty, int tw, int th ) {
  surf.quad(x,y,w,h, tx,ty,tw,th);
  }

void SurfaceRender::PaintDev::line(int x, int y, int x2, int y2) {
  surf.line(x,y,x2,y2);
  }

void SurfaceRender::PaintDev::setBlendMode(BlendMode m) {
  surf.state.bm = m;
  }

PaintTextEngine &SurfaceRender::PaintDev::textEngine() {
  return te;
  }

void SurfaceRender::PaintDev::setColor( float r, float g, float b, float a ) {
  surf.state.cl.set(r,g,b,a);
  }

void SurfaceRender::PaintDev::setFlip(bool h, bool v) {
  surf.state.flip[0] = h;
  surf.state.flip[1] = v;
  }

void SurfaceRender::PaintDev::setNullState() {
  surf.state.bm      = Tempest::noBlend;
  surf.state.cl      = Tempest::Color(1);
  surf.state.flip[0] = false;
  surf.state.flip[1] = false;
  }

void SurfaceRender::PaintDev::pushState() {
  surf.sstk.push_back( surf.state );
  }

void SurfaceRender::PaintDev::popState() {
  surf.state = sstk.back();
  surf.sstk.pop_back();
  }

SurfaceRender::PaintDev::PaintDev(SurfaceRender &s,
                                   std::vector<SurfaceRender::RState> &sstk,
                                  SpritesHolder &sp,
                                   int rx, int ry, int rw, int rh )
  :surf(s), sstk(sstk), te(*this), sp(sp) {
  surf.curTex   = Tempest::Sprite();
  surf.curTex2d = 0;

  setScissor(rx,ry,rw,rh);
  }

void SurfaceRender::PaintDev::setTexture(const Texture2d &t) {
  surf.invTw = 1.0f/t.width();
  surf.invTh = 1.0f/t.height();

  unsetTexture();
  surf.curTex2d = &t;
  }

void SurfaceRender::PaintDev::setTexture(const Sprite &t) {
  surf.invTw = 1.0f/t.width();
  surf.invTh = 1.0f/t.height();

  unsetTexture();
  surf.curTex = t;
  }

void SurfaceRender::PaintDev::unsetTexture() {
  surf.curTex   = Tempest::Sprite();
  surf.curTex2d = 0;
  }


SurfaceRender::TextEngine::TextEngine(PaintDev &p)
                       :p(p) {
  }

void SurfaceRender::TextEngine::setFont(const Tempest::Font &f) {
  font = f;
  }

template< class T >
void SurfaceRender::TextEngine::dText( int x, int y, int w, int h,
                                       const T &str,
                                       int flg ) {
  font.fetch( str, p.sp );

  Tempest::Rect oldScissor = p.scissor();
  p.setScissor( oldScissor.intersected( Tempest::Rect(x,y,w,h) ) );
  p.setBlendMode( Tempest::alphaBlend );

  int tx = 0, ty = 0, tw = 0, th = 0;
  for( size_t i=0; i<str.size(); ++i ){
    const Font::Letter& l = font.letter( str[i], p.sp );

    tw = std::max( tx+l.dpos.x+l.size.w, tw );
    th = std::max( ty+l.dpos.y+l.size.h, th );

    tx+= l.advance.x;
    ty+= l.advance.y;
    }


  if( flg & Tempest::AlignHCenter )
    x += (w-tw)/2; else

  if( flg & Tempest::AlignRight )
    x += (w-tw);

  if( flg & Tempest::AlignVCenter )
    y += (h-th)/2; else
  if( flg & Tempest::AlignBottom )
    y += (h-th);


  for( size_t i=0; i<str.size(); ++i ){
    const Font::Letter& l = font.letter( str[i], p.sp );
    p.setTexture( l.surf );
    p.drawRect( x+l.dpos.x, y+l.dpos.y, l.size.w, l.size.h,
                0,0 );
    x+= l.advance.x;
    y+= l.advance.y;
    }

  p.setScissor(oldScissor);
  }

void SurfaceRender::TextEngine::drawText( int x, int y, int w, int h,
                                          const std::wstring &s, int align) {
  dText(x, y, w, h, s, align);
  }

void SurfaceRender::TextEngine::drawText( int x, int y, int w, int h,
                                          const std::string &s, int align) {
  dText(x, y, w, h, s, align);
  }

const Font::Letter &SurfaceRender::TextEngine::letter(const Font &f, wchar_t c) {
  return f.letter(c, p.sp);
  }
