LOCAL_PATH := $(call my-dir)

include /Users/poly/workspace/NewType/jni/MarkerDetection/Android.mk

include $(CLEAR_VARS)

OPENCV_CAMERA_MODULES:=on
OPENCV_INSTALL_MODULES:=on
OPENCV_LIB_TYPE:=SHARED

include /Users/poly/workspace/OpenCV-2.4.6-android-sdk/sdk/native/jni/OpenCV.mk

LOCAL_SRC_FILES  := n3_marker_jni.cpp
LOCAL_C_INCLUDES += $(LOCAL_PATH) $(LOCAL_PATH)/MarkerDetection
LOCAL_CFLAGS    := -Werror -O3 -ffast-math
LOCAL_LDLIBS     += -llog -ldl
LOCAL_ARM_NEON := true
LOCAL_MODULE     := newtype
LOCAL_SHARED_LIBRARIES += n3_marker

include $(BUILD_SHARED_LIBRARY)
