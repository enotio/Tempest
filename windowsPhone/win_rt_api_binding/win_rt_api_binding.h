#pragma once

namespace WinRt{
  typedef int( *MainFunction )(int, const char**);
  int startApplication( MainFunction func );

  void nextEvent();
  void nextEvents();
  void setMainWidget( void* w );
  }