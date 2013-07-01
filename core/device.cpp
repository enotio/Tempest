#include "device.h"

#include <Tempest/AbstractAPI>
#include <Tempest/Texture2d>
#include <Tempest/RenderState>
#include <Tempest/VertexBufferHolder>
#include <Tempest/VertexBuffer>

#include <set>
#include <cassert>


using namespace Tempest;

struct Device::Data{
  typedef std::set< AbstractHolderBase* > Holders;
  typedef Holders::iterator HIterator;

  Holders holders;

  typedef std::set< VertexDeclaration* > Declarations;
  typedef Declarations::iterator VDIterator;

  Declarations declarations;

  Tempest::AbstractAPI::StdDSSurface* depthStencil;

  struct {
    AbstractAPI::VertexBuffer* vbo;
    int vboSize;

    AbstractAPI::IndexBuffer*  ibo;
    } cash;

  int mrtSize;

  bool isLost;
  bool isPaintMode;
  void * windowHwnd;  

  struct BeginPaintArg{
    BeginPaintArg(){
      ds        = 0;
      isDelayd  = 0;
      mrt.reserve(32);
      }

    std::vector<AbstractAPI::Texture*> mrt;
    AbstractAPI::Texture* ds;
    bool isDelayd;

    void setup(){
      mrt.clear();
      ds = 0;
      }

    void setup( Texture2d rt[], int count ){
      mrt.resize(count);

      for( int i=0; i<count; ++i )
        mrt[i] = rt[i].data.value();

      ds = 0;
      }

    void setup( Texture2d rt[], int count, Texture2d &d ){
      mrt.resize(count);

      for( int i=0; i<count; ++i )
        mrt[i] = rt[i].data.value();

      ds = d.data.value();
      }

    void setup( Texture2d& rt, Texture2d &d ){
      mrt.resize(1);
      mrt[0] = rt.data.value();
      ds = d.data.value();
      }

    void setup( Texture2d& rt ){
      mrt.resize(1);
      mrt[0] = rt.data.value();
      ds = 0;
      }

    //

    bool isSame(){
      return mrt.size()==0 && ds==0;
      }

    bool isSame( Texture2d rt[], int count ){
      if( int(mrt.size())!=count )
        return 0;

      for( int i=0; i<count; ++i )
        if( mrt[i]!=rt[i].data.value() )
          return 0;

      return ds==0;
      }

    bool isSame( Texture2d rt[], int count, Texture2d &d ){
      if( int(mrt.size())!=count )
        return 0;

      for( int i=0; i<count; ++i )
        if( mrt[i]!=rt[i].data.value() )
          return 0;

      return ds==d.data.value();
      }

    bool isSame( Texture2d& rt, Texture2d &d ){
      if( int(mrt.size())!=1 )
        return 0;

      return mrt[0]==rt.data.value() && ds==d.data.value();
      }

    bool isSame( Texture2d& rt ){
      if( int(mrt.size())!=1 )
        return 0;

      return mrt[0]==rt.data.value() && ds==0;
      }
    };

  BeginPaintArg paintTaget;

  RenderState rs;
  bool  delaydRS;

  Tempest::VertexDeclaration quadDecl;
  struct QuadVertex{
    float x,y;
    float u,v;
    };

  VertexBufferHolder* vboH;
  Tempest::VertexBuffer<QuadVertex> quad;

  struct Dec{
    AbstractAPI::VertexDecl* v;
    VertexDeclaration::Declarator d;
    size_t count;
    };

  std::vector<Dec> declPool;

  AbstractAPI::VertexDecl* createDecl( const VertexDeclaration::Declarator& d ){
    for( size_t i=0; i<declPool.size(); ++i )
      if( declPool[i].d==d ){
        ++declPool[i].count;
        return declPool[i].v;
        }

    return 0;
    }

  void addDecl( const VertexDeclaration::Declarator& de,
                AbstractAPI::VertexDecl* v ){
    Dec d;
    d.v = v;
    d.d = de;
    d.count = 1;

    declPool.push_back(d);
    }

  bool deleteDecl( AbstractAPI::VertexDecl *v ){
    for( size_t i=0; i<declPool.size(); ++i )
      if( declPool[i].v==v ){
        --declPool[i].count;
        if( declPool[i].count==0 ){
          declPool[i] = declPool.back();
          declPool.pop_back();
          return 1;
          } else {
          return 0;
          }
        }

    return 1;
    }
  };


Device::Device(const AbstractAPI &dx,
               void *windowHwnd):api(dx){
  init(dx, Options(), windowHwnd);
  }

Device::Device( const AbstractAPI & dx,
                const Options &opt,
                void *windowHwnd ):api(dx){
  init(dx, opt, windowHwnd);
  }

void Device::init( const AbstractAPI &,
                   const Device::Options &opt,
                   void *windowHwnd ) {
  impl = api.createDevice( windowHwnd, opt );

  shLang = api.createShadingLang( impl );

  data = new Data();
  data->declPool.reserve(64);

  data->windowHwnd = windowHwnd;
  data->isLost = 0;
  data->isPaintMode = false;

  data->cash.vbo     = 0;
  data->cash.vboSize = 0;
  data->cash.ibo     = 0;
  data->depthStencil = api.getDSSurfaceTaget(impl);

  data->delaydRS = false;

  data->vboH = new VertexBufferHolder(*this);

  Tempest::VertexDeclaration::Declarator decl;
  decl.add( Tempest::Decl::float2, Tempest::Usage::Position )
      .add( Tempest::Decl::float2, Tempest::Usage::TexCoord );

  data->quadDecl = Tempest::VertexDeclaration( *this, decl );

  Data::QuadVertex q[6] = {
    {-1,-1,  0,1},
    { 1, 1,  1,0},
    { 1,-1,  1,1},

    {-1,-1,  0,1},
    {-1, 1,  0,0},
    { 1, 1,  1,0}
    };

  data->quad = data->vboH->load(q, 6);
  }

Device::~Device(){
  data->quad     = VertexBuffer<Data::QuadVertex>();
  data->quadDecl = VertexDeclaration();

  delete data->vboH;

  api.retDSSurfaceTaget( impl, data->depthStencil );
  delete data;

  api.deleteShadingLang( shLang );
  api.deleteDevice( impl );
  }

AbstractAPI::Caps Device::caps() const {
  return api.caps( impl );
  }

void Device::clear(const Color &cl, float z, unsigned s) {
  api.clear(impl, cl, z, s);
  }

void Device::clear( const Color& cl ) {
  api.clear(impl, cl );
  }

void Device::clear( const Color& cl, float z ) {
  api.clear(impl, cl, z);
  }

void Device::clear(float z, unsigned s ) {
  api.clear(impl, z, s);
  }

void Device::clearZ(float z ) {
  api.clearZ(impl, z);
  }

void Device::clearStencil( unsigned s ) {
  api.clearStencil(impl, s);
  }

void Device::beginPaint(){
  assert( !data->isPaintMode );
  data->isPaintMode = true;

  if( data->paintTaget.isDelayd && data->paintTaget.isSame() ){
    return;
    }

  forceEndPaint();
  data->paintTaget.setup();

  api.beginPaint(impl);
  shadingLang().beginPaint();
  }

void Device::beginPaint( Texture2d &rt ) {
  assert( !data->isPaintMode );
  data->isPaintMode = true;

  if( data->paintTaget.isDelayd && data->paintTaget.isSame(rt) ){
    return;
    }

  forceEndPaint();
  data->paintTaget.setup(rt);

  data->mrtSize   = 1;

  api.setDSSurfaceTaget( impl, (AbstractAPI::Texture*)0 );
  api.setRenderTaget( impl, rt.data.value(), 0,
                      0 );

  api.beginPaint(impl);
  shadingLang().beginPaint();
  }

void Device::beginPaint( Texture2d &rt, Texture2d &depthStencil ) {
  assert( !data->isPaintMode );
  data->isPaintMode = true;

  if( data->paintTaget.isDelayd && data->paintTaget.isSame(rt, depthStencil) ){
    return;
    }

  forceEndPaint();
  data->paintTaget.setup(rt, depthStencil);

  data->mrtSize   = 1;

  api.setDSSurfaceTaget( impl, depthStencil.data.value() );
  api.setRenderTaget( impl, rt.data.value(), 0,
                      0 );

  api.beginPaint(impl);
  shadingLang().beginPaint();
  }

void Device::beginPaint( Texture2d rt[], int count ){
  assert( !data->isPaintMode );
  data->isPaintMode = true;

  if( data->paintTaget.isDelayd && data->paintTaget.isSame(rt, count) ){
    return;
    }

  forceEndPaint();
  data->paintTaget.setup(rt, count);

  data->mrtSize   = count;

  api.setDSSurfaceTaget( impl, (AbstractAPI::Texture*)0 );
  for( int i=0; i<count; ++i )
    api.setRenderTaget( impl,
                        rt[i].data.value(), 0,
                        i );

  api.beginPaint(impl);
  shadingLang().beginPaint();
  }

void Device::beginPaint( Texture2d rt[], int count,
                         Texture2d &depthStencil) {
  assert( !data->isPaintMode );
  data->isPaintMode = true;

  if( data->paintTaget.isDelayd && data->paintTaget.isSame(rt, count, depthStencil) ){
    return;
    }

  forceEndPaint();
  data->paintTaget.setup(rt, count, depthStencil);

  data->mrtSize   = count;

  api.setDSSurfaceTaget( impl, depthStencil.data.value() );

  for( int i=0; i<count; ++i )
    api.setRenderTaget( impl,
                        rt[i].data.value(), 0,
                        i );

  api.beginPaint(impl);
  shadingLang().beginPaint();
  }

void Device::endPaint  (){
  assert( data->isPaintMode );
  data->isPaintMode        = false;
  data->paintTaget.isDelayd = true;
  //forceEndPaint();
  }

void Device::forceEndPaint() const {
  if( !data->paintTaget.isDelayd )
    return;

  shadingLang().endPaint();

  data->paintTaget.isDelayd = false;
  data->cash.vbo     = 0;
  data->cash.vboSize = 0;
  data->cash.ibo     = 0;

  if( data->mrtSize!=0 ){
    api.endPaint(impl);
    api.unsetRenderTagets( impl, data->mrtSize );

    data->mrtSize = 0;
    } else {
    api.endPaint(impl);
    }

  api.setDSSurfaceTaget( impl, data->depthStencil );
  }

void Device::setRenderState( const RenderState & r ) const {
  data->delaydRS = true;
  data->rs       = r;
  }

void Device::applyRs() const {
  if( data->delaydRS ){
    api.setRenderState( impl, data->rs );
    data->delaydRS = false;
    }
  }

bool Device::startRender(){
  if( data->isLost ){

    if( api.startRender( impl, true ) ){
      data->isLost = reset();
      return data->isLost;
      }

    return 0;
    }

  api.startRender( impl, false );
  return 1;
  }

bool Device::reset( const Options & opt ) {
  if( api.startRender( impl, data->isLost ) ){
    forceEndPaint();

    if( !hasManagedStorge() )
      invalidateDeviceObjects();

    if( api.reset( impl, data->windowHwnd, opt ) ){
      if( !hasManagedStorge() )
        data->isLost = !restoreDeviceObjects();
      } else {
      assert(0);
      }

    return data->isLost;
    }

  return 0;
  }

void Device::present(){
  forceEndPaint();

  Data::HIterator end = data->holders.end();

  for( Data::HIterator i = data->holders.begin(); i!=end; ++i ){
    (*i)->presentEvent();
    }

  data->isLost = api.present( impl );
  }

bool Device::hasManagedStorge() const {
  return api.hasManagedStorge();
  }

void Device::drawFullScreenQuad(VertexShader &vs, FragmentShader &fs) {
  drawPrimitive( AbstractAPI::Triangle, vs, fs,
                 data->quadDecl, data->quad,
                 0, 2 );
  }

AbstractAPI::Texture *Device::createTexture( const Pixmap &p,
                                             bool mips,
                                             bool compress ) {
  forceEndPaint();
  return api.createTexture( impl, p, mips, compress );
  }

AbstractAPI::Texture *Device::recreateTexture( AbstractAPI::Texture *t,
                                               const Pixmap &p,
                                               bool mips,
                                               bool compress ) {
  forceEndPaint();
  return api.recreateTexture(impl, t,p,mips, compress);
  }

AbstractAPI::Texture* Device::createTexture( int w, int h,
                                             bool mips,
                                             AbstractTexture::Format::Type f,
                                             TextureUsage u ) {
  forceEndPaint();
  AbstractAPI::Texture* t = api.createTexture( impl, w, h, mips, f, u );
  return t;
  }

void Device::deleteTexture( AbstractAPI::Texture* & t ){
  forceEndPaint();
  api.deleteTexture( impl, t );
  }

void Device::setTextureFlag(AbstractAPI::Texture *t, AbstractAPI::TextureFlag f, bool v) {
  api.setTextureFlag(impl, t, f, v);
  }

AbstractAPI::VertexBuffer* Device::createVertexbuffer( size_t size, size_t el,
                                                       AbstractAPI::BufferUsage u ){
  forceEndPaint();
  return api.createVertexBuffer( impl, size, el, u );
  }

AbstractAPI::VertexBuffer* Device::createVertexbuffer( size_t size, size_t el,
                                                       const void* src,
                                                       AbstractAPI::BufferUsage u ){
  forceEndPaint();
  return api.createVertexBuffer( impl, size, el, src, u );
  }

void Device::deleteVertexBuffer( AbstractAPI::VertexBuffer* vbo ){
  forceEndPaint();
  api.deleteVertexBuffer( impl, vbo );
  }

AbstractAPI::IndexBuffer* Device::createIndexBuffer( size_t size,
                                                     size_t elSize,
                                                     AbstractAPI::BufferUsage u  ){
  forceEndPaint();
  return api.createIndexBuffer( impl, size, elSize, u );
  }

AbstractAPI::IndexBuffer *Device::createIndexBuffer( size_t size,
                                                     size_t elSize,
                                                     const void *src,
                                                     AbstractAPI::BufferUsage u ) {
  forceEndPaint();
  return api.createIndexBuffer( impl, size, elSize, src, u );
  }

void Device::deleteIndexBuffer( AbstractAPI::IndexBuffer* b ){
  forceEndPaint();
  api.deleteIndexBuffer( impl, b);
  }

void* Device::lockBuffer( AbstractAPI::VertexBuffer * vbo,
                          unsigned off, unsigned size ){
  forceEndPaint();
  return api.lockBuffer( impl, vbo, off, size);
  }

void Device::unlockBuffer( AbstractAPI::VertexBuffer* vbo){
  forceEndPaint();
  api.unlockBuffer( impl, vbo );
  }

void* Device::lockBuffer( AbstractAPI::IndexBuffer * ibo,
                          unsigned offset, unsigned size ){
  forceEndPaint();
  return api.lockBuffer( impl, ibo, offset, size );
  }

void Device::unlockBuffer( AbstractAPI::IndexBuffer * ibo ) {
  forceEndPaint();
  api.unlockBuffer( impl, ibo );
  }

AbstractAPI::VertexDecl *
      Device::createVertexDecl( const VertexDeclaration::Declarator &de ) const {
  forceEndPaint();

  AbstractAPI::VertexDecl * v = data->createDecl(de);
  if( !v ){
    v = api.createVertexDecl( impl, de );
    data->addDecl(de, v);
    }

  return v;
  }

void Device::deleteVertexDecl( AbstractAPI::VertexDecl* d ) const {
  forceEndPaint();

  if( data->deleteDecl(d) )
    api.deleteVertexDecl( impl, d );
  }

void Device::bind( const Tempest::VertexShader &s ){
  shadingLang().bind(s);
  }

void Device::bind( const Tempest::FragmentShader &s ){
  shadingLang().bind(s);
  }

void Device::unBind( const Tempest::VertexShader &s ){
  shadingLang().unBind(s);
  }

void Device::unBind( const Tempest::FragmentShader &s ){
  shadingLang().unBind(s);
  }

void Device::bind( const Tempest::VertexDeclaration & d ){
  Tempest::AbstractAPI::VertexDecl* decl =
      reinterpret_cast<Tempest::AbstractAPI::VertexDecl*>(d.decl->impl);

  api.setVertexDeclaration( impl, decl );
  }
 
void Device::bind( AbstractAPI::VertexBuffer* b, int vsize ){
  if( data->cash.vbo!=b || data->cash.vboSize!=vsize ){
    api.bindVertexBuffer( impl, b, vsize );

    data->cash.vbo     = b;
    data->cash.vboSize = vsize;
    }
  }

void Device::bind( AbstractAPI::IndexBuffer* b ){
  if( data->cash.ibo!=b ){
    api.bindIndexBuffer( impl, b );
    data->cash.ibo = b;
    }
  }

void Device::draw( AbstractAPI::PrimitiveType t,
                   int firstVertex, int pCount ){
  api.draw( impl, t, firstVertex, pCount );
  }

void Device::drawIndexedPrimitive( AbstractAPI::PrimitiveType t,
                                   int vboOffsetIndex,
                                   int iboOffsetIndex,
                                   int pCount ) {
  api.drawIndexed( impl,
                   t,
                   vboOffsetIndex,
                   iboOffsetIndex,
                   pCount );
  }

void Device::addHolder( AbstractHolderBase& h ){
  data->holders.insert( &h );
  }

void Device::delHolder( AbstractHolderBase& h ){
  data->holders.erase ( &h );
  }

void Device::addVertexDeclaration( VertexDeclaration& h ){
  data->declarations.insert( &h );
  }

void Device::delVertexDeclaration( VertexDeclaration& h ){
  data->declarations.erase ( &h );
  }


void Device::invalidateDeviceObjects(){
  forceEndPaint();

  api.retDSSurfaceTaget( impl, data->depthStencil );
  data->depthStencil = 0;

  {
    Data::HIterator end = data->holders.end();

    for( Data::HIterator i = data->holders.begin(); i!=end; ++i ){
      (*i)->reset();
      }
    }

  {
    Data::VDIterator end = data->declarations.end();

    for( Data::VDIterator i = data->declarations.begin(); i!=end; ++i ){
      VertexDeclaration *v = (*i);

      if( v->decl && v->decl->impl ){
        deleteVertexDecl( (AbstractAPI::VertexDecl*)v->decl->impl );
        v->decl->impl = 0;
        }
      }
    }

  }

bool Device::restoreDeviceObjects(){
  bool ok = true;
  {
    Data::HIterator end = data->holders.end();

    for( Data::HIterator i = data->holders.begin(); i!=end; ++i ){
      ok &= (*i)->restore();
      }
    }

  {
    Data::VDIterator end = data->declarations.end();

    for( Data::VDIterator i = data->declarations.begin(); i!=end; ++i ){
      VertexDeclaration *v = (*i);

      if( v->decl && v->decl->impl==0 ){
        v->decl->impl = createVertexDecl( v->decl->decl );
        ok &= (v->decl->impl!=0);
        }
      }
    }

  if( ok )
    data->depthStencil = api.getDSSurfaceTaget(impl);

  return ok;
  }
