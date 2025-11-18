// include/platform/fe_window.h

#ifndef FE_WINDOW_H
#define FE_WINDOW_H

#include <stdint.h>
#include <stdbool.h>
#include "error/fe_error.h"

// ----------------------------------------------------------------------
// 1. GRAFİK API'SI VE PENCERE AYARLARI
// ----------------------------------------------------------------------

/**
 * @brief Fiction Engine'in desteklediği Grafik API'leri.
 * * Bu, motorun dinamik render sisteminin (DynamicR) temelini oluşturur.
 */
typedef enum fe_graphics_api {
    FE_API_OPENGL_43,    // Fiction Engine'in varsayılanı (Raylib tarafından desteklenir)
    FE_API_VULKAN,       // İleride eklenecek yüksek performanslı seçenek
    FE_API_DIRECTX_12,   // İleride eklenecek Windows/Xbox seçeneği
    FE_API_METAL         // İleride eklenecek Apple seçeneği
} fe_graphics_api_t;


/**
 * @brief Pencere oluşturma ve motor ayarlarını içeren yapı.
 */
typedef struct fe_window_config {
    const char* title;               // Pencerenin başlığı
    int width;                       // Başlangıç genişliği
    int height;                      // Başlangıç yüksekliği
    fe_graphics_api_t desired_api;   // Kullanılacak grafik API'si
    bool fullscreen;                 // Tam ekran modunda başlasın mı?
    uint32_t target_fps;             // Hedeflenen Kare Hızı (FPS)
} fe_window_config_t;


// ----------------------------------------------------------------------
// 2. ANA PENCERE FONKSİYONLARI
// ----------------------------------------------------------------------

/**
 * @brief Raylib'i kullanarak pencereyi ve grafik bağlamını (context) başlatır.
 * * @param config Pencere ayarları.
 * @return Başarı durumunda FE_OK.
 */
fe_error_code_t fe_window_init(const fe_window_config_t* config);

/**
 * @brief Pencereyi ve Raylib grafik bağlamını temizler ve kapatır.
 * * Motorun kapanışında çağrılmalıdır.
 */
void fe_window_shutdown(void);

/**
 * @brief Motorun ana döngüsünde pencerenin kapatılıp kapatılmadığını kontrol eder.
 * * @return Kapatılması isteniyorsa true.
 */
bool fe_window_should_close(void);

/**
 * @brief Raylib'in çizime başlaması için gereken fonksiyonu çağırır.
 * * Render döngüsünün başında çağrılmalıdır.
 */
void fe_window_begin_drawing(void);

/**
 * @brief Raylib'in çizimi sonlandırması ve tamponları takas etmesi (swap buffers) için gereken fonksiyonu çağırır.
 * * Render döngüsünün sonunda çağrılmalıdır.
 */
void fe_window_end_drawing(void);

/**
 * @brief Motorun ayarlanan FPS hedefine ulaşması için bekler.
 */
void fe_window_sync_fps(void);

#endif // FE_WINDOW_H