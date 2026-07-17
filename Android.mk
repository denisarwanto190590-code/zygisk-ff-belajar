LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

# Nama modul disamakan persis dengan id di module.prop agar tidak error
LOCAL_MODULE := zygisk_ff_belajar

# Mendaftarkan file kode C++ utama
LOCAL_SRC_FILES := main.cpp

# Menambahkan pustaka sistem Android agar fungsi LOGI (logcat) bisa bekerja
LOCAL_LDLIBS := -llog -landroid

# PERBAIKAN: Mengaktifkan C++20 khusus untuk C++ agar zygisk.h tidak error makro
LOCAL_CPPFLAGS := -std=c++20

# Standar keamanan dan optimasi tambahan untuk compiler
LOCAL_CFLAGS := -Wall -O3

include $(BUILD_SHARED_LIBRARY)
