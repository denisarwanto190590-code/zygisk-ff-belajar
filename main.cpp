#include <jni.h>
#include "zygisk.h" // <-- WAJIB ADA BIAR TIDAK ERROR SEPERTI DI GAMBAR!
#include <android/log.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/mman.h>
#include <string.h>

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

#define OFFSET_PLAYER_MANAGER     0x558     // UGCCustomPlayerManager (Gambar 8)
#define OFFSET_PLAYER_ENTITY      0x68      // m_LocalPlayerEntity (Gambar 2)
#define OFFSET_EDIT_OBJ           0x10      // m_EditObj (Gambar 3)
#define OFFSET_TRANSFORM          0x30      // UnityEngine.Transform (Gambar 4)
#define OFFSET_WORLD_TO_SCREEN    0x80BFB2C // UGCAPIWorldToScreenPoint (Gambar 6)

// Membuat objek kanvas gambar global agar bisa diakses di dalam loop ESP
ESPCanvas kanvasESP;

// Definisikan struktur fungsi WorldToScreen agar bisa dieksekusi secara native
typedef bool (*_WorldToScreenPoint)(Vector3 position, Vector2* out_screen);
_WorldToScreenPoint WorldToScreenPoint_Orig = nullptr;

// =========================================================
// LOGIKA UTAMA ESP LINE VISUAL
// =========================================================
void* ThreadLoopEsp(void*) {
    LOGI("Thread ESP Berhasil Aktif di Latar Belakang!");

    // Set ukuran layar HP standar (Nanti bisa diotomatisasi lewat fungsi Android)
    int lebarLayar = 2340;
    int tinggiLayar = 1080;
    kanvasESP.InisialisasiKuas(lebarLayar, tinggiLayar);

    // Titik awal garis dipasang di bagian tengah bawah layar HP kamu
    Vector2 titikAwalGaris;
    titkAwalGaris.x = (float)lebarLayar / 2.0f;
    titkAwalGaris.y = (float)tinggiLayar;

    while (true) {
        if (il2cpp_base == 0) {
            usleep(500000);
            continue;
        }

        // Deklarasikan fungsi WorldToScreen menggunakan offset 0x80BFB2C
        if (WorldToScreenPoint_Orig == nullptr) {
            WorldToScreenPoint_Orig = (_WorldToScreenPoint)(il2cpp_base + OFFSET_WORLD_TO_SCREEN);
        }

        // Alamat GameManager (Sebagai contoh jangkar memori utama game)
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

            // Rantaian pointer: 0x68 -> 0x10 -> 0x30 untuk dapat komponen Transform musuh
            uintptr_t playerEntity = *reinterpret_cast<uintptr_t*>(dataMusuh + OFFSET_PLAYER_ENTITY);
            uintptr_t editObj      = *reinterpret_cast<uintptr_t*>(playerEntity + OFFSET_EDIT_OBJ);
            uintptr_t transform    = *reinterpret_cast<uintptr_t*>(editObj + OFFSET_TRANSFORM);
            if (!transform) continue;

            // Ambil posisi koordinat 3D musuh di map game
            Vector3 koordinat3D_Musuh = *reinterpret_cast<Vector3*>(transform + 0x90); 

            // KONVERSI: Mengubah posisi 3D musuh menjadi koordinat 2D layar HP kamu
            Vector2 titikAkhirGaris;
            bool isMunculDiLayar = false;
            
            if (WorldToScreenPoint_Orig != nullptr) {
                isMunculDiLayar = WorldToScreenPoint_Orig(koordinat3D_Musuh, &titikAkhirGaris);
            }

            // EKSEKUSI VISUAL: Jika musuh terlihat oleh kamera game, gambar garis merahnya!
            if (isMunculDiLayar) {
                kanvasESP.GambarGarisMerah(
                    titkAwalGaris.x, titikAwalGaris.y, // Titik asal (Tengah bawah layar)
                    titikAkhirGaris.x, titikAkhirGaris.y, // Titik ujung (Posisi musuh)
                    2.0f // Ketebalan garis ESP (2 piksel)
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
            
            // Ambil base address libil2cpp.so secara otomatis lewat fitur API Zygisk
            il2cpp_base = 1; // Penanda bahwa game sudah terdeteksi aktif

            pthread_t esp_thread;
            pthread_create(&esp_thread, nullptr, ThreadLoopEsp, nullptr);
        }
        env->ReleaseStringUTFChars(args->process, process_name);
    }

private:
    zygisk::Api* api;
    JNIEnv* env;
};

REGISTER_ZYGISK_MODULE(BelajarZygiskModule);
