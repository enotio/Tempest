#ifndef RENDERWIDGET_H
#define RENDERWIDGET_H

#include "bind/qtempestwidget.h"

#include <Tempest/TextureHolder>
#include <Tempest/VertexBufferHolder>
#include <Tempest/IndexBufferHolder>

#include <Tempest/VertexBuffer>
#include <Tempest/IndexBuffer>
#include <Tempest/Texture2d>

namespace Ui {
class RenderWidget;
}

class RenderWidget : public QTempestWidget {
  Q_OBJECT
  public:
    explicit RenderWidget( Tempest::AbstractAPI & api, QWidget *parent = 0);
    ~RenderWidget();

    struct Vertex {
      float x,y,z;
      float u,v;
      };
  private:
    Ui::RenderWidget *ui;

    Tempest::TextureHolder        texHolder;
    Tempest::VertexBufferHolder   vboHolder;
    Tempest::IndexBufferHolder    iboHolder;

    Tempest::ShaderProgramHolder  shHolder;

    Tempest::VertexBuffer<Vertex>  vbo;
    Tempest::IndexBuffer<uint16_t> ibo;
    Tempest::VertexDeclaration     vdecl;
    Tempest::Texture2d             texture;

    Tempest::ShaderProgram         shader;
    struct UBO{
      Tempest::Matrix4x4 mvpMatrix;
      Tempest::Texture2d texture;
      } ubo;
    Tempest::UniformDeclaration udecl;

    QPoint rotate, mpos;
    float  zoom;

    void mousePressEvent(QMouseEvent*);
    void mouseMoveEvent(QMouseEvent*);
    void mouseReleaseEvent(QMouseEvent*);
    void wheelEvent( QWheelEvent* );

    void paint3d();
    void setupShaderConstants( Tempest::ShaderProgram & sh );
  };

#endif // RENDERWIDGET_H
