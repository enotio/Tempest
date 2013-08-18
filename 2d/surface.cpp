#include "surface.h"

#include <Tempest/RenderState>
#include <Tempest/Sprite>

using namespace Tempest;

Surface::Surface( Tempest::VertexShaderHolder   & vs,
                  Tempest::FragmentShaderHolder & fs )
  :vsH(vs), fsH(fs) {
  cpuGm.reserve(8096);

  invW = 1;
  invH = 1;

  invTw = 1;
  invTh = 1;

  curTex   = 0;
  curTex2d = 0;

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

void Surface::loadShader() {
  const char* vs_src =
      "attribute vec2 Position;"
      "attribute vec2 TexCoord;"
      "attribute vec4 TexCoord1;"

      "varying vec2 tc;"
      "varying vec4 cl;"

      "void main() {"
        "tc = TexCoord;"
        "cl = TexCoord1;"
        "gl_Position = vec4(Position, 0.0, 1.0);"
        "}";

  const char* fs_src =
      "varying vec2 tc;"
      "varying vec4 cl;"
      "uniform sampler2D texture;"

      "void main() {"
        "gl_FragColor = texture2D(texture, tc)*cl;"
        "}";

  vs = vsH.loadFromSource(vs_src);
  fs = fsH.loadFromSource(fs_src);
  }

void Surface::buildVbo( VertexBufferHolder & vbHolder,
                        IndexBufferHolder  & /*ibHolder*/ ) {
  PaintDev p(*this, sstk);
  PaintEvent e(p,0);

  invW =  2.0f/w();
  invH = -2.0f/h();

  invTw = 1;
  invTh = 1;

  cpuGm.clear();
  blocks.clear();

  this->paintEvent(e);

  vbo = Tempest::VertexBuffer<Vertex>();
  vbo = vbHolder.load( cpuGm );

  if( !vdecl.isValid() )
    vdecl = Tempest::VertexDeclaration( vbHolder.device(), decl() );
  }

void Surface::renderTo( Device &dev ) {
  for( size_t i=0; i<blocks.size(); ++i ){
    Block& b = blocks[i];
    dev.setRenderState( rstate[b.state.bm] );

    if( b.curTex && !b.curTex->pageRect().isEmpty() )
      dev.setUniform( fs, b.curTex->pageRawData(),   "texture" );
    if( b.curTex2d )
      dev.setUniform( fs, *b.curTex2d, "texture" );

    int sz = b.isLine? 2:3;
    dev.drawPrimitive( b.isLine? AbstractAPI::Lines : AbstractAPI::Triangle,
                       vs, fs,
                       vdecl,
                       vbo,
                       b.begin,
                       (b.end-b.begin)/sz );
    }
  }

void Surface::quad( int  x, int  y, int  w, int  h,
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
    v[i].color[0] = 1;
    v[i].color[1] = 1;
    v[i].color[2] = 1;
    v[i].color[3] = 1;

    v[i].x -= 1;
    v[i].y += 1;// - v[i].y;
    }

  if( curTex ){
    const Tempest::Size s = curTex->pageRawData().size();
    const Tempest::Rect r = curTex->pageRect();

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

void Surface::line(int x, int y, int x2, int y2) {
  Vertex v[2];
  v[0].x = x*invW;
  v[0].y = y*invH;

  v[1].x = x2*invW;
  v[1].y = y2*invH;

  for( int i=0; i<2; ++i ){
    v[i].color[0] = 1;
    v[i].color[1] = 1;
    v[i].color[2] = 1;
    v[i].color[3] = 1;

    v[i].u = 0;
    v[i].v = 0;

    v[i].x -= 1;
    v[i].y += 1;
    }

  cpuGm.push_back( v[0] );
  cpuGm.push_back( v[1] );

  updateBackBlock(true);
  }

void Surface::updateBackBlock( bool isLine ) {
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
      blocks.back().curTex   != curTex ||
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

const Tempest::VertexDeclaration::Declarator &Surface::decl() {
  static Tempest::VertexDeclaration::Declarator vdecl = declImpl();
  return vdecl;
  }

const VertexDeclaration::Declarator Surface::declImpl() {
  Tempest::VertexDeclaration::Declarator decl;
  decl.add( Tempest::Decl::float2, Tempest::Usage::Position )
      .add( Tempest::Decl::float2, Tempest::Usage::TexCoord, 0 )
      .add( Tempest::Decl::float4, Tempest::Usage::TexCoord, 1 );

  return decl;
  }

void Surface::PaintDev::quad( int x, int y, int w, int h,
                              int tx, int ty, int tw, int th ) {
  surf.quad(x,y,w,h, tx,ty,tw,th);
  }

void Surface::PaintDev::line(int x, int y, int x2, int y2) {
  surf.line(x,y,x2,y2);
  }

void Surface::PaintDev::setBlendMode(BlendMode m) {
  surf.state.bm = m;
  }

void Surface::PaintDev::setNullState() {
  surf.state.bm = Tempest::noBlend;
  surf.state.cl = Tempest::Color(1);
  }

void Surface::PaintDev::pushState() {
  surf.sstk.push_back( surf.state );
  }

void Surface::PaintDev::popState() {
  surf.state = sstk.back();
  surf.sstk.pop_back();
  }

Surface::PaintDev::PaintDev(Surface &s, std::vector<Surface::RState> &sstk )
  :surf(s), sstk(sstk) {
  surf.curTex   = 0;
  surf.curTex2d = 0;
  }

void Surface::PaintDev::setTexture(const PainterDevice::Texture &) {
  surf.invTw = 1;
  surf.invTh = 1;
  }

void Surface::PaintDev::setTexture(const Texture2d &t) {
  surf.invTw = 1.0f/t.width();
  surf.invTh = 1.0f/t.height();

  unsetTexture();
  surf.curTex2d = &t;
  }

void Surface::PaintDev::setTexture(const Sprite &t) {
  surf.invTw = 1.0f/t.width();
  surf.invTh = 1.0f/t.height();

  unsetTexture();
  surf.curTex = &t;
  }

void Surface::PaintDev::unsetTexture() {
  surf.curTex   = 0;
  surf.curTex2d = 0;
  }
