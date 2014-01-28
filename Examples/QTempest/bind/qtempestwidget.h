#ifndef QTEMPESTWIDGET_H
#define QTEMPESTWIDGET_H

#include <QWidget>
#include <QPaintEngine>

#include <Tempest/Device>

#include <Tempest/TextureHolder>
#include <Tempest/VertexBufferHolder>
#include <Tempest/IndexBufferHolder>
#include <Tempest/VertexShaderHolder>
#include <Tempest/FragmentShaderHolder>

#include <Tempest/VertexBuffer>
#include <Tempest/IndexBuffer>
#include <Tempest/VertexShader>
#include <Tempest/FragmentShader>
#include <Tempest/ProgramObject>

#include <Tempest/SurfaceRender>
#include <Tempest/SpritesHolder>

#ifdef __ANDROID__
#include <QtOpenGL/QGLWidget>
typedef QGLWidget QTempestWidgetBase;
#else
typedef QWidget QTempestWidgetBase;
#endif

class QTempestWidget :public QTempestWidgetBase {
  Q_OBJECT
  public:
    explicit QTempestWidget( Tempest::AbstractAPI & api, QWidget * parent = 0 );

    enum PaintDeviceType{
      TempestPaintEngine = QPaintEngine::User+1
      };

  private:
    struct Flagsetup{
      Flagsetup( QWidget* );
      } flg;

    struct Impl{
      Impl( Tempest::Device& dev );
      Tempest::TextureHolder        texHolder;
      Tempest::VertexBufferHolder   vboHolder;
      Tempest::IndexBufferHolder    iboHolder;

      Tempest::VertexShaderHolder   vsHolder;
      Tempest::FragmentShaderHolder fsHolder;

      Tempest::SpritesHolder         spHolder;
      Tempest::SurfaceRender         uiRender;
      };

  protected:
    Tempest::Device device;

    bool event(QEvent *);
    void paintEvent(QPaintEvent*);

    virtual void paint3d();
    virtual void resetDevice();

    class PaintEngine: public QPaintEngine {
      public:
        bool begin(QPaintDevice *pdev);
        bool  end();
        void updateState( const QPaintEngineState& st );

        void drawPixmap( const QRectF &r, const QPixmap &pm, const QRectF &sr );
        void drawPolygon(const QPoint *points, int pointCount, PolygonDrawMode mode);
        Type type() const;

        Impl * impl;
      };

    QPaintEngine *paintEngine() const;
    mutable PaintEngine   pengine;

  private:
    Impl impl;
  };

#endif // QTEMPESTWIDGET_H
