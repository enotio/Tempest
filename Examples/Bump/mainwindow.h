#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <Tempest/Window>
#include <Tempest/Device>

#include <Tempest/VertexBufferHolder>
#include <Tempest/IndexBufferHolder>

#include <Tempest/VertexBuffer>
#include <Tempest/IndexBuffer>

#include <cstdint>

class MainWindow : public Tempest::Window {
  public:
    MainWindow( Tempest::AbstractAPI & api );

  protected:
    void render();
    void resizeEvent(Tempest::SizeEvent &e);

  private:
    void setShaderConstants( float sx, float sy,
                             const Tempest::Texture2d &tex,
                             const Tempest::Texture2d &normal,
                             const Tempest::Texture2d &height );

    void mouseDownEvent(Tempest::MouseEvent &e);
    void mouseDragEvent(Tempest::MouseEvent &e);

    Tempest::Point mpos, spin;

    Tempest::Device device;

    Tempest::TextureHolder texHolder;
    Tempest::Texture2d     texture, normal, height;

    Tempest::VertexBufferHolder  vboHolder;
    Tempest::IndexBufferHolder   iboHolder;

    Tempest::ShaderProgramHolder shHolder;
    Tempest::ShaderProgram       shader;

    struct Vertex {
      float    x, y, z;
      float    tu, tv;
      float    normal [3];
      float    bnormal[3];
      };

    Tempest::VertexBuffer<Vertex>   vbo;
    Tempest::IndexBuffer <uint16_t> ibo;
    Tempest::VertexDeclaration    vdecl;

    Tempest::UniformDeclaration udecl;
    struct UBO{
      UBO(){
        lightDir[0] = 0;
        lightDir[1] = 0;
        lightDir[2] = -1;
        }
      Tempest::Matrix4x4 modelView,mvpMatrix;
      float lightDir[3];
      Tempest::Texture2d texture, bump, heightMap;
      } ubo;
    Tempest::Matrix4x4 mProj;

    static Tempest::Device::Options options();

    static Vertex   quadVertices[];
    static uint16_t quadIndexes[];
  };

#endif // MAINWINDOW_H
