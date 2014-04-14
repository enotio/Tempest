#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <Tempest/Window>
#include <Tempest/Device>

#include <Tempest/VertexBufferHolder>
#include <Tempest/IndexBufferHolder>

#include <Tempest/VertexBuffer>
#include <Tempest/IndexBuffer>

#include <Tempest/LocalTexturesHolder>

#include <Tempest/ProgramObject>

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
    void loadShader( Tempest::ProgramObject & prog, const std::string &file);
    void loadShader( Tempest::ProgramObject & prog,
                     const std::string &file,
                     const std::initializer_list<const char*>& def );

    Tempest::Texture2d mkBuffer( Tempest::Size sz );
    Tempest::Texture2d mkBuffer( int w, int h );

    void setShadowConstants(Tempest::ProgramObject &shader);
    void setShaderConstants(SceneObject &obj, Tempest::ProgramObject &shader);
    void getLightDir(float d[] , const Tempest::Matrix4x4 &m);

    void mainPasss(const Shadow &sm);
    Tempest::Texture2d distorsion( const Tempest::Texture2d& scene,
                                   Tempest::Texture2d &depth,
                                   Tempest::ProgramObject &shader );
    Tempest::Texture2d blur( const Tempest::Texture2d & in );
    Tempest::Texture2d brightPass( const Tempest::Texture2d &tex );
    Tempest::Texture2d resizeTex( const Tempest::Texture2d &tex,
                                  const Tempest::Size & nsz );
    void renderShadow( Tempest::ProgramObject &shader,
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

    Tempest::VertexBufferHolder   vboHolder;
    Tempest::IndexBufferHolder    iboHolder;

    Tempest::VertexShaderHolder   vsHolder;
    Tempest::FragmentShaderHolder fsHolder;

    Tempest::ProgramObject shadow, shadow_refract;
    Tempest::ProgramObject shader, gbuf, mono, refract, emission, gemission;
    Tempest::ProgramObject blt, gauss, bright, combine;

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
                       Tempest::ProgramObject &shader,
                       const Shadow &sm,
                       bool sh = false );
    void updateCamera();
    Tempest::Matrix4x4 shadowMatrix();

    std::string shaderSource( const std::string& f, const std::initializer_list<const char*>& def );
    std::string shaderSource( Tempest::RFile& f, const std::initializer_list<const char*>& def );
    float zoom;
  };

#endif // MAINWINDOW_H
