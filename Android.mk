LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := Tempest
LOCAL_CFLAGS := -std=gnu++11

SDL_PATH     := C:/Users/Try/Home/Programming/android/android-project/jni/SDL
Tempest_PATH := C:/Users/Try/Home/Programming/Tempest/Tempest

LOCAL_C_INCLUDES := $(SDL_PATH)/include \
                    $(Tempest_PATH)/include\
                    $(Tempest_PATH)/math

LOCAL_CFLAGS := -std=gnu++11
LOCAL_SRC_FILES := \
  $(subst $(LOCAL_PATH)/,,\
  $(wildcard $(LOCAL_PATH)/core/wrappers/*.cpp) \
  $(wildcard $(LOCAL_PATH)/core/*.cpp) \
  $(wildcard $(LOCAL_PATH)/dataControl/*.cpp) \
  $(wildcard $(LOCAL_PATH)/math/*.cpp) \
  $(wildcard $(LOCAL_PATH)/ogl/*.cpp) \
  $(wildcard $(LOCAL_PATH)/render/*.cpp) \
  $(wildcard $(LOCAL_PATH)/scene/*.cpp) \
  $(wildcard $(LOCAL_PATH)/shading/*.cpp) \
  $(wildcard $(LOCAL_PATH)/system/*.cpp) \
  $(wildcard $(LOCAL_PATH)/utils/*.cpp) \
  $(wildcard $(LOCAL_PATH)/*.cpp) )


LOCAL_SHARED_LIBRARIES := SDL2

LOCAL_LDLIBS    := -llog -landroid -lEGL -lGLESv1_CM -lGLESv2 -ljnigraphics

include $(BUILD_SHARED_LIBRARY)
