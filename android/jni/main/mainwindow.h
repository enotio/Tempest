#pragma once

#include <Tempest/Window>

#include <Tempest/TextureHolder>
#include <Tempest/VertexBufferHolder>
#include <Tempest/IndexBufferHolder>

#include <Tempest/VertexBuffer>
#include <Tempest/IndexBuffer>
#include <Tempest/Texture2d>
#include <Tempest/ShaderProgramHolder>
#include <Tempest/ShaderProgram>

#include <Tempest/SurfaceRender>
#include <Tempest/SpritesHolder>

class MainWindow : public Tempest::Window {
  public:
    MainWindow(Tempest::AbstractAPI& api);

  protected:
    Tempest::Device               device;
    Tempest::TextureHolder        texHolder;
    Tempest::VertexBufferHolder   vboHolder;
    Tempest::IndexBufferHolder    iboHolder;

    Tempest::ShaderProgramHolder  shHolder;

    Tempest::Texture2d             texture;

    Tempest::SpritesHolder         spHolder;
    Tempest::SurfaceRender         uiRender;

    void paintEvent( Tempest::PaintEvent& e );

    void render();
    void resizeEvent( Tempest::SizeEvent& e );
  };



