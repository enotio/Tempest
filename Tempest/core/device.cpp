#include "device.h"

#include <Tempest/AbstractAPI>
#include <Tempest/Texture2d>
#include <Tempest/RenderState>
#include <Tempest/VertexBufferHolder>
#include <Tempest/VertexBuffer>
#include <Tempest/Assert>

#include <set>
#include <unordered_set>
#include <sstream>

using namespace Tempest;

struct Device::Data{
  typedef std::unordered_set< AbstractHolderBase* > Holders;
  typedef Holders::iterator HIterator;

  Holders holders;
  Device::Options actualOptions;

  typedef std::set< VertexDeclaration* > Declarations;
  typedef Declarations::iterator VDIterator;

  Declarations declarations;

  Tempest::AbstractAPI::StdDSSurface* depthStencil;

  struct {
    AbstractAPI::VertexBuffer* vbo;
    int vboSize;

    AbstractAPI::IndexBuffer*  ibo;

    void reset(){
      vbo = 0;
      ibo = 0;
      vboSize = 0;
      }
    } cash;

  int mrtSize;

  bool isLost;
  bool isPaintMode;
  void * windowHwnd;

  Tempest::Size viewPortSize;

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

  struct AllocCount {
    AllocCount(){
      vbo = 0;
      ibo = 0;
      tex = 0;
      dec = 0;
      }

    ~AllocCount(){
      if( !(vbo==0 && ibo==0 && tex==0 && dec==0) ){
        std::stringstream ss;
        ss << "resource leak detected" << std::endl
           << "vbo:  " << vbo << std::endl
           << "ibo:  " << ibo << std::endl
           << "tex:  " << tex << std::endl
           << "dec:  " << dec << std::endl;

        T_ASSERT_X( vbo==0 && ibo==0 && tex==0 && dec==0, ss.str().c_str() );
        }
      }

    int vbo, ibo, tex, dec;
    } allockCount;

  template< class ... Args >
  AbstractAPI::Texture* createTexture( const AbstractAPI  &api,
                                       AbstractAPI::Texture*
                                         (AbstractAPI::*c)( AbstractAPI::Device *impl, const Pixmap &p, Args...) const,
                                       Device &dev,
                                       AbstractAPI::Device *impl,
                                       const Pixmap &p,
                                       Args&...args ){
    AbstractAPI::Texture * tx = 0;
    if( api.isFormatSupported(impl, p.format()) ){
      AbstractAPI::Caps caps = dev.caps();

      if( !caps.hasNpotTexture||
          p.width()  > caps.maxTextureSize ||
          p.height() > caps.maxTextureSize  ){
        bool isPot = ((p.width()  &(p.width() -1))==0) &&
                     ((p.height() &(p.height()-1))==0);
        if( !isPot ){
          Pixmap px = p;
          px.toPOT( caps.maxTextureSize );
          return createTexture( api,c,dev, impl,px,args... );
          }
        }

      tx = (api.*c)( impl, p, args... );
      } else {
      Pixmap px = p;
      px.setFormat( p.hasAlpha() ? Pixmap::Format_RGBA:Pixmap::Format_RGB );
      return createTexture( api,c,dev, impl,px,args... );
      }

    return tx;
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
  impl   = api.createDevice( windowHwnd, opt );

  shLang = api.createShadingLang( impl );

  data = new Data();
  data->declPool.reserve(64);

  data->windowHwnd = windowHwnd;
  data->isLost = 0;
  data->isPaintMode = false;

  data->cash.reset();
  data->depthStencil = api.getDSSurfaceTaget(impl);

  data->delaydRS = false;

  data->vboH = new VertexBufferHolder(*this);

  Tempest::VertexDeclaration::Declarator decl;
  decl.add( Tempest::Decl::float2, Tempest::Usage::Position )
      .add( Tempest::Decl::float2, Tempest::Usage::TexCoord );

  data->quadDecl = Tempest::VertexDeclaration( *this, decl );

  static const Data::QuadVertex q[6] = {
    {-1,-1,  0,1},
    { 1, 1,  1,0},
    { 1,-1,  1,1},

    {-1,-1,  0,1},
    {-1, 1,  0,0},
    { 1, 1,  1,0}
    };

  data->quad = data->vboH->load(q, 6);

  data->viewPortSize = windowSize();
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

std::string Device::vendor() const {
  return api.vendor( impl );
  }

std::string Device::renderer() const {
  return api.renderer( impl );
  }

void Device::clear(const Color &cl, float z, unsigned s) {
  applyRs();
  api.clear(impl, cl, z, s);
  }

void Device::clear( const Color& cl ) {
  applyRs();
  api.clear(impl, cl );
  }

void Device::clear( const Color& cl, float z ) {
  applyRs();
  api.clear(impl, cl, z);
  }

void Device::clear(float z, unsigned s ) {
  applyRs();
  api.clear(impl, z, s);
  }

void Device::clearZ(float z ) {
  applyRs();
  api.clearZ(impl, z);
  }

void Device::clearStencil( unsigned s ) {
  applyRs();
  api.clearStencil(impl, s);
  }

bool Device::testDisplaySettings( const DisplaySettings &s ) {
  return api.testDisplaySettings(s);
  }

bool Device::setDisplaySettings( const DisplaySettings &s ) {
  data->actualOptions.displaySettings = s;
  return reset( data->actualOptions );
  }

void Device::beginPaint(){
  T_ASSERT_X( !data->isPaintMode, "recursive paint detected" );
  data->isPaintMode = true;

  if( data->paintTaget.isDelayd && data->paintTaget.isSame() ){
    return;
    }

  forceEndPaint();
  data->paintTaget.setup();
  data->viewPortSize = windowSize();

  beginPaintImpl();
  }

void Device::beginPaint( Texture2d &rt ) {
  T_ASSERT_X( !data->isPaintMode, "recursive paint detected" );
  data->isPaintMode = true;

  if( data->paintTaget.isDelayd && data->paintTaget.isSame(rt) ){
    return;
    }

  forceEndPaint();
  data->paintTaget.setup(rt);

  beginPaintImpl();
  }

void Device::beginPaint( Texture2d &rt, Texture2d &depthStencil ) {
  T_ASSERT_X( !data->isPaintMode, "recursive paint detected" );
  data->isPaintMode = true;

  if( data->paintTaget.isDelayd && data->paintTaget.isSame(rt, depthStencil) ){
    return;
    }

  forceEndPaint();
  data->paintTaget.setup(rt, depthStencil);

  beginPaintImpl();
  }

void Device::beginPaint( Texture2d rt[], int count ){
  T_ASSERT_X( !data->isPaintMode, "recursive paint detected" );
  data->isPaintMode = true;

  if( data->paintTaget.isDelayd && data->paintTaget.isSame(rt, count) ){
    return;
    }

  forceEndPaint();
  data->paintTaget.setup(rt, count);

  beginPaintImpl();
  }

void Device::beginPaint( Texture2d rt[], int count,
                         Texture2d &depthStencil) {
  T_ASSERT_X( !data->isPaintMode, "recursive paint detected" );
  data->isPaintMode = true;

  if( data->paintTaget.isDelayd && data->paintTaget.isSame(rt, count, depthStencil) ){
    return;
    }

  forceEndPaint();
  data->paintTaget.setup(rt, count, depthStencil);

  beginPaintImpl();
  }

void Device::endPaint  (){
  T_ASSERT_X( data->isPaintMode, "invalid endPaint call" );
  data->isPaintMode         = false;
  data->paintTaget.isDelayd = true;
  data->viewPortSize = windowSize();
  //forceEndPaint();
  }

void Device::beginPaintImpl() const {
  data->mrtSize   = data->paintTaget.mrt.size();

  if( data->paintTaget.ds ){
    api.setDSSurfaceTaget( impl, data->paintTaget.ds );
    } else {
    api.retDSSurfaceTaget( impl, data->depthStencil  );
    }

  for( int i=0; i<data->mrtSize; ++i ){
    api.setRenderTaget( impl,
                        data->paintTaget.mrt[i], 0,
                        i );
    }
  //data->viewPortSize = depthStencil.size();

  api.beginPaint(impl);
  shadingLang().beginPaint();
  data->cash.reset();
  }

void Device::endPaintImpl() const {
  shadingLang().endPaint();

  data->paintTaget.isDelayd = false;
  data->cash.reset();

  if( data->mrtSize!=0 ){
    api.endPaint(impl);
    api.unsetRenderTagets( impl, data->mrtSize );

    data->mrtSize = 0;
    } else {
    api.endPaint(impl);
    }

  api.setDSSurfaceTaget( impl, data->depthStencil );
  }

void Device::wrapPaintBegin() const {
  if( data->isPaintMode )
    endPaintImpl();
  }

void Device::wrapPaintEnd() const {
  if( data->isPaintMode )
    beginPaintImpl();
  }

void Device::forceEndPaint() const {
  if( !data->paintTaget.isDelayd )
    return;

  endPaintImpl();
  }

void Device::setRenderState( const RenderState & r ) const {
  data->delaydRS = true;
  data->rs       = r;
  }

const RenderState &Device::renderState() const {
  return data->rs;
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
      T_ASSERT_X( 0, "reset device error" );
      }

    data->actualOptions = opt;
    if( !data->isLost )
      onRestored();

    return data->isLost;
    }

  return 0;
  }

void Device::present( AbstractAPI::SwapBehavior b ){
  forceEndPaint();

  Data::HIterator end = data->holders.end();

  for( Data::HIterator i = data->holders.begin(); i!=end; ++i ){
    (*i)->presentEvent();
    }

  data->isLost = api.present( impl, b );
  }

bool Device::hasManagedStorge() const {
  return api.hasManagedStorge();
  }

void Device::drawFullScreenQuad(const ProgramObject &p) {
  drawFullScreenQuad( p.vs, p.fs );
  }

void Device::drawFullScreenQuad( const VertexShader   &vs,
                                 const FragmentShader &fs ) {
  drawPrimitive( AbstractAPI::Triangle, vs, fs,
                 data->quadDecl, data->quad,
                 0, 2 );
  }

Size Device::windowSize() const {
  return api.windowSize(impl);
  }

Size Device::viewPortSize() const {
  return data->viewPortSize;
  }

std::string Device::surfaceShader( AbstractShadingLang::ShaderType t,
                                   const AbstractShadingLang::UiShaderOpt &opt,
                                   bool &hasHalfpixOffset) {
  return shadingLang().surfaceShader(t, opt, hasHalfpixOffset);
  }

std::string Device::surfaceShader( AbstractShadingLang::ShaderType t,
                                   const AbstractShadingLang::UiShaderOpt &opt ) {
  return shadingLang().surfaceShader(t, opt);
  }

AbstractAPI::Texture *Device::createTexture( const Pixmap &p,
                                             bool mips,
                                             bool compress ) {
  forceEndPaint();

  AbstractAPI::Texture * tx = data->createTexture( api,
                                                   &AbstractAPI::createTexture,
                                                   *this,
                                                   impl, p, mips, compress );
  if( tx )
    data->allockCount.tex++;

  return tx;
  }

AbstractAPI::Texture *Device::recreateTexture( const Pixmap &p,
                                               bool mips,
                                               bool compress,
                                               AbstractAPI::Texture *t
                                               ) {
  forceEndPaint();
  return data->createTexture( api,
                              &AbstractAPI::recreateTexture,
                              *this,
                              impl, p, mips, compress, t );
  }

AbstractAPI::Texture* Device::createTexture( int w, int h,
                                             bool mips,
                                             AbstractTexture::Format::Type f,
                                             TextureUsage u ) {
  forceEndPaint();
  AbstractAPI::Texture* t = api.createTexture( impl, w, h, mips, f, u );
  if( t )
    data->allockCount.tex++;
  return t;
  }

AbstractAPI::Texture *Device::createTexture3d( int x, int y, int z,
                                               bool mips,
                                               const char* d,
                                               AbstractTexture::Format::Type f,
                                               TextureUsage u ) {
  forceEndPaint();
  AbstractAPI::Texture* t = api.createTexture3d( impl, x, y, z, mips, f, u, d );
  if( t )
    data->allockCount.tex++;
  return t;
  }

void Device::deleteTexture( AbstractAPI::Texture* & t ){
  forceEndPaint();
  data->allockCount.tex--;
  api.deleteTexture( impl, t );
  }

void Device::setTextureFlag(AbstractAPI::Texture *t, AbstractAPI::TextureFlag f, bool v) {
  api.setTextureFlag(impl, t, f, v);
  }

AbstractAPI::VertexBuffer* Device::createVertexbuffer( size_t size, size_t el,
                                                       AbstractAPI::BufferUsage u ){
  forceEndPaint();
  AbstractAPI::VertexBuffer* v = api.createVertexBuffer( impl, size, el, u );
  if( v )
    data->allockCount.vbo++;
  return v;
  }

AbstractAPI::VertexBuffer* Device::createVertexbuffer( size_t size, size_t el,
                                                       const void* src,
                                                       AbstractAPI::BufferUsage u ){
  forceEndPaint();
  AbstractAPI::VertexBuffer* v =  api.createVertexBuffer( impl, size, el, src, u );
  if( v )
    data->allockCount.vbo++;

  return v;
  }

void Device::deleteVertexBuffer( AbstractAPI::VertexBuffer* vbo ){
  forceEndPaint();
  data->allockCount.vbo--;
  api.deleteVertexBuffer( impl, vbo );
  }

AbstractAPI::IndexBuffer* Device::createIndexBuffer( size_t size,
                                                     size_t elSize,
                                                     AbstractAPI::BufferUsage u  ){
  forceEndPaint();
  AbstractAPI::IndexBuffer* i = api.createIndexBuffer( impl, size, elSize, u );
  if( i )
    data->allockCount.ibo++;

  return i;
  }

AbstractAPI::IndexBuffer *Device::createIndexBuffer( size_t size,
                                                     size_t elSize,
                                                     const void *src,
                                                     AbstractAPI::BufferUsage u ) {
  forceEndPaint();
  AbstractAPI::IndexBuffer* i =  api.createIndexBuffer( impl, size, elSize, src, u );
  if( i )
    data->allockCount.ibo++;

  return i;
  }

void Device::deleteIndexBuffer( AbstractAPI::IndexBuffer* b ){
  forceEndPaint();
  data->allockCount.ibo--;
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

  if( !(api.caps(impl).hasHalf2 || api.caps(impl).hasHalf4) )
    for( int i=0; i<de.size(); ++i )
      if( de[i].component==Decl::half2 ||
          de[i].component==Decl::half4 )
        T_WARNING_X(0, "half float unaviable");

  AbstractAPI::VertexDecl * v = data->createDecl(de);
  if( !v ){
    v = api.createVertexDecl( impl, de );
    data->addDecl(de, v);
    }

  if( v )
    data->allockCount.dec++;

  return v;
  }

void Device::deleteVertexDecl( AbstractAPI::VertexDecl* d ) const {
  forceEndPaint();

  if( data->deleteDecl(d) )
    api.deleteVertexDecl( impl, d );

  data->allockCount.dec--;
  }

void Device::assertPaint() {
  T_ASSERT_X( data->isPaintMode, "Device::beginPaint not called" );
  }

void Device::bindShaders( const VertexShader &vs,
                          const FragmentShader &fs ) {
  bind(vs);
  bind(fs);

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
