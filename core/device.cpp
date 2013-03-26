#include "device.h"

#include <Tempest/AbstractAPI>
#include <Tempest/Texture2d>

#include <set>
#include <cassert>

#include <iostream>

using namespace Tempest;

struct Device::Data{
  typedef std::set< AbstractHolderBase* > Holders;
  typedef Holders::iterator HIterator;

  Holders holders;

  typedef std::set< VertexDeclaration* > Declarations;
  typedef Declarations::iterator VDIterator;

  Declarations declarations;

  RenderTaget* currentRT;
  Tempest::AbstractAPI::StdDSSurface* depthStencil;

  struct {
    AbstractAPI::VertexBuffer* vbo;
    int vboSize;

    AbstractAPI::IndexBuffer*  ibo;
    } cash;

  int mrtSize;

  bool isLost;
  void * windowHwnd;
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

  data->windowHwnd = windowHwnd;
  data->isLost = 0;

  data->cash.vbo     = 0;
  data->cash.vboSize = 0;
  data->cash.ibo     = 0;
  data->depthStencil = api.getDSSurfaceTaget(impl);
  }

Device::~Device(){
  api.retDSSurfaceTaget( impl, data->depthStencil );
  delete data;

  api.deleteShadingLang( shLang );
  api.deleteDevice( impl );
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
  api.beginPaint(impl);
  shadingLang().beginPaint();
  }

void Device::beginPaint( Texture2d &rt ) {
  data->currentRT = 0;
  data->mrtSize   = 1;

  api.setDSSurfaceTaget( impl, (AbstractAPI::Texture*)0 );
  api.setRenderTaget( impl, rt.data.value(), 0,
                      0 );

  api.beginPaint(impl);
  shadingLang().beginPaint();
  }

void Device::beginPaint( Texture2d &rt, Texture2d &depthStencil ) {
  data->currentRT = 0;
  data->mrtSize   = 1;

  api.setDSSurfaceTaget( impl, depthStencil.data.value() );
  api.setRenderTaget( impl, rt.data.value(), 0,
                      0 );

  api.beginPaint(impl);
  shadingLang().beginPaint();
  }

void Device::beginPaint( Texture2d rt[], int count ){
  data->currentRT = 0;
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
  data->currentRT = 0;
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
  shadingLang().endPaint();

  data->cash.vbo     = 0;
  data->cash.vboSize = 0;
  data->cash.ibo     = 0;

  if( data->mrtSize!=0 ){
    api.endPaint(impl);
    api.unsetRenderTagets( impl, data->mrtSize );

    data->mrtSize = 0;
    } else

  if( data->currentRT==0 ){
    api.endPaint(impl);
    } else {
    //api.endPaint( impl,
    //              data->currentRT->tex.data.value() );
    data->currentRT = 0;
    }  

  api.setDSSurfaceTaget( impl, data->depthStencil );
  }


void Device::setRenderState( const RenderState & r ) const {
  api.setRenderState( impl, r );
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
    invalidateDeviceObjects();

    if( api.reset( impl, data->windowHwnd, opt ) ){
      data->isLost = !restoreDeviceObjects();
      } else {
      assert(0);
      }

    return data->isLost;
    }

  return 0;
  }

void Device::present(){
  Data::HIterator end = data->holders.end();

  for( Data::HIterator i = data->holders.begin(); i!=end; ++i ){
    (*i)->presentEvent();
    }

  data->isLost = api.present( impl );
  }

bool Device::hasManagedStorge() const {
  return api.hasManagedStorge();
  }

AbstractAPI::Texture *Device::createTexture( const Pixmap &p,
                                             bool mips,
                                             bool compress ) {
  return api.createTexture( impl, p, mips, compress );
  }

AbstractAPI::Texture *Device::recreateTexture( AbstractAPI::Texture *t,
                                               const Pixmap &p,
                                               bool mips,
                                               bool compress ) {
  return api.recreateTexture(impl, t,p,mips, compress);
  }

AbstractAPI::Texture* Device::createTexture( int w, int h,
                                             int mips,
                                             AbstractTexture::Format::Type f,
                                             TextureUsage u ) {
  AbstractAPI::Texture* t = api.createTexture( impl, w, h, mips, f, u );
  return t;
  }

void Device::deleteTexture( AbstractAPI::Texture* & t ){
  api.deleteTexture( impl, t );
  }
/*
AbstractAPI::RenderTagetSurface Device::createRenderTaget( Texture2d &t,
                                                      int mipLevel ) const {
  return api.createRenderTaget( impl, t.data.const_value(), mipLevel );
  }

void Device::deleteRenderTaget( AbstractAPI::RenderTagetSurface t) const {
  api.deleteRenderTaget( impl, t );
  }*/

AbstractAPI::VertexBuffer* Device::createVertexbuffer( size_t size, size_t el ){
  return api.createVertexbuffer( impl, size, el );
  }

void Device::deleteVertexBuffer( AbstractAPI::VertexBuffer* vbo ){
  api.deleteVertexBuffer( impl, vbo );
  }

AbstractAPI::IndexBuffer* Device::createIndexBuffer( size_t size,
                                                     size_t elSize ){
  return api.createIndexbuffer( impl, size, elSize );
  }

void Device::deleteIndexBuffer( AbstractAPI::IndexBuffer* b ){
  api.deleteIndexBuffer( impl, b);
  }

void* Device::lockBuffer( AbstractAPI::VertexBuffer * vbo,
                          unsigned off, unsigned size ){
  return api.lockBuffer( impl, vbo, off, size);
  }

void Device::unlockBuffer( AbstractAPI::VertexBuffer* vbo){
  api.unlockBuffer( impl, vbo );
  }

void* Device::lockBuffer( AbstractAPI::IndexBuffer * ibo,
                          unsigned offset, unsigned size ){
  return api.lockBuffer( impl, ibo, offset, size );
  }

void Device::unlockBuffer( AbstractAPI::IndexBuffer * ibo ) {
  api.unlockBuffer( impl, ibo );
  }

AbstractAPI::VertexDecl *
      Device::createVertexDecl( const VertexDeclaration::Declarator &de ) const {
  return api.createVertexDecl( impl, de );
  }

void Device::deleteVertexDecl( AbstractAPI::VertexDecl* d ) const {
  api.deleteVertexDecl( impl, d );
  }

const AbstractShadingLang& Device::shadingLang(){
  return *shLang;
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
                                   int minIndex,
                                   int vertexCount,
                                   int firstIndex,
                                   int pCount ) {
  api.drawIndexed( impl,
                   t,
                   vboOffsetIndex,
                   minIndex,
                   vertexCount,
                   firstIndex,
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

  {/*
    Data::RTIterator end = data->renderTagets.end();

    for( Data::RTIterator i = data->renderTagets.begin(); i!=end; ++i ){
      (*i)->reset();
      }*/
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

  {/*
    Data::RTIterator end = data->renderTagets.end();

    for( Data::RTIterator i = data->renderTagets.begin(); i!=end; ++i ){
      ok &= (*i)->restore();
      }*/
    }

  if( ok )
    data->depthStencil = api.getDSSurfaceTaget(impl);

  return ok;
  }
