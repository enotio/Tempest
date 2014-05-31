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

  protected:
    Tempest::Device               device;
    Tempest::TextureHolder        texHolder;
    Tempest::VertexBufferHolder   vboHolder;
    Tempest::IndexBufferHolder    iboHolder;

    Tempest::VertexShaderHolder   vsHolder;
    Tempest::FragmentShaderHolder fsHolder;

    Tempest::Texture2d             texture;

    Tempest::SpritesHolder         spHolder;
    Tempest::SurfaceRender         uiRender;

    void paintEvent( Tempest::PaintEvent& e );

    void render();
    void resizeEvent( Tempest::SizeEvent& e );
  };

#endif // MAINWINDOW_H
