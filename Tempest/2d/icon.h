#ifndef ICON_H
#define ICON_H

#include <Tempest/Pixmap>
#include <Tempest/Sprite>

namespace Tempest {

class Icon {
  public:
    Icon();
    Icon(const Pixmap& pm, Tempest::SpritesHolder& h);

    Sprite normal;
    Sprite disabled;
  };

}

#endif // ICON_H
