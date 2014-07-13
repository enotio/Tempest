#include <Tempest/WindowsPhone>

extern int Tempest_main(int,const char**);

[Platform::MTAThread]
int WinMain() {
  return Tempest::WinPhoneAPI::preMain( Tempest_main );
  }
