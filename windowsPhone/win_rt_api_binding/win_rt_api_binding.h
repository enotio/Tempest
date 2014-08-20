#pragma once

#include <string>

namespace WinRt{
  typedef int( *MainFunction )(int, const char**);
  int startApplication( MainFunction func );
  
  std::wstring getAssetsFolder();

  bool  nextEvent();
  bool  nextEvents();
  void  setMainWidget( void* w );
  void* getMainRtWidget();
  void  getScreenRect( void* hwnd, int& scrW, int& scrH );
  int   orientation();
  }