#include <Tempest/Application>
#include <Tempest/Pixmap>
#include <Tempest/File>

int main() {
  Tempest::Pixmap p;

  p.load("data/rocks.png");
  p.setFormat( Tempest::Pixmap::Format_ETC1_RGB8 );
  p.save("data/rocks.ktx");
  p.setFormat( Tempest::Pixmap::Format_RGB );
  p.save("data/rocks_test_uncompressed.png");

  p.load("data/rgb-mipmap-reference.ktx");
  p.setFormat( Tempest::Pixmap::Format_RGB );
  p.save("data/rgb-mipmap-reference.png");

  return 0;
  }

