#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <Tempest/Window>
#include <Tempest/Device>

#include <Tempest/VertexBufferHolder>
#include <Tempest/IndexBufferHolder>

#include <Tempest/VertexBuffer>
#include <Tempest/IndexBuffer>

#include <Tempest/LocalTexturesHolder>

#include <Tempest/ShaderProgram>

#include <cstdint>

#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <Tempest/AbstractScene>
#include <Tempest/AbstractGraphicObject>
#include <Tempest/GraphicObject>

struct Material{
  Tempest::Texture2d diffuse, normal, height;
  Tempest::Color     color;

  enum Shader{
    Main,
    MonoColor,
    Transparent,
    Emission
    } shaderType = Main;
  };
typedef Tempest::AbstractGraphicObject<Material>      AbstractGraphicObject;
typedef Tempest::GraphicObject<Material>              SceneObject;
typedef Tempest::AbstractScene<AbstractGraphicObject> Scene;

class MainWindow : public Tempest::Window {
  public:
    MainWindow( Tempest::AbstractAPI & api );
    ~MainWindow();

  protected:
    void render();
    void resizeEvent(Tempest::SizeEvent &e);

  private:
    struct Shadow{
      Tempest::Texture2d sm, rsm[3], depth;
      };
    void loadShader( Tempest::ShaderProgram & prog, const std::string &file);
    void loadShader( Tempest::ShaderProgram & prog,
                     const std::string &file,
                     const std::initializer_list<const char*>& def );

    Tempest::Texture2d mkBuffer( Tempest::Size sz );
    Tempest::Texture2d mkBuffer( int w, int h );

    void setShaderConstants(SceneObject &obj, Tempest::ShaderProgram &shader);
    void getLightDir(float d[] , const Tempest::Matrix4x4 &m);

    void mainPasss(const Shadow &sm);
    Tempest::Texture2d distorsion( const Tempest::Texture2d& scene,
                                   Tempest::Texture2d &depth,
                                   Tempest::ShaderProgram &shader );
    Tempest::Texture2d blur( const Tempest::Texture2d & in );
    Tempest::Texture2d brightPass( const Tempest::Texture2d &tex );
    Tempest::Texture2d resizeTex( const Tempest::Texture2d &tex,
                                  const Tempest::Size & nsz );
    void renderShadow( Tempest::ShaderProgram &shader,
                       Tempest::Texture2d &sh,
                       Tempest::Texture2d &depth,
                       bool inv, int smSz);
    void renderShadow(Tempest::Texture2d *sh, int cnt,
                       Tempest::Texture2d &depth, int smSz);
    void combinePass( const Tempest::Texture2d &tex,
                      const Tempest::Texture2d *bloom );

    void mouseDownEvent(Tempest::MouseEvent &e);
    void mouseDragEvent(Tempest::MouseEvent &e);

    void mouseWheelEvent( Tempest::MouseEvent &e );

    template< class F >
    bool loadScene( const char* f, const F& func );

    template< class F, class Vertex >
    void loadSceneImpl( const F& func,
                        std::vector<Vertex>   &vert,
                        std::vector<uint16_t> &index,
                        const aiScene * sc,
                        const aiNode  * nd,
                        const Tempest::Matrix4x4 &transform );

    Tempest::Point mpos, spin;

    Tempest::Device device;

    Tempest::TextureHolder       texHolder;
    Tempest::LocalTexturesHolder ltexHolder;
    Tempest::Texture2d           texture;

    Tempest::VertexBufferHolder  vboHolder;
    Tempest::IndexBufferHolder   iboHolder;

    Tempest::ShaderProgramHolder shHolder;

    Tempest::ShaderProgram shadow, shadow_refract;
    Tempest::ShaderProgram shader, gbuf, mono, refract, emission, gemission;
    Tempest::ShaderProgram blt, gauss, bright, combine;

    struct UboBlt{
      Tempest::Texture2d texture;
      } uboBlt;
    Tempest::UniformDeclaration ublt;

    struct UboGauss{
      Tempest::Texture2d image;
      float              dir[2] = {0,0};
      } uboGauss;
    Tempest::UniformDeclaration ugauss;

    struct UboBright{
      Tempest::Texture2d image;
      } uboBright;
    Tempest::UniformDeclaration ubright;

    struct UboCombine{
      Tempest::Texture2d image;
      Tempest::Texture2d bloom[4];
      } uboCombine;
    Tempest::UniformDeclaration ucombine;

    struct UboLighting{
      float lightDir[3];
      Tempest::Texture2d shadow;
      Tempest::Matrix4x4 invMvp;
      Tempest::Texture2d rsmColor;
      Tempest::Texture2d rsmNormal;
      } uboLighting;
    Tempest::UniformDeclaration ulighting;

    struct UboMaterial{
      Tempest::Matrix4x4 shadowMatrix;
      Tempest::Matrix4x4 modelView;
      Tempest::Matrix4x4 mvpMatrix;
      Tempest::Matrix4x4 projMatrix;
      Tempest::Texture2d diffuse;
      Tempest::Texture2d screen;
      Tempest::Color     matColor;
      } uboMaterial;
    Tempest::UniformDeclaration umaterial;

    struct Vertex {
      float    x, y, z;
      float    tu, tv;
      float    normal[3];
      };

    Vertex mkVertex(const aiMesh *face , size_t id, const Tempest::Matrix4x4 &m);
    Tempest::VertexDeclaration    vdecl;

    Scene scene;
    std::vector<SceneObject> objects;
    void setupLigting(Scene &scene,
                       Tempest::ShaderProgram &shader,
                       const Shadow &sm,
                       bool sh = false );
    void updateCamera();
    Tempest::Matrix4x4 shadowMatrix();

    std::string shaderSource( const std::string& f, const std::initializer_list<const char*>& def );
    std::string shaderSource( Tempest::RFile& f, const std::initializer_list<const char*>& def );
    float zoom;
  };

#endif // MAINWINDOW_H
