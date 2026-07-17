LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

# Mengubah nama output biner menjadi arm64-v8a
LOCAL_MODULE := arm64-v8a
# Menghilangkan awalan teks 'lib' otomatis dari NDK compiler
LOCAL_MODULE_FILENAME := arm64-v8a

LOCAL_SRC_FILES := main.cpp

# PERBAIKAN: Ditambahkan -lGLESv2 agar fungsi grafis glLineWidth dari imgui_overlay.h dikenali
LOCAL_LDLIBS := -llog -landroid -lGLESv2

# WAJIB MENGGUNAKAN CPPFLAGS DAN C++20 UNTUK MENGHILANGKAN ERROR MAKRO ZYGISK
LOCAL_CPPFLAGS := -std=c++20
LOCAL_CFLAGS := -Wall -O3

include $(BUILD_SHARED_LIBRARY)
