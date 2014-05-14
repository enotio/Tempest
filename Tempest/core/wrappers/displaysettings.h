#ifndef DISPLAYSETTINGS_H
#define DISPLAYSETTINGS_H

namespace Tempest{

class Size;

//! screen display settings
class DisplaySettings {
  public:
    DisplaySettings( int w, int h, int bits = 32, bool fullScreen = false );
    DisplaySettings( const Size& wh, int bits = 32, bool fullScreen = false );

    int width;
    int height;

    int  bits;
    bool fullScreen;

    Size size() const;
    void setSize( const Size& s );
  };

}

#endif // DISPLAYSETTINGS_H
