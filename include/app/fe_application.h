// include/app/fe_application.h

#ifndef FE_APPLICATION_H
#define FE_APPLICATION_H

#include <stdint.h>
#include <stdbool.h>
#include "error/fe_error.h"
#include "graphics/fe_renderer.h" // Render Backend tipini kullanmak için
#include "math/fe_camera3d.h"     // Ana kamerayı kullanmak için

// ----------------------------------------------------------------------
// 1. YAPILANDIRMA YAPISI
// ----------------------------------------------------------------------

/**
 * @brief Uygulama baslatilirken kullanilacak genel ayarlar.
 */
typedef struct fe_app_config {
    const char* window_title;
    int window_width;
    int window_height;
    bool fullscreen;
    fe_render_backend_type_t render_backend; // Kullanilacak render backend tipi
} fe_app_config_t;

// ----------------------------------------------------------------------
// 2. ANA UYGULAMA YAPISI
// ----------------------------------------------------------------------

/**
 * @brief Uygulama çekirdegi tarafindan tutulan temel veriler.
 */
typedef struct fe_application {
    bool is_running;
    fe_app_config_t config;
    
    // Zamanlama verileri
    double last_frame_time;
    float delta_time; // Son kareden bu yana geçen süre (saniye)

    // Motorun Temel Sistemleri
    // NOTE: Bu moduller ayri ayri baslatilip kapatilir.
    // fe_renderer, fe_input, fe_logger, fe_platform vb.

    // Sahne verisi (Basit yer tutucu)
    fe_camera3d_t* main_camera;

    // TODO: Bu yapiya pencere isaretçisi (Window Handle) eklenecektir.

} fe_application_t;


// ----------------------------------------------------------------------
// 3. YÖNETİM VE YAŞAM DÖNGÜSÜ FONKSİYONLARI
// ----------------------------------------------------------------------

/**
 * @brief Uygulamayi baslatir, alt sistemleri hazirlar ve pencereyi olusturur.
 * * @param config Uygulama baslatma ayarlari.
 * @return Basariliysa FE_OK, degilse hata kodu.
 */
fe_error_code_t fe_application_init(const fe_app_config_t* config);

/**
 * @brief Ana oyun/render döngüsünü çalistirir.
 * * Baslatma basarili olduktan sonra çagrilir.
 */
void fe_application_run(void);

/**
 * @brief Uygulamanin çalismasini sonlandirmasini ister (Döngüden çikilmasi).
 */
void fe_application_quit(void);

/**
 * @brief Uygulamayi ve tüm alt sistemleri kapatir, kaynaklari serbest birakir.
 */
void fe_application_shutdown(void);


#endif // FE_APPLICATION_H
