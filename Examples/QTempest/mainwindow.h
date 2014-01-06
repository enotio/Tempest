#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <Tempest/Opengl2x>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit MainWindow( Tempest::AbstractAPI& api, QWidget *parent = 0);
  ~MainWindow();

private:
  Ui::MainWindow *ui;

};

#endif // MAINWINDOW_H
