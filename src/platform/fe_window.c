// src/platform/fe_window.c

#include "platform/fe_window.h"
#include "utils/fe_logger.h"
#include <raylib.h> // Raylib kütüphanesini dahil et
#include <string.h> // strcmp/memset için

// Raylib sadece OpenGL'i destekler. İleride diğer API'ler için bu dosya
// başka bir soyutlama katmanına yönlendirilmelidir.
// Şimdilik sadece OpenGL'i desteklediğini varsayıyoruz.

/**
 * @brief Dahili olarak Raylib'in Configuration Flags'lerini ayarlar.
 */
static void fe_window_set_raylib_config(const fe_window_config_t* config) {
    // Raylib varsayılan olarak OpenGL kullanır. Diğer API'ler için Raylib uygun bir seçenek değildir,
    // ancak temel pencereyi başlatmak için kullanılır.

    if (config->fullscreen) {
        // Tam ekran için bayrak ayarla (Raylib pencereyi tam ekran açar)
        SetWindowState(FLAG_FULLSCREEN_MODE);
    }

    // Ekran boyutunu ayarlama (fullscreen modunda çözünürlüğü korumak için gerekli olabilir)
    SetWindowSize(config->width, config->height);
    
    // FPS kontrolünü ayarlama
    if (config->target_fps > 0) {
        SetTargetFPS(config->target_fps);
    }
    
    FE_LOG_INFO("Hedeflenen Grafik API: %s", 
        config->desired_api == FE_API_OPENGL_43 ? "OpenGL 4.3 (Raylib)" : "Diger API (Tanimlanacak)");
}


/**
 * Uygulama: fe_window_init
 */
fe_error_code_t fe_window_init(const fe_window_config_t* config) {
    if (!config) {
        FE_LOG_ERROR("Gecersiz pencere yapilandirmasi.");
        return FE_ERR_INVALID_ARGUMENT;
    }
    
    // Raylib'e özgü bayrakları ayarla (tam ekran, boyut vb.)
    fe_window_set_raylib_config(config);
    
    // Raylib pencereyi başlatır
    InitWindow(config->width, config->height, config->title);

    if (!IsWindowReady()) {
        FE_LOG_FATAL("Pencere baslatilamadi veya grafik bağlami olusturulamadi!");
        return FE_ERR_GENERAL_UNKNOWN; 
    }

    FE_LOG_INFO("Pencere baslatildi: %s (%dx%d)", config->title, config->width, config->height);
    
    // Motorun diğer modüllerinin (Girdi, Ses vb.) kullanabileceği Raylib başlangıç kontrolü
    // SetExitKey(KEY_NULL); // Raylib'in varsayılan ESC ile kapatmasını devre dışı bırakmak isteyebilirsiniz.

    return FE_OK;
}

/**
 * Uygulama: fe_window_shutdown
 */
void fe_window_shutdown(void) {
    if (IsWindowReady()) {
        CloseWindow();
        FE_LOG_INFO("Pencere ve grafik bağlami kapatildi.");
    }
}

/**
 * Uygulama: fe_window_should_close
 */
bool fe_window_should_close(void) {
    // Raylib'in kapatma olayını kontrol eder (X butonu, ESC tuşu vb.)
    return WindowShouldClose();
}

/**
 * Uygulama: fe_window_begin_drawing
 */
void fe_window_begin_drawing(void) {
    // Raylib'in çizim komutlarını kaydetmeye başla
    BeginDrawing();
}

/**
 * Uygulama: fe_window_end_drawing
 */
void fe_window_end_drawing(void) {
    // Raylib'in çizimi bitir ve tamponları takas et (VSync varsa burada bekler)
    EndDrawing();
}

/**
 * Uygulama: fe_window_sync_fps
 */
void fe_window_sync_fps(void) {
    // Raylib, SetTargetFPS() ile ayarlanan değere göre dahili olarak bekleme yapar.
    // Bu fonksiyon, Raylib'in bu dahili mekanizmasını kullanmak için sadece bir iskelettir.
    // Motorun ana döngüsü EndDrawing çağrısından sonra delta_time kullanılarak kendi 
    // bekleme mantığını (fe_timer kullanarak) da uygulayabilir.
}