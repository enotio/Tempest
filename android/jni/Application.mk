APP_OPTIM             := debug
APP_PLATFORM          := android-10
APP_STL               := gnustl_shared
APP_CPPFLAGS          += -frtti
APP_CPPFLAGS          += -fexceptions
APP_ABI               := armeabi armeabi-v7a
#NDK_TOOLCHAIN         := gcc
NDK_TOOLCHAIN_VERSION := 4.8
LOCAL_CPPFLAGS        += -std=c++11

APP_MODULES  := game network Tempest
