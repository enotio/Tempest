#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "renderwidget.h"

MainWindow::MainWindow( Tempest::AbstractAPI &api, QWidget *parent ) :
  QMainWindow(parent),
  ui(new Ui::MainWindow) {
  ui->setupUi(this);
  connect( ui->close, SIGNAL(clicked()),
           this, SLOT(close()) );

  ui->layMain->addWidget( new RenderWidget(api) );
  }

MainWindow::~MainWindow() {
  delete ui;
  }
