
LOCAL_PATH := $(call my-dir)

$(info LOCAL_PATH: $(LOCAL_PATH))

CORE_DIR := $(LOCAL_PATH)/../..

$(info CORE_DIR: $(CORE_DIR))

include $(CORE_DIR)/platform/libretro/Makefile.common

$(info SOURCES_CXX: $(SOURCES_CXX))

COREFLAGS := -DANDROID -D__LIBRETRO__ -Wall -Wno-deprecated -ffunction-sections -std=c++17 -fno-rtti -fexceptions  $(INCFLAGS)


include $(CLEAR_VARS)

LOCAL_MODULE        := retro
LOCAL_SRC_FILES     := $(SOURCES_CXX)
LOCAL_C_INCLUDES    := $(MY_INCLUDES)
LOCAL_CXXFLAGS      := $(COREFLAGS)
LOCAL_LDFLAGS       := -Wl

include $(BUILD_SHARED_LIBRARY)
