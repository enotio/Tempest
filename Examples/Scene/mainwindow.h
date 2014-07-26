#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <Tempest/Window>
#include <Tempest/Device>

#include <Tempest/TextureHolder>
#include <Tempest/VertexBufferHolder>
#include <Tempest/IndexBufferHolder>

#include <Tempest/VertexBuffer>
#include <Tempest/IndexBuffer>
#include <Tempest/Texture2d>
#include <Tempest/ShaderProgram>

#include <Tempest/AbstractScene>
#include <Tempest/AbstractGraphicObject>
#include <Tempest/GraphicObject>
#include <Tempest/Camera>

struct  Material{
  Tempest::Texture2d texture;
  };
typedef Tempest::AbstractGraphicObject<Material>    AbstractSceneObject;
typedef Tempest::GraphicObject<Material>            SceneObject;
typedef Tempest::AbstractScene<AbstractSceneObject> Scene;

class MainWindow:public Tempest::Window {
  public:
    MainWindow( Tempest::AbstractAPI& api );

    struct Vertex {
      float x,y,z;
      float u,v;
      };
  protected:
    Tempest::Device device;
    Tempest::TextureHolder       texHolder;
    Tempest::VertexBufferHolder  vboHolder;
    Tempest::IndexBufferHolder   iboHolder;

    Tempest::ShaderProgramHolder shHolder;
    Tempest::ShaderProgram       shader;

    Tempest::Point mpos;

    void mouseDownEvent ( Tempest::MouseEvent& e );
    void mouseDragEvent ( Tempest::MouseEvent& e );
    void mouseWheelEvent( Tempest::MouseEvent& e );

    void render();
    void resizeEvent( Tempest::SizeEvent& e );

    void setupShaderConstants(const SceneObject &obj,
                               Tempest::ShaderProgram & sh );

    struct UBO{
      Tempest::Matrix4x4 mvpMatrix;
      Tempest::Texture2d texture;
      } ubo;
    Tempest::UniformDeclaration udecl;

    std::vector<SceneObject> objects;
    Scene           scene;
    Tempest::Camera camera;
    void updateCamera();
  };

#endif // MAINWINDOW_H
