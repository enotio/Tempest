#include "renderstate.h"

using namespace Tempest;

RenderState::RenderState(){
  init();
  }

RenderState::RenderState( RenderState::Preset p ) {
  init();

  if( p==PostProcess ){
    setZTest(0);
    }
  }

void RenderState::init() {
  atest     = 0;
  atestType = AlphaTestMode::Always;
  zWriting  = 1;
  zTest     = 1;
  blend     = 0;
  sfactor   = AlphaBlendMode::one;
  dfactor   = AlphaBlendMode::zero;

  alphaCoverage = 0;

  setColorMask(1,1,1,1);

  zTestType = ZTestMode::Less;
  setCullFaceMode( CullMode::back );
  setPolygonRenderMode( PolyRenderMode::Fill,
                        PolyRenderMode::Fill );
  }

double RenderState::alphaTestRef()  const{
    return atest;
    }

void  RenderState::setAlphaTestRef(double v){
    atest = v;
    }

void RenderState::setAlphaTestMode( RenderState::AlphaTestMode::Type mode ){
    atestType = mode;
    }

RenderState::AlphaTestMode::Type RenderState::alphaTestMode() const{
    return atestType;
    }

bool RenderState::isZWriting()  const{
    return zWriting;
    }

void RenderState::setZWriting(bool w){
    zWriting = w;
    }

void RenderState::setColorMask(bool  r, bool  g, bool  b, bool  a){
    clMask[0] = r;
    clMask[1] = g;
    clMask[2] = b;

    clMask[3] = a;
    }

void RenderState::getColorMask(bool& r, bool& g, bool& b, bool& a) const{
    r = clMask[0];
    g = clMask[1];
    b = clMask[2];

    a = clMask[3];
    }

void RenderState::setPolygonRenderMode( RenderState::PolyRenderMode::Type front,
                                        RenderState::PolyRenderMode::Type back ) {
    prenderMode[0] = front;
    prenderMode[1] =  back;
    }

void RenderState::setPolygonRenderMode( RenderState::PolyRenderMode::Type f ) {
    setPolygonRenderMode(f,f);
    }

RenderState::PolyRenderMode::Type RenderState::frontPolygonRenderMode() const {
    return prenderMode[0];
    }

RenderState::PolyRenderMode::Type RenderState::backPolygonRenderMode() const {
    return prenderMode[1];
    }

bool RenderState::isZTest()  const{
    return zTest;
    }

void RenderState::setZTest(bool use){
    zTest = use;
    }

bool RenderState::isAlphaCoverage()  const{
    return alphaCoverage;
    }

void RenderState::setAlphaCoverage(bool use){
    alphaCoverage = use;
    }

bool RenderState::isBlend()  const{
    return blend;
    }

void RenderState::setBlend(bool use){
    blend = use;
    }

RenderState::CullMode::Type RenderState::cullFaceMode() const{
    return cullMode;
    }

void RenderState::setCullFaceMode( RenderState::CullMode::Type use){
    cullMode = use;
    }

RenderState::AlphaBlendMode::Type RenderState::getBlendSFactor()  const{
    return sfactor;
    }

RenderState::AlphaBlendMode::Type RenderState::getBlendDFactor()  const{
    return dfactor;
    }

void RenderState::getBlendMode( RenderState::AlphaBlendMode::Type& out_sfactor,
                                RenderState::AlphaBlendMode::Type& out_dfactor)
                                 const{
    out_sfactor = sfactor;
    out_dfactor = dfactor;
    }

void RenderState::setBlendMode( RenderState::AlphaBlendMode::Type s,
                                RenderState::AlphaBlendMode::Type d ){
    sfactor = s;
    dfactor = d;
    }

RenderState::ZTestMode::Type RenderState::getZTestMode() const{
  return zTestType;
  }

void RenderState::setZTestMode(ZTestMode::Type use){
  zTestType = use;
  }

bool RenderState::operator ==(const RenderState &other) const {
  for( int i=0; i<4; ++i )
    if( other.clMask[i]!= clMask[i] )
      return false;

  return other.alphaCoverage == this->alphaCoverage &&
         other.atest         == this->atest &&
         other.atestType     == this->atestType &&
         other.blend         == this->blend &&
         other.cullMode      == this->cullMode &&
         other.dfactor       == this->dfactor &&
         other.sfactor       == this->sfactor &&
         other.zTest         == this->zTest   &&
         other.zTestType     == this->zTestType &&
         other.zWriting      == this->zWriting;

}

bool RenderState::operator !=(const RenderState &other) const {
  return !( *this==other );
  }
