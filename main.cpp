#include <jni.h>

// ============================================================================
// TRIK KHUSUS: Mengabaikan macro bawaan zygisk.h yang memicu error visibility
// ============================================================================
#define REGISTER_ZYGISK_MODULE(clazz) \
    extern "C" [[gnu::visibility("default")]] zygisk::ModuleBase *zygisk_module_entry(zygisk::Api *api, JNIEnv *env) { \
        return new clazz(); \
    }

#include "zygisk.h" 
#include <android/log.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/mman.h>
#include <string.h>
#include <stdio.h>  // Ditambahkan untuk fungsi fopen, fgets, fclose

// Menghubungkan ke file kuas gambar yang barusan kita buat
#include "imgui_overlay.h"

#define LOG_TAG "Zygisk_ESP_FF"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

struct Vector2 { float x, y; };
struct Vector3 { float x, y, z; };

// =========================================================
// DAFTAR OFFSET SEBELUMNYA
// =========================================================
uintptr_t il2cpp_base = 0;

#define OFFSET_PLAYER_MANAGER     0x558     // UGCCustomPlayerManager
#define OFFSET_PLAYER_ENTITY      0x68      // m_LocalPlayerEntity
#define OFFSET_EDIT_OBJ           0x10      // m_EditObj
#define OFFSET_TRANSFORM          0x30      // UnityEngine.Transform
#define OFFSET_WORLD_TO_SCREEN    0x80BFB2C // UGCAPIWorldToScreenPoint

// Membuat objek kanvas gambar global agar bisa diakses di dalam loop ESP
ESPCanvas kanvasESP;

// Definisikan struktur fungsi WorldToScreen agar bisa dieksekusi secara native
typedef bool (*_WorldToScreenPoint)(Vector3 position, Vector2* out_screen);
_WorldToScreenPoint WorldToScreenPoint_Orig = nullptr;

// =========================================================
// FUNGSI MENCARI BASE ADDRESS LIBIL2CPP.SO
// =========================================================
uintptr_t DapatkanBaseAddress(const char* nama_library) {
    uintptr_t address = 0;
    char jalur[256];
    char baris[512];
    
    FILE* f = fopen("/proc/self/maps", "r");
    if (!f) return 0;

    while (fgets(baris, sizeof(baris), f)) {
        if (strstr(baris, nama_library)) {
            // Mengambil alamat memori awal dari baris teks di maps
            sscanf(baris, "%lx-%*x", &address);
            break;
        }
    }
    fclose(f);
    return address;
}

// =========================================================
// LOGIKA UTAMA ESP LINE VISUAL
// =========================================================
void* ThreadLoopEsp(void*) {
    LOGI("Thread ESP Berhasil Aktif di Latar Belakang!");

    // Tunggu sampai game benar-benar memuat libil2cpp.so ke memori
    while (il2cpp_base == 0) {
        il2cpp_base = DapatkanBaseAddress("libil2cpp.so");
        usleep(500000); // Cek setiap 0.5 detik
    }
    
    LOGI("Sistem Berhasil Menemukan Alamat libil2cpp.so: 0x%lx", il2cpp_base);

    // Set ukuran layar HP standar
    int lebarLayar = 2340;
    int tinggiLayar = 1080;
    kanvasESP.InisialisasiKuas(lebarLayar, tinggiLayar);

    // Titik awal garis dipasang di bagian tengah bawah layar HP kamu
    Vector2 titikAwalGaris;
    titkAwalGaris.x = (float)lebarLayar / 2.0f;
    titkAwalGaris.y = (float)tinggiLayar;

    while (true) {
        // Deklarasikan fungsi WorldToScreen menggunakan offset
        if (WorldToScreenPoint_Orig == nullptr) {
            WorldToScreenPoint_Orig = (_WorldToScreenPoint)(il2cpp_base + OFFSET_WORLD_TO_SCREEN);
        }

        // Alamat GameManager
        uintptr_t gameManager = *reinterpret_cast<uintptr_t*>(il2cpp_base + 0x4ABCEF0); 
        if (!gameManager) { usleep(100000); continue; }

        // Masuk ke daftar pemain
        uintptr_t playerManager = *reinterpret_cast<uintptr_t*>(gameManager + OFFSET_PLAYER_MANAGER);
        if (!playerManager) { usleep(100000); continue; }

        int totalPemain = *reinterpret_cast<int*>(playerManager + 0x18); 
        uintptr_t playerList = *reinterpret_cast<uintptr_t*>(playerManager + 0x10);
        if (!playerList) { usleep(100000); continue; }

        // LOOPING: Cek satu per satu semua pemain di dalam game
        for (int i = 0; i < totalPemain; i++) {
            uintptr_t dataMusuh = *reinterpret_cast<uintptr_t*>(playerList + (i * 0x8));
            if (!dataMusuh) continue;

            // Rantaian pointer ke komponen Transform musuh
            uintptr_t playerEntity = *reinterpret_cast<uintptr_t*>(dataMusuh + OFFSET_PLAYER_ENTITY);
            if (!playerEntity) continue;
            uintptr_t editObj      = *reinterpret_cast<uintptr_t*>(playerEntity + OFFSET_EDIT_OBJ);
            if (!editObj) continue;
            uintptr_t transform    = *reinterpret_cast<uintptr_t*>(editObj + OFFSET_TRANSFORM);
            if (!transform) continue;

            // Ambil posisi koordinat 3D musuh di map game
            Vector3 koordinat3D_Musuh = *reinterpret_cast<Vector3*>(transform + 0x90); 

            // KONVERSI: Mengubah posisi 3D musuh menjadi koordinat 2D layar
            Vector2 titikAkhirGaris;
            bool isMunculDiLayar = false;
            
            if (WorldToScreenPoint_Orig != nullptr) {
                isMunculDiLayar = WorldToScreenPoint_Orig(koordinat3D_Musuh, &titikAkhirGaris);
            }

            // EKSEKUSI VISUAL: Jika musuh terlihat oleh kamera game, gambar garis merahnya!
            if (isMunculDiLayar) {
                kanvasESP.GambarGarisMerah(
                    titkAwalGaris.x, titikAwalGaris.y, 
                    titikAkhirGaris.x, titikAkhirGaris.y, 
                    2.0f 
                );
            }
        }

        usleep(16000); // Batasi proses agar stabil di kisaran 60 FPS
    }
    return nullptr;
}

// =========================================================
// INISIALISASI MODULE ZYGISK
// =========================================================
class BelajarZygiskModule : public zygisk::ModuleBase {
public:
    void onLoad(zygisk::Api* api, JNIEnv* env) override {
        this->api = api;
        this->env = env;
    }

    void preAppSpecialize(zygisk::AppSpecializeArgs* args) override {
        const char* process_name = env->GetStringUTFChars(args->process, nullptr);
        
        if (process_name && strcmp(process_name, "com.dts.freefireth") == 0) {
            LOGI("Game Free Fire Terdeteksi! Menyiapkan Sistem ESP Visual...");
            
            // Membuat thread loop ESP secara terpisah
            pthread_t esp_thread;
            pthread_create(&esp_thread, nullptr, ThreadLoopEsp, nullptr);
        }
        env->ReleaseStringUTFChars(args->process, process_name);
    }

private:
    zygisk::Api* api;
    JNIEnv* env;
};

// Mengeksekusi macro kustom yang sudah kita paksa di baris paling atas (tanpa titik koma)
REGISTER_ZYGISK_MODULE(BelajarZygiskModule)
