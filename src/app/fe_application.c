// src/app/fe_application.c

#include "app/fe_application.h"
#include "utils/fe_logger.h"          // Motor loglama sistemi
#include "platform/fe_platform.h"     // Pencere ve zamanlama islemleri (Varsayilir)
#include "input/fe_input.h"           // Girdi sistemi
#include "graphics/fe_renderer.h"     // Render sistemi
#include "graphics/fe_renderer_tools.h" // Renderer ayarları

#include <string.h> // memcpy

// ----------------------------------------------------------------------
// 1. GLOBAL UYGULAMA DURUMU
// ----------------------------------------------------------------------

static fe_application_t g_app_state = {0};


// ----------------------------------------------------------------------
// 2. MOTOR YAŞAM DÖNGÜSÜ
// ----------------------------------------------------------------------

/**
 * Uygulama: fe_application_init
 */
fe_error_code_t fe_application_init(const fe_app_config_t* config) {
    if (g_app_state.is_running) {
        FE_LOG_WARN("Uygulama zaten calisiyor. Yeniden baslatma engellendi.");
        return FE_OK;
    }

    FE_LOG_INFO("--- Frontend Engine Baslatiliyor ---");

    // 1. Konfigürasyonu Kaydet
    memcpy(&g_app_state.config, config, sizeof(fe_app_config_t));
    g_app_state.is_running = true;

    // 2. Temel Alt Sistemleri Başlat
    
    // Loglama (Varsayalim ki fe_logger zaten baslatildi veya otomatik baslatilir)
    
    // Platform (Pencere, Girdi Olaylari)
    fe_error_code_t result = fe_platform_init(config->window_title, config->window_width, config->window_height, config->fullscreen);
    if (result != FE_OK) return result;

    // Giriş Sistemi
    fe_input_init();
    
    // Renderer Tools (Ayarlar)
    fe_renderer_tools_init();
    
    // Renderer (Grafik Backend)
    result = fe_renderer_init(config->window_width, config->window_height, config->render_backend);
    if (result != FE_OK) {
        fe_application_shutdown();
        return result;
    }

    // 3. Kamera Başlatma
    // Varsayılan kamera oluşturulur (45 derece FOV, Yakın: 0.1, Uzak: 1000.0)
    g_app_state.main_camera = fe_camera3d_create(
        (float)(M_PI / 4.0f), // 45 derece radyan
        (float)config->window_width / (float)config->window_height,
        0.1f, 1000.0f
    );
    if (!g_app_state.main_camera) {
        fe_application_shutdown();
        return FE_ERR_FATAL_ERROR;
    }
    
    // Kamerayı başlangıç konumuna ayarla
    fe_vec3_t cam_pos = {0.0f, 2.0f, 5.0f};
    fe_camera3d_set_transform(g_app_state.main_camera, &cam_pos, 0.0f, 0.0f);


    // 4. Test Sahnesi Yüklemesi (Basitlik için yer tutucu)
    // fe_renderer_load_scene_geometry(test_meshes, mesh_count);

    FE_LOG_INFO("--- Engine Hazir ve Calismaya Baslayacak ---");
    g_app_state.last_frame_time = fe_platform_get_time(); // İlk zamanı kaydet
    return FE_OK;
}

/**
 * Uygulama: fe_application_quit
 */
void fe_application_quit(void) {
    FE_LOG_INFO("Uygulama sonlandirma talep edildi.");
    g_app_state.is_running = false;
}

/**
 * Uygulama: fe_application_shutdown
 */
void fe_application_shutdown(void) {
    FE_LOG_INFO("--- Frontend Engine Kapatiliyor ---");
    
    // Kaynakları serbest bırakma sırası genellikle tersidir.
    if (g_app_state.main_camera) {
        fe_camera3d_destroy(g_app_state.main_camera);
        g_app_state.main_camera = NULL;
    }
    
    fe_renderer_shutdown();
    fe_renderer_tools_shutdown();
    fe_input_shutdown();
    
    fe_platform_shutdown(); // Pencereyi ve platformu kapat

    FE_LOG_INFO("--- Kapatma Tamamlandi ---");
    g_app_state.is_running = false;
}


// ----------------------------------------------------------------------
// 3. ANA DÖNGÜ MANTIĞI
// ----------------------------------------------------------------------

/**
 * @brief Motorun bir karesini gunceller (Mantık, Fizik, Yapay Zeka).
 */
static void fe_application_update(fe_application_t* app) {
    float dt = app->delta_time;
    
    // 1. Kamera Hareketini Giriş Sisteminden Güncelle (Test için)
    fe_vec3_t move_dir = {0.0f, 0.0f, 0.0f}; // X, Y, Z (Sağ, Yukarı, İleri)

    if (fe_input_is_key_down(FE_KEY_W)) move_dir.z += 1.0f;
    if (fe_input_is_key_down(FE_KEY_S)) move_dir.z -= 1.0f;
    if (fe_input_is_key_down(FE_KEY_A)) move_dir.x -= 1.0f;
    if (fe_input_is_key_down(FE_KEY_D)) move_dir.x += 1.0f;
    
    if (fe_input_is_key_down(FE_KEY_LSHIFT)) move_dir = fe_vec3_scale(move_dir, 2.0f); // Hızlandır

    // ESC basıldığında uygulamayı kapat
    if (fe_input_is_key_pressed(FE_KEY_ESCAPE)) fe_application_quit();

    fe_camera3d_move(app->main_camera, &move_dir, dt);

    // Fare dönüşü (Sadece sağ tuşa basılıyorsa döndür)
    if (fe_input_is_mouse_button_down(FE_MOUSE_BUTTON_RIGHT)) {
        float sensitivity = 0.003f;
        float delta_yaw = fe_input_get_mouse_delta_x() * sensitivity;
        float delta_pitch = fe_input_get_mouse_delta_y() * sensitivity;
        
        fe_camera3d_rotate(app->main_camera, delta_yaw, delta_pitch);
    }
    
    // 2. Diğer Mantık, Fizik, Animasyon Güncellemeleri buraya gelir
    // fe_physics_update(dt);
    // fe_animation_update(dt);
}

/**
 * @brief Motorun bir karesini çizer (Render).
 */
static void fe_application_render(fe_application_t* app) {
    // 1. Render Hazırlığı
    fe_renderer_begin_frame();
    
    // 2. Hedef FBO'yu Bağla (Varsayılan olarak Ana Pencere)
    fe_renderer_bind_framebuffer(NULL);
    
    // 3. Ekranı Temizle
    fe_renderer_clear(FE_CLEAR_COLOR | FE_CLEAR_DEPTH, 0.1f, 0.1f, 0.15f, 1.0f, 1.0f);

    // 4. Render Pass'leri Yürüt (G-Buffer, Ray Tracing, Illumination, vb.)
    fe_renderer_execute_passes(&app->main_camera->view_matrix, &app->main_camera->projection_matrix);
    
    // 5. Nihai Görüntüyü Ekrana Gönder
    fe_renderer_end_frame(); // (Bu, fe_platform_swap_buffers'ı çağırır)
}

/**
 * Uygulama: fe_application_run
 */
void fe_application_run(void) {
    if (!g_app_state.is_running) return;

    // Ana Oyun Döngüsü
    while (g_app_state.is_running) {
        
        // --- 1. Zamanlama (Delta Time) ---
        double current_time = fe_platform_get_time();
        g_app_state.delta_time = (float)(current_time - g_app_state.last_frame_time);
        g_app_state.last_frame_time = current_time;
        
        // --- 2. Girdi Güncellemesi ---
        // Platformdan gelen olayları (fe_input_on_key_event) işle
        fe_platform_process_events(); 
        fe_input_begin_frame(); // Bu, delta fare pozisyonlarını hesaplar ve durumu günceller.
        
        // --- 3. Motor Güncellemesi ---
        fe_application_update(&g_app_state);
        
        // --- 4. Render ---
        fe_application_render(&g_app_state);

        // Pencerenin kapatılma isteği varsa döngüden çık
        if (fe_platform_window_should_close()) {
            fe_application_quit();
        }
    }
}
