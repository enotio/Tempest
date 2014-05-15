#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <Tempest/Window>
#include <Tempest/Device>

#include <Tempest/TextureHolder>
#include <Tempest/VertexBufferHolder>
#include <Tempest/IndexBufferHolder>
#include <Tempest/VertexShaderHolder>
#include <Tempest/FragmentShaderHolder>

#include <Tempest/VertexBuffer>
#include <Tempest/IndexBuffer>
#include <Tempest/Texture2d>
#include <Tempest/VertexShader>
#include <Tempest/FragmentShader>
#include <Tempest/ProgramObject>

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
    Tempest::VertexBufferHolder   vboHolder;
    Tempest::IndexBufferHolder    iboHolder;

    Tempest::VertexShaderHolder   vsHolder;
    Tempest::FragmentShaderHolder fsHolder;

    Tempest::VertexBuffer<Vertex>  vbo;
    Tempest::IndexBuffer<uint16_t> ibo;
    Tempest::VertexDeclaration     vdecl;
    Tempest::Texture2d             texture;

    Tempest::ProgramObject         shader;

    Tempest::SpritesHolder         spHolder;
    Tempest::SurfaceRender         uiRender;

    Tempest::Point rotate, mpos;
    float zoom;

    void paintEvent( Tempest::PaintEvent& e );

    void mouseDownEvent ( Tempest::MouseEvent& e );
    void mouseDragEvent ( Tempest::MouseEvent& e );
    void mouseWheelEvent( Tempest::MouseEvent& e );

    void render();
    void resizeEvent( Tempest::SizeEvent& e );

    void setupShaderConstants( Tempest::ProgramObject & sh );
  };

#endif // MAINWINDOW_H
