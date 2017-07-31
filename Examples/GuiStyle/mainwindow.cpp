#include "customstyle.h"
#include "mainwindow.h"

#include <Tempest/Assert>
#include <Tempest/RenderState>
#include <Tempest/Painter>

#include <Tempest/Application>
#include <Tempest/Layout>
#include <Tempest/Button>
#include <Tempest/LineEdit>
#include <Tempest/Panel>
#include <Tempest/ListView>
#include <Tempest/ListBox>
#include <Tempest/ScrollWidget>
#include <Tempest/CheckBox>
#include <Tempest/Label>

using namespace Tempest;

MainWindow::MainWindow(Tempest::AbstractAPI &api)
   :device( api, handle() ),
    texHolder(device),
    vboHolder(device),
    iboHolder(device),
    shHolder (device),
    spHolder (texHolder),
    uiRender (shHolder),
    listBoxDelegate(listBoxItems),
    listViewDelegate(listViewItems) {
  Application::showHint.bind(*this,&MainWindow::setHint);
  Application::setMainFont(Font("data/arial",16));

  for(int i=0; i<5; ++i )
    listBoxItems.emplace_back("item "+std::to_string(i));

  for(int i=0; i<16; ++i )
    listViewItems.push_back(1.0/(i+1.0));

  setupUi();
  }

void MainWindow::setupUi() {
  Widget* a=createPage();
  a->setStyle(std::make_shared<CustomStyle>(
                Color(74/255.f,20/255.f,140/255.f),
                Color(48/255.f,49/255.f,254/255.f)));
  layout().add(a);

  Widget* b=createPage();
  b->setPosition(300,0);
  b->setStyle(std::make_shared<Tempest::Style>());
  layout().add(b);
  }

Widget *MainWindow::createPage() {
  Panel *panel = new Panel();
  panel->setDragable(true);
  panel->setLayout(Vertical);

  Button* button = new Button();
  button->setText("Button");
  button->setHint("button hint");
  button->setIcon(Icon("data/icon.png",spHolder));
  button->onClicked.bind(this,&MainWindow::buttonClick);
  panel->layout().add(button);

  button = new CheckBox();
  button->setText("Checkbox");
  button->setHint("checkbox hint");
  panel->layout().add(button);

  Label* label = new Label();
  label->setText("Label");
  panel->layout().add(label);

  LineEdit* edit = new LineEdit();
  edit->setText("LineEdit");
  edit->setHint("some edit");
  panel->layout().add(edit);

  ListBox* listBox = new ListBox();
  listBox->setDelegate(listBoxDelegate);
  panel->layout().add(listBox);

  scroll = new ScrollWidget();

  ListView* list = new ListView();
  list->setDelegate(listViewDelegate);
  scroll->centralWidget().layout().add(list);
  panel->layout().add(scroll);

  return panel;
  }

void MainWindow::paintEvent(PaintEvent &e) {
  Painter p(e);

  p.setFont( Application::mainFont() );
  p.drawText(100, 80, hint);

  paintNested(e);
  }

void MainWindow::buttonClick() {
  scroll->setEnabled(!scroll->isEnabledTo(scroll));
  }

void MainWindow::render() {
  if( !device.startRender() )
    return;

  if( needToUpdate() )
    uiRender.buildWindowVbo(*this, vboHolder, iboHolder, spHolder);

  device.clear( Color(225/255.0f), 1 );

  device.beginPaint();
  device.draw( uiRender );
  device.endPaint();

  device.present();
  }

void MainWindow::resizeEvent( SizeEvent & ) {
  device.reset();
  }

void MainWindow::setHint(const std::u16string &h, const Rect &) {
  hint = h;
  update();
  }
