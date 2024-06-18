LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

TENSORFLOW_CFLAGS	  := \
  -fstack-protector-strong \
  -fpic \
  -ffunction-sections \
  -funwind-tables \
  -no-canonical-prefixes \
  -fno-canonical-system-headers \
  '-march=armv7-a' \
  '-mfpu=vfpv3-d16' \
  '-mfloat-abi=softfp' \
  -DHAVE_PTHREAD \
  -Wall \
  -Wwrite-strings \
  -Woverloaded-virtual \
  -Wno-sign-compare \
  '-Wno-error=unused-function' \
  '-std=c++11' \
  -fno-exceptions \
  -DEIGEN_AVOID_STL_ARRAY \
  '-mfpu=neon' \
  '-std=c++11' \
  '-DMIN_LOG_LEVEL=0' \
  -DTF_LEAN_BINARY \
  -O2 \
  -Os \
  -frtti \
  -MD \

#-MF \

TENSORFLOW_SRC_FILES := \
	main.cpp \
	../../source_files/FilterManagementLibrary/TensorflowIntegration/TensorflowNNInstance.cpp \
	../../source_files/FilterManagementLibrary/TensorflowIntegration/TensorflowNNInstanceClassifier.cpp \
	../../source_files/FilterManagementLibrary/Utilities.cpp \
	../../source_files/FilterManagementLibrary/Logger.cpp \

LOCAL_MODULE    := native_lib
LOCAL_ARM_MODE  := arm
LOCAL_SRC_FILES := $(TENSORFLOW_SRC_FILES)
LOCAL_CFLAGS    := $(TENSORFLOW_CFLAGS)

LOCAL_LDLIBS    := \
	-Wl,-whole-archive \
	$(LOCAL_PATH)/libs/$(TARGET_ARCH_ABI)/libtensorflow-core.a \
	$(LOCAL_PATH)/libs/$(TARGET_ARCH_ABI)/libprotobuf.a \
	$(LOCAL_PATH)/libs/$(TARGET_ARCH_ABI)/libnsync.a \
	libhexagon_controller.so \
	-Wl,-no-whole-archive \
	$(NDK_ROOT)/sources/cxx-stl/gnu-libstdc++/4.9/libs/$(TARGET_ARCH_ABI)/libgnustl_static.a \
	$(NDK_ROOT)/sources/cxx-stl/gnu-libstdc++/4.9/libs/$(TARGET_ARCH_ABI)/libsupc++.a \
	-landroid \
	-ljnigraphics \
	-llog \
	-lm \
	-z defs \
	-s \
	'-Wl,--icf=all' \
	-Wl,--exclude-libs,ALL \
	-lz \
	-static-libgcc \
	-no-canonical-prefixes \
	'-march=armv7-a' \
	-Wl,--fix-cortex-a8 \
	-Wl,-S \

LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../dependencies/include/tensorflow \
	$(LOCAL_PATH)/../../dependencies/include/tensorflow/tensorflow/contrib/makefile/gen/host_obj \
	$(LOCAL_PATH)/../../dependencies/include/tensorflow/tensorflow/contrib/makefile/downloads \
	$(LOCAL_PATH)/../../dependencies/include/tensorflow/tensorflow/contrib/makefile/downloads/eigen \
	$(LOCAL_PATH)/../../dependencies/include/tensorflow/tensorflow/contrib/makefile/downloads/gemmlowp \
	$(LOCAL_PATH)/../../dependencies/include/tensorflow/tensorflow/contrib/makefile/downloads/nsync/public \
	$(LOCAL_PATH)/../../dependencies/include/tensorflow/tensorflow/contrib/makefile/gen/proto \
	$(LOCAL_PATH)/../../dependencies/include/tensorflow/tensorflow/contrib/makefile/gen/proto_text \
	$(LOCAL_PATH)/../../dependencies/include/tensorflow/tensorflow/contrib/makefile/gen/protobuf-host/include \
	$(LOCAL_PATH)/../../header_files \

NDK_MODULE_PATH := $(call my-dir)

include $(BUILD_SHARED_LIBRARY)

