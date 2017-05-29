LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_C_INCLUDES += \
  $(LOCAL_PATH)\
  $(tempestPath)/include \

LOCAL_CPP_FEATURES := rtti exceptions
LOCAL_MODULE       := main

FILE_LIST          := $(wildcard $(LOCAL_PATH)/*.cpp)
LOCAL_SRC_FILES    := $(FILE_LIST:$(LOCAL_PATH)/%=%)

LOCAL_SHARED_LIBRARIES := Tempest

$(info path of $(LOCAL_MODULE)=$(LOCAL_PATH))
include $(BUILD_SHARED_LIBRARY)
