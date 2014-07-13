#pragma once

namespace WinRt{
typedef int( *MainFunction )(int, const char**);
int startApplication( MainFunction func );
}