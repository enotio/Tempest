#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <Tempest/Window>
#include <Tempest/Device>

#include <Tempest/TextureHolder>
#include <Tempest/VertexBufferHolder>
#include <Tempest/IndexBufferHolder>
#include <Tempest/LocalVertexBufferHolder>
#include <Tempest/LocalTexturesHolder>

#include <Tempest/VertexBuffer>
#include <Tempest/IndexBuffer>
#include <Tempest/Texture2d>
#include <Tempest/ShaderProgram>

#include <Tempest/SurfaceRender>
#include <Tempest/SpritesHolder>

class MainWindow:public Tempest::Window {
  public:
    MainWindow( Tempest::AbstractAPI& api );

    struct Vertex {
      float x,y,z;
      float u,v;
      };
  protected:
    Tempest::Device device;
    Tempest::TextureHolder        texHolder;
    Tempest::LocalTexturesHolder  ltexHolder;

    Tempest::VertexBufferHolder   vboHolder;
    Tempest::IndexBufferHolder    iboHolder;

    Tempest::LocalVertexBufferHolder lvboHolder;
    Tempest::LocalIndexBufferHolder  liboHolder;

    Tempest::VertexBuffer<Vertex>  vbo;
    Tempest::IndexBuffer<uint16_t> ibo;
    Tempest::VertexDeclaration     vdecl;
    Tempest::Texture2d             texture;
    Tempest::Texture2d             renderTarget[2];

    Tempest::ShaderProgramHolder   shHolder;
    Tempest::ShaderProgram         shader;
    Tempest::UniformDeclaration udecl;
    struct UBO{
      Tempest::Matrix4x4 mvpMatrix;
      Tempest::Texture2d texture;
      } ubo;

    Tempest::SpritesHolder         spHolder;
    Tempest::SurfaceRender         uiRender;

    Tempest::Point rotate, mpos;
    float zoom;

    void paintEvent( Tempest::PaintEvent& e );

    void mouseDownEvent ( Tempest::MouseEvent& e );
    void mouseDragEvent ( Tempest::MouseEvent& e );
    void mouseWheelEvent( Tempest::MouseEvent& e );

    void mrt(Tempest::Texture2d *a);
    void render();
    void resizeEvent( Tempest::SizeEvent& e );

    void setupShaderConstants(Tempest::ShaderProgram & sh , const Tempest::Size &rtSize);
  };

#endif // MAINWINDOW_H
