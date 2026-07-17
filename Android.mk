LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

# Nama modul disamakan persis dengan id di module.prop agar tidak error
LOCAL_MODULE := zygisk_ff_belajar
LOCAL_SRC_FILES := main.cpp

# PERBAIKAN: Ditambahkan -lGLESv2 agar fungsi grafis glLineWidth dari imgui_overlay.h dikenali
LOCAL_LDLIBS := -llog -landroid -lGLESv2

# WAJIB MENGGUNAKAN CPPFLAGS DAN C++20 UNTUK MENGHILANGKAN ERROR MAKRO ZYGISK
LOCAL_CPPFLAGS := -std=c++20
LOCAL_CFLAGS := -Wall -O3

include $(BUILD_SHARED_LIBRARY)
