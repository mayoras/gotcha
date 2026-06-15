LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := gotcha
LOCAL_SRC_FILES := gotcha.cpp
LOCAL_LDLIBS := -llog -lstdc++
include $(BUILD_SHARED_LIBRARY)
