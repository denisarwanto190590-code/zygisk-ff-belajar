LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

# Nama modul disamakan persis dengan id di module.prop agar tidak error
LOCAL_MODULE := zygisk_ff_belajar

# Mendaftarkan file kode C++ utama
LOCAL_SRC_FILES := main.cpp

# Menambahkan pustaka sistem Android agar fungsi LOGI (logcat) bisa bekerja
LOCAL_LDLIBS := -llog -landroid

# Standar keamanan tambahan agar kode berjalan optimal di sistem 64-bit
LOCAL_CFLAGS := -std=c++17 -Wall -O3

include $(BUILD_SHARED_LIBRARY)
