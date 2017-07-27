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
#include <Tempest/ShaderProgramHolder>
#include <Tempest/ShaderProgram>

#include <Tempest/SurfaceRender>
#include <Tempest/SpritesHolder>

#include <Tempest/ListDelegate>
#include <Tempest/ScrollWidget>

class MainWindow:public Tempest::Window {
  public:
    MainWindow( Tempest::AbstractAPI& api );

  protected:
    Tempest::Device               device;
    Tempest::TextureHolder        texHolder;
    Tempest::VertexBufferHolder   vboHolder;
    Tempest::IndexBufferHolder    iboHolder;

    Tempest::ShaderProgramHolder  shHolder;

    Tempest::SpritesHolder        spHolder;
    Tempest::SurfaceRender        uiRender;

    Tempest::ScrollWidget*        scroll;

    void setupUi();
    Widget* createPage();
    void paintEvent( Tempest::PaintEvent& e );

    void buttonClick();

    void render();
    void resizeEvent( Tempest::SizeEvent& e );

    void setHint(const std::u16string& hint, const Tempest::Rect& r);
    std::u16string hint;

    // list items
    std::vector<std::string>                listBoxItems;
    Tempest::ArrayListDelegate<std::string> listBoxDelegate;

    std::vector<double>                listViewItems;
    Tempest::ArrayListDelegate<double> listViewDelegate;
  };

#endif // MAINWINDOW_H
