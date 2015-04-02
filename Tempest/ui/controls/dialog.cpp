#include "dialog.h"

#include <Tempest/SystemAPI>
#include <Tempest/Window>
#include <Tempest/Layout>

#include <Tempest/Application>

using namespace Tempest;

struct Dialog::LayShadow : Tempest::Layout {
  void applyLayout(){
    for(Widget* w:widgets()){
      Point pos = w->pos();

      if(w->x()+w->w()>owner()->w())
        pos.x = owner()->w()-w->w();
      if(w->y()+w->h()>owner()->h())
        pos.y = owner()->h()-w->h();
      if(pos.x<=0)
        pos.x = 0;
      if(pos.y<=0)
        pos.y = 0;

      w->setPosition(pos);
      }
    }
  };

struct Dialog::Overlay : public Tempest::WindowOverlay {
  bool isModal = true;
  Dialog * dlg = nullptr;

  void mouseDownEvent(MouseEvent& e){
    e.accept();
    }

  void mouseWheelEvent(MouseEvent& e){
    e.accept();
    }

  void paintEvent(PaintEvent& e){
    dlg->paintShadow(e);
    paintNested(e);
    }

  void keyDownEvent(Tempest::KeyEvent& e){
    dlg->keyDownEvent(e);
    }

  void keyUpEvent(Tempest::KeyEvent& e){
    dlg->keyUpEvent(e);
    }
  };

Dialog::Dialog() : owner_ov(nullptr) {
  resize(300,200);
  setDragable(1);
  }

Dialog::~Dialog() {
  close();
  }

int Dialog::exec() {
  if(!owner_ov){
    owner_ov = new Overlay();
    owner_ov->dlg = this;

    T_ASSERT( SystemAPI::instance().addOverlay( owner_ov )!=0 );
    owner_ov->setLayout( new LayShadow() );
    owner_ov->layout().add( this );
    }

  setPosition( (owner_ov->w()-w())/2,
               (owner_ov->h()-h())/2 );
  while( owner_ov && !Application::isQuit() ) {
    Application::processEvents();
    }
  return 0;
  }

void Dialog::close() {
  if( owner_ov ){
    Tempest::WindowOverlay* ov = owner_ov;
    owner_ov = 0;

    setVisible(0);
    CloseEvent e;
    this->closeEvent(e);
    ov->layout().take(this);
    ov->deleteLater();
    }
  }

void Dialog::setModal(bool m) {
  if(owner_ov)
    owner_ov->isModal = m;
  }

bool Dialog::isModal() const {
  return owner_ov ? owner_ov->isModal : false;
  }

void Dialog::closeEvent( CloseEvent & e ) {
  if(!owner_ov)
    e.ignore(); else
    close();
  }

void Dialog::keyDownEvent(KeyEvent &e) {
  if(e.key==KeyEvent::K_ESCAPE)
    close();
  }

void Dialog::paintShadow(PaintEvent &e) {
  if(!owner_ov)
    return;

  Painter p(e);
  p.setBlendMode(alphaBlend);
  p.setColor(0,0,0,0.5);
  p.drawRect(0,0,owner_ov->w(),owner_ov->h());
  }
