#include "qtempestwidget.h"

#include <QEvent>

using namespace Tempest;

QTempestWidget::Flagsetup::Flagsetup(QWidget *w) {
  w->setAttribute(Qt::WA_NativeWindow);
  w->setAttribute(Qt::WA_PaintOnScreen);
  w->setAttribute(Qt::WA_NoSystemBackground);
  }

bool QTempestWidget::PaintEngine::begin(QPaintDevice *pdev){
  if( pdev==0 )
    return 0;

  impl = &((QTempestWidget*)pdev)->impl;
  setActive(true);
  return 1;
  }

bool QTempestWidget::PaintEngine::end(){
  bool ok = isActive();
  setActive(false);
  return ok;
  }

void QTempestWidget::PaintEngine::updateState( const QPaintEngineState& st ){
  (void)st;
  }

void QTempestWidget::PaintEngine::drawPixmap( const QRectF &r,
                                              const QPixmap &pm,
                                              const QRectF &sr ){
  (void)r;
  (void)pm;
  (void)sr;
  }

void QTempestWidget::PaintEngine::drawPolygon( const QPoint *points,
                                               int pointCount,
                                               QPaintEngine::PolygonDrawMode mode ) {
  (void)points;
  (void)pointCount;
  (void)mode;
  //impl->uiRender;
  }

QPaintEngine::Type QTempestWidget::PaintEngine::type() const{
  return (QPaintEngine::Type)QTempestWidget::TempestPaintEngine;
  }

QTempestWidget::QTempestWidget(Tempest::AbstractAPI &api, QWidget *parent)
  :QTempestWidgetBase(parent), flg(this), device(api, (void*)winId()), impl(device) {
  }

bool QTempestWidget::event( QEvent *e ) {
  if( e->type()==QEvent::Paint ){
    if( !device.startRender() )
      return false;

    paint3d();
    paintEvent( (QPaintEvent*)e );

    device.present();

    return true;
    }

  if( e->type()==QEvent::Resize ){
    resetDevice();
    return QWidget::event(e);
    }

  return QWidget::event(e);
  }

void QTempestWidget::paintEvent(QPaintEvent *) {
  QPainter p(this);
  //p.drawLine(0,0, 100, 100);

  }

void QTempestWidget::paint3d() {
  device.clear( Color(0,0,0) );
  }

void QTempestWidget::resetDevice() {
  device.reset();
  }

QPaintEngine *QTempestWidget::paintEngine() const {
  return &pengine;
  }

QTempestWidget::Impl::Impl(Device &device)
  : texHolder(device),
    vboHolder(device),
    iboHolder(device),
    vsHolder (device),
    fsHolder (device),
    spHolder (texHolder),
    uiRender ( vsHolder, fsHolder ) {

  }
