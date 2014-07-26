#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <Tempest/Window>
#include <Tempest/Device>

#include <Tempest/TextureHolder>
#include <Tempest/VertexBufferHolder>
#include <Tempest/IndexBufferHolder>
#include <Tempest/VolumeHolder>

#include <Tempest/VertexBuffer>
#include <Tempest/IndexBuffer>
#include <Tempest/Texture2d>
#include <Tempest/Texture3d>
#include <Tempest/ShaderProgram>

class MainWindow:public Tempest::Window {
  public:
    MainWindow( Tempest::AbstractAPI& api );

    struct Vertex {
      float x,y,z;
      float u,v;
      };
  protected:
    Tempest::Device device;
    Tempest::VolumeHolder         volHolder;
    Tempest::TextureHolder        texHolder;
    Tempest::VertexBufferHolder   vboHolder;
    Tempest::IndexBufferHolder    iboHolder;

    Tempest::VertexBuffer<Vertex>  vbo;
    Tempest::IndexBuffer<uint16_t> ibo;
    Tempest::VertexDeclaration     vdecl;
    Tempest::Texture3d             volume;
    Tempest::Texture2d             grad;

    Tempest::ShaderProgramHolder   shHolder;
    Tempest::ShaderProgram         shader;
    struct UBO{
      Tempest::Matrix4x4 mvpMatrix;
      Tempest::Matrix4x4 mvpMatrixInv;
      Tempest::Texture3d volume;
      Tempest::Texture2d grad;
      } ubo;
    Tempest::UniformDeclaration udecl;

    Tempest::Point rotate, mpos;
    Tempest::Matrix4x4     view;
    float zoom;
    float modeSize[3];

    void loadData();
    void mouseDownEvent ( Tempest::MouseEvent& e );
    void mouseDragEvent ( Tempest::MouseEvent& e );
    void mouseWheelEvent( Tempest::MouseEvent& e );

    void render();
    void resizeEvent( Tempest::SizeEvent& e );

    void setupShaderConstants( Tempest::ShaderProgram & sh );
  };

#endif // MAINWINDOW_H
