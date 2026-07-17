SKIPUNZIP=0

# Memberitahu Magisk bahwa ini adalah modul berbasis Zygisk
ZYGISK_MOD=1

ui_print "- [INFO] Memulai Pemasangan Modul ESP..."

# Membuat folder rahasia internal Magisk yang diwajibkan oleh Magisk v26
mkdir -p "$MODPATH/zygisk/arm64-v8a"

# Mencari file biner arm64-v8a.so di dalam ZIP
if [ -f "$ZIPFILE" ]; then
    ui_print "- [INFO] Menyetel arsitektur biner 64-bit..."
    
    # Ekstrak file arm64-v8a.so ke folder tujuan
    unzip -j -o "$ZIPFILE" "arm64-v8a.so" -d "$MODPATH/zygisk/arm64-v8a"
    
    # TRIK UTAMA: Mengubah nama file secara otomatis di sistem Android menjadi zygisk.so
    mv "$MODPATH/zygisk/arm64-v8a/arm64-v8a.so" "$MODPATH/zygisk/arm64-v8a/zygisk.so"
    
    ui_print "- [SUKSES] Konfigurasi biner Zygisk selesai!"
else
    ui_print "- [ERROR] File arm64-v8a.so tidak ditemukan di dalam ZIP!"
    abort "- Pemasangan dibatalkan!"
fi

ui_print "- [INFO] Sinkronisasi modul berhasil diselesaikan!"
