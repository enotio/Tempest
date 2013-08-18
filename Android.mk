LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := Tempest

Tempest_PATH := C:/Users/Try/Home/Programming/Tempest/Tempest

LOCAL_C_INCLUDES := $(Tempest_PATH)/include\
                    $(Tempest_PATH)/math\
                    $(Tempest_PATH)/squish\
                    $(Tempest_PATH)/thirdparty

LOCAL_EXPORT_C_INCLUDES := $(Tempest_PATH)/include

LOCAL_CFLAGS    := -std=c++0x
LOCAL_CFLAGS    += -D_GLIBCXX_USE_C99_MATH=1
LOCAL_CPPFLAGS  := -D__STDC_INT64__ -Dsigset_t="unsigned int"

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
  $(wildcard $(LOCAL_PATH)/squish/*.cpp) \
  $(wildcard $(LOCAL_PATH)/system/*.cpp) \
  $(wildcard $(LOCAL_PATH)/utils/*.cpp) \
  $(wildcard $(LOCAL_PATH)/ui/*.cpp) \
  $(wildcard $(LOCAL_PATH)/thirdparty/nv_math/*.cpp) \
  $(wildcard $(LOCAL_PATH)/thirdparty/libpng/*.c) \
  $(wildcard $(LOCAL_PATH)/*.cpp) )

LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)

LOCAL_STATIC_LIBRARIES := android_native_app_glue
#LOCAL_STATIC_LIBRARIES := nv_and_util
#LOCAL_SHARED_LIBRARIES := native_subclass
LOCAL_LDLIBS    := -llog -landroid -lEGL -lGLESv1_CM -lGLESv2 -ljnigraphics -lz

include $(BUILD_SHARED_LIBRARY)
$(call import-module,android/native_app_glue)

