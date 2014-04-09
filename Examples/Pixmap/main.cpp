#include <Tempest/Pixmap>
#include <Tempest/File>
#include <iostream>

const char* x[] = {
  "grass1.png",
  "grass2.png",
  "grass3.png",
  "grass4.png",
  "grass5.png",
  "grass6.png",
  "grass7.png",
  "sand.png",
  "snow.png",
  "snow2.png",
  "snow3.png",
  "ice.png",
  "snow_grass.png",
  "rock_snow.png",
  "rock.png",
  "rock1.png",
  "cliff.png",
  "temple.png",
  "ground.png",
  "ground_rock.png",
  "brick0.jpg",
  "brick1.png",
  "brick2.jpg",
  "lava.png",

  "grass1n.png",
  "grass2n.png",
  "grass3n.png",
  "grass4n.png",
  "grass5n.png",
  "grass6n.png",
  "grass7n.png",
  "sandn.png",
  "snown.png",
  "snow2n.png",
  "snow3n.png",
  "icen.png",
  "snow_grassn.png",
  "rock_snown.png",
  "rockn.png",
  "rock1n.png",
  "cliffn.png",
  "templen.png",
  "groundn.png",
  "ground_rockn.png",
  "brick0n.png",
  "brick1n.png",
  "brick2n.png",
  "lavan.png",
  "lava_glow.png",
  0
};

int main() {
  Tempest::Pixmap p;
  p.load("data/test.jpeg");
  p.save("data/test.jpeg.png");
  return 0;

  p.load("data/rocks.png");
  p.setFormat( Tempest::Pixmap::Format_ETC1_RGB8 );
  p.save("data/test.ktx");
  p.setFormat( Tempest::Pixmap::Format_RGB );
  p.save("data/test_uncompressed.png");

  p.load("data/rgb-mipmap-reference.ktx");
  p.setFormat( Tempest::Pixmap::Format_RGB );
  p.save("data/rgb-mipmap-reference.png");

  return 0;

  /*
  const std::string diri = "C:/Users/Try/Home/Programming/game/game-build-desktop/data/textures/land_original/";
  const std::string diro = "C:/Users/Try/Home/Programming/game/game-build-desktop/data/textures/land/";
  for( int i=0; x[i]; ++i ){
    Tempest::Pixmap p;
    p.load( diri + x[i] );
    p.setFormat( Tempest::Pixmap::Format_ETC1_RGB8 );
    p.save( diro + x[i]+".ktx" );
    std::cout << diro << x[i] << ".ktx" << std::endl;
    }

  return 0;
  */
  }

