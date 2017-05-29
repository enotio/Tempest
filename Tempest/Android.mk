LOCAL_PATH    := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE  := Tempest

$(info path of $(LOCAL_MODULE)=$(LOCAL_PATH))

LOCAL_C_INCLUDES += \
  $(LOCAL_PATH)\
  $(LOCAL_PATH)/include \
  $(LOCAL_PATH)/thirdparty \
  $(LOCAL_PATH)/thirdparty/freetype/include \
  $(LOCAL_PATH)/thirdparty/squish \
  $(LOCAL_PATH)/thirdparty/libjpeg

SOURCES := $(wildcard $(LOCAL_PATH)*.cpp)
SOURCES += $(wildcard $(LOCAL_PATH)/**/*.cpp)
SOURCES += $(wildcard $(LOCAL_PATH)/**/**/*.cpp)

SOURCES += $(wildcard $(LOCAL_PATH)/**/*.c)
SOURCES += $(wildcard $(LOCAL_PATH)/**/**/*.c)
SOURCES += $(wildcard $(LOCAL_PATH)/**/**/**/*.c)

SOURCES += \
  $(LOCAL_PATH)/thirdparty/freetype/src/autofit/autofit.c \
  $(LOCAL_PATH)/thirdparty/freetype/src/base/basepic.c \
  $(LOCAL_PATH)/thirdparty/freetype/src/base/ftapi.c \
  $(LOCAL_PATH)/thirdparty/freetype/src/base/ftbase.c \
  $(LOCAL_PATH)/thirdparty/freetype/src/base/ftbbox.c \
  $(LOCAL_PATH)/thirdparty/freetype/src/base/ftbitmap.c \
  $(LOCAL_PATH)/thirdparty/freetype/src/base/ftdbgmem.c \
  $(LOCAL_PATH)/thirdparty/freetype/src/base/ftdebug.c \
  $(LOCAL_PATH)/thirdparty/freetype/src/base/ftglyph.c \
  $(LOCAL_PATH)/thirdparty/freetype/src/base/ftinit.c \
  $(LOCAL_PATH)/thirdparty/freetype/src/base/ftpic.c \
  $(LOCAL_PATH)/thirdparty/freetype/src/base/ftstroke.c \
  $(LOCAL_PATH)/thirdparty/freetype/src/base/ftsynth.c \
  $(LOCAL_PATH)/thirdparty/freetype/src/base/ftsystem.c \
  $(LOCAL_PATH)/thirdparty/freetype/src/cff/cff.c \
  $(LOCAL_PATH)/thirdparty/freetype/src/pshinter/pshinter.c \
  $(LOCAL_PATH)/thirdparty/freetype/src/psnames/psnames.c \
  $(LOCAL_PATH)/thirdparty/freetype/src/raster/raster.c \
  $(LOCAL_PATH)/thirdparty/freetype/src/sfnt/sfnt.c \
  $(LOCAL_PATH)/thirdparty/freetype/src/smooth/smooth.c \
  $(LOCAL_PATH)/thirdparty/freetype/src/truetype/truetype.c

LOCAL_SRC_FILES := $(SOURCES:$(LOCAL_PATH)/%=%)
#LOCAL_SRC_FILES    += $(patsubst $(LOCAL_PATH)/%, %, $(wildcard $(LOCAL_PATH)/**/*.cpp))
#LOCAL_SRC_FILES    += $(patsubst $(LOCAL_PATH)/%, %, $(wildcard $(LOCAL_PATH)/core/**/*.cpp))

LOCAL_CPP_FEATURES := rtti exceptions
LOCAL_CPPFLAGS     := "-DFT2_BUILD_LIBRARY=1"
LOCAL_CFLAGS       := $(LOCAL_CPPFLAGS)
LOCAL_LDLIBS       := -llog -landroid -lEGL -lGLESv2

include $(BUILD_SHARED_LIBRARY)
