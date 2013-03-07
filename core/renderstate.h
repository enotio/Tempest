#ifndef RENDERSTATE_H
#define RENDERSTATE_H

namespace Tempest{

  //! Класс обертка на состоянием рендера.
  class RenderState {
    public:
      RenderState();
      virtual ~RenderState(){}

      enum Preset{
        PostProcess
        };

      RenderState( Preset p );

      struct AlphaTestMode{
        //! Режим альфа-теста.
        enum Type{
          Never,

          Greater,
          Less,

          GEqual,
          LEqual,

          NOEqual,
          Equal,
          Always,
          Count
          };
        };
      struct ZTestMode{
        //! Режим альфа-теста.
        enum Type{
          Never,

          Greater,
          Less,

          GEqual,
          LEqual,

          NOEqual,
          Equal,
          Always,
          Count
          };
        };

      //! Режим альфа смешивания.
      struct AlphaBlendMode{
        //! Режим альфа смешивания.
        enum Type{
          zero,                 //GL_ZERO,
          one,                  //GL_ONE,
          src_color,            //GL_SRC_COLOR,
          one_minus_src_color,  //GL_ONE_MINUS_SRC_COLOR,
          src_alpha,            //GL_SRC_ALPHA,
          one_minus_src_alpha,  //GL_ONE_MINUS_SRC_ALPHA,
          dst_alpha,            //GL_DST_ALPHA,
          one_minus_dst_alpha,  //GL_ONE_MINUS_DST_ALPHA,
          dst_color,            //GL_DST_COLOR,
          one_minus_dst_color,  //GL_ONE_MINUS_DST_COLOR,
          src_alpha_saturate,   //GL_SRC_ALPHA_SATURATE,
          Count
          };
        };

      struct CullMode{
        enum Type{
          noCull = 0,
          front,
          back
          };
        };

      double alphaTestRef() const;
      void  setAlphaTestRef(double v);

      void setAlphaTestMode( RenderState::AlphaTestMode::Type mode );
      AlphaTestMode::Type alphaTestMode() const ;

      bool isZWriting() const;
      void setZWriting(bool use);

      bool isZTest() const;
      void setZTest(bool use);

      ZTestMode::Type getZTestMode() const;
      void setZTestMode(ZTestMode::Type use);

      bool isBlend() const;
      void setBlend(bool use);

      CullMode::Type cullFaceMode() const;
      void setCullFaceMode(CullMode::Type use);

      bool isAlphaCoverage() const;
      void setAlphaCoverage(bool use);

      RenderState::AlphaBlendMode::Type getBlendSFactor() const;
      RenderState::AlphaBlendMode::Type getBlendDFactor() const;
      void getBlendMode( RenderState::AlphaBlendMode::Type& out_sfactor,
                         RenderState::AlphaBlendMode::Type& out_dfactor) const;
      void setBlendMode( RenderState::AlphaBlendMode::Type sfactor,
                         RenderState::AlphaBlendMode::Type dfactor );

      bool operator == ( const RenderState & other ) const;
      bool operator != ( const RenderState & other ) const;

      void setColorMask(bool  r, bool  g, bool  b, bool  a);
      void getColorMask(bool& r, bool& g, bool& b, bool& a) const;

  private:
      double atest;
      AlphaTestMode::Type atestType;
      ZTestMode::Type     zTestType;
      CullMode::Type      cullMode;

      bool zWriting, zTest, blend, alphaCoverage;
      AlphaBlendMode::Type sfactor, dfactor;

      bool clMask[4];

      void init();
    };

  }

#endif // RENDERSTATE_H
