#ifndef DEVICESM5_H
#define DEVICESM5_H

#include <Tempest/Device>
#include <Tempest/TessShader>
#include <Tempest/EvalShader>

namespace Tempest{

class DeviceSM5 : public Device {
  public:
    DeviceSM5( const AbstractAPI & dx,
               void * windowHwnd );

    DeviceSM5( const AbstractAPI & dx,
               const Options & opt,
               void * windowHwnd );


    template< class T, class I >
    void drawIndexed( const Tempest::VertexShader   &vs,
                      const Tempest::FragmentShader &fs,
                      const Tempest::TessShader     &ts,
                      const Tempest::EvalShader     &es,
                      const Tempest::VertexDeclaration &decl,
                      const Tempest::VertexBuffer<T> & vbo,
                      const Tempest::IndexBuffer<I>  & ibo,
                      int vboOffsetIndex,
                      int iboOffsetIndex,
                      int pCount ){
      assertPaint();

      if( pCount==0 ||
          !decl.isValid() ||
          !vs.isValid() ||
          !fs.isValid() )
        return;

      applyRs();
      bindShaders(vs, fs);
      bindShaders(ts, es);
      implDrawIndexed( AbstractAPI::PrimitiveType(0),
                       decl, vbo, ibo, vboOffsetIndex, iboOffsetIndex, pCount);
      }

  protected:
    void deleteShader( AbstractAPI::TessShader* ) const;
    void deleteShader( AbstractAPI::EvalShader* ) const;

    void bind( const Tempest::TessShader &ts );
    void bind( const Tempest::EvalShader &es );

    using Device::bindShaders;
    void bindShaders( const Tempest::TessShader &ts,
                      const Tempest::EvalShader &es );
  };

}

#endif // DEVICESM5_H
