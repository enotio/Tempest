LOCAL_PATH := $(call my-dir)
JNI_PATH   := $(LOCAL_PATH)

include $(JNI_PATH)/tempest/Android.mk
include $(JNI_PATH)/main/Android.mk

