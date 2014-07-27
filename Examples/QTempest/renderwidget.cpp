#include "renderwidget.h"
#include "ui_renderwidget.h"

#include <QMouseEvent>

static RenderWidget::Vertex quadVb[] = {
  { 1,-1,-1,  1,1},
  {-1,-1,-1,  0,1},
  {-1, 1,-1,  0,0},
  { 1, 1,-1,  1,0},

  {-1,-1, 1,  1,1},
  { 1,-1, 1,  0,1},
  { 1, 1, 1,  0,0},
  {-1, 1, 1,  1,0},

  {-1,  1,-1, 1,0},
  {-1, -1,-1, 1,1},
  {-1, -1, 1, 0,1},
  {-1,  1, 1, 0,0},

  { 1, -1,-1,  0,1},
  { 1,  1,-1,  0,0},
  { 1,  1, 1,  1,0},
  { 1, -1, 1,  1,1},

  {-1, -1,-1,  0,0},
  { 1, -1,-1,  1,0},
  { 1, -1, 1,  1,1},
  {-1, -1, 1,  0,1},

  { 1,  1,-1,  1,1},
  {-1,  1,-1,  0,1},
  {-1,  1, 1,  0,0},
  { 1,  1, 1,  1,0},
  };

static uint16_t quadId[] = {
   0, 1, 2, 0, 2, 3,
   4, 5, 6, 4, 6, 7,
   8, 9,10, 8,10,11,
  12,13,14,12,14,15,
  16,17,18,16,18,19,
  20,21,22,20,22,23
  };

using namespace Tempest;

RenderWidget::RenderWidget( Tempest::AbstractAPI & api, QWidget *parent ) :
  QTempestWidget(api, parent),
  ui(new Ui::RenderWidget),
  texHolder(device),
  vboHolder(device),
  iboHolder(device),
  shHolder (device)
{
  ui->setupUi(this);

  zoom = 1;

  vbo = vboHolder.load(quadVb, sizeof(quadVb)/sizeof(quadVb[0]));
  ibo = iboHolder.load(quadId, sizeof(quadId)/sizeof(quadId[0]));

  VertexDeclaration::Declarator decl;
  decl.add( Decl::float3, Usage::Position )
      .add( Decl::float2, Usage::TexCoord );
  vdecl = VertexDeclaration(device, decl);

  udecl.add("mvpMatrix",Decl::Matrix4x4)
       .add("texture",  Decl::Texture2d);

  texture = texHolder.load("data/texture.png");

  shader = shHolder.load({"shader/basic.vs.glsl","shader/basic.fs.glsl","","",""});

  T_ASSERT( shader.isValid() );
  }

RenderWidget::~RenderWidget() {
  delete ui;
  }

void RenderWidget::mousePressEvent(QMouseEvent *e) {
  mpos = e->pos();
  setMouseTracking(1);
  }

void RenderWidget::mouseMoveEvent(QMouseEvent *e) {
  rotate += (e->pos() - mpos);
  mpos = e->pos();
  update();
  }

void RenderWidget::mouseReleaseEvent(QMouseEvent *) {
  setMouseTracking(0);
  }

void RenderWidget::wheelEvent( QWheelEvent *e ) {
  if( e->delta()>0 )
    zoom *= 1.1; else
    zoom /= 1.1;

  update();
  }

void RenderWidget::paint3d() {
  device.clear( Color(0,0,1), 1 );

  device.beginPaint();
  setupShaderConstants(shader);
  device.drawIndexed( AbstractAPI::Triangle,
                      shader, vdecl,
                      vbo, ibo,
                      0,0, ibo.size()/3 );
  device.endPaint();
  }

void RenderWidget::setupShaderConstants( ShaderProgram &sh ) {
  Matrix4x4 mvpMatrix, projective, view;

  projective.perspective( 45.0, (float)width()/height(), 0.1, 100.0 );

  view.translate(0,0,4);
  view.rotate(rotate.y(), 1, 0, 0);
  view.rotate(rotate.x(), 0, 1, 0);
  view.scale(zoom);

  mvpMatrix = projective;
  mvpMatrix.mul(view);

  ubo.mvpMatrix = mvpMatrix;
  ubo.texture   = texture;
  sh.setUniform(ubo,udecl,0);
  }
