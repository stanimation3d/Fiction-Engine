// src/graphics/dynamicr/fe_dynamicr_backend.c

#include "graphics/dynamicr/fe_dynamicr_backend.h"
#include "graphics/dynamicr/fe_dynamicr_scene.h"
#include "graphics/dynamicr/fe_screen_tracing.h"
#include "graphics/dynamicr/fe_distance_fields.h"
#include "graphics/opengl/fe_gl_commands.h" // Düşük seviye çizim için (fe_gl_draw_mesh çağrılır)
#include "graphics/fe_shader_compiler.h"    // Shader yönetimi için
#include "graphics/fe_renderer.h"           // G-Buffer çizimi için fe_renderer_draw_mesh kullanacağız.
#include "utils/fe_logger.h"
#include <stdlib.h> // calloc, free için


// ----------------------------------------------------------------------
// 1. DYNAMICR GLOBAL DURUMU
// ----------------------------------------------------------------------

// DynamicR'ın tüm kalıcı verilerini ve alt sistem bağlamlarını tutan ana yapı.
static fe_dynamicr_scene_t* g_dynamicr_scene = NULL;


// ----------------------------------------------------------------------
// 2. BACKEND YAŞAM DÖNGÜSÜ UYGULAMALARI
// ----------------------------------------------------------------------

/**
 * Uygulama: fe_dynamicr_init
 */
fe_error_code_t fe_dynamicr_init(int width, int height) {
    FE_LOG_INFO("DynamicR Backend baslatiliyor...");

    // 1. Ana Sahne Yapısını ve Alt Sistemleri Başlat
    g_dynamicr_scene = fe_dynamicr_scene_init(width, height);
    if (!g_dynamicr_scene) {
        return FE_ERR_FATAL_ERROR;
    }
    
    // 2. Uzaklık Alanlarını Oluştur/Yeniden İnşa Et (Başlangıçta)
    // fe_distance_field_rebuild(...) burada çağrılmalıdır.

    // 3. (Gerekirse) Donanımsal Işın Takibi Sistemini Başlat
    // fe_hrt_context_t* hrt_ctx = fe_hrt_init(); // fe_dynamicr_scene'e dahil edilecektir.

    // TODO: Final Compose Shader'ı yükle.

    FE_LOG_INFO("DynamicR Backend hazir.");
    return FE_OK;
}

/**
 * Uygulama: fe_dynamicr_shutdown
 */
void fe_dynamicr_shutdown(void) {
    if (g_dynamicr_scene) {
        fe_dynamicr_scene_shutdown(g_dynamicr_scene);
        g_dynamicr_scene = NULL;
    }
    FE_LOG_INFO("DynamicR Backend kapatildi.");
}

/**
 * Uygulama: fe_dynamicr_begin_frame
 */
void fe_dynamicr_begin_frame(void) {
    // Burada özel bir GL/API çağrısı olmayabilir, ancak alt sistemleri hazırlar.
    // fe_gl_begin_frame() burada çağrılabilir, ancak fe_renderer'da bırakmak daha iyidir.
}

/**
 * Uygulama: fe_dynamicr_end_frame
 */
void fe_dynamicr_end_frame(void) {
    // fe_gl_end_frame() burada çağrılabilir.
}

/**
 * Uygulama: fe_dynamicr_draw_mesh
 */
void fe_dynamicr_draw_mesh(const fe_mesh_t* mesh, uint32_t instance_count) {
    // DynamicR, mesh'leri doğrudan çizmek yerine, G-Buffer geçişi sırasında bunları kullanır.
    // Bu fonksiyon, mesh'i G-Buffer'a çizmek üzere planlamak için kullanılır.
    
    // NOTE: fe_renderer_draw_mesh'in çağrılmasıyla bu kısım atlanabilir.
    // Veya: Bu fonksiyon, mesh'i g_dynamicr_scene->meshes listesine ekler.
    
    // Şimdilik, sadece OpenGL'in düşük seviye çizimini çağırarak G-Buffer geçişini simüle edelim.
    fe_gl_draw_mesh(mesh, instance_count);
}

// ----------------------------------------------------------------------
// 3. RENDER BORU HATTI YÜRÜTÜLMESİ
// ----------------------------------------------------------------------

/**
 * Uygulama: fe_dynamicr_execute_passes
 */
void fe_dynamicr_execute_passes(const fe_mat4_t* view, const fe_mat4_t* proj) {
    if (!g_dynamicr_scene) return;

    // 1. Sahne Durumunu Güncelle
    fe_dynamicr_scene_update(g_dynamicr_scene, view, proj);

    // --- RENDER PASS'LERİ YÜRÜT ---
    
    // PASS 1: G-Buffer Geçişi (Geometri Çizimi)
    // Çıktı: g_dynamicr_scene->gbuffer_fbo
    // * Bu aşamada, sahnede görünür olan tüm mesh'ler G-Buffer'a çizilmelidir.
    // * fe_renderer boru hattı bu FBO'yu bağlar ve fe_dynamicr_draw_mesh çağrıları ile geometri çizilir.
    
    FE_LOG_TRACE("DynamicR Pass 1: G-Buffer Tamamlandı.");
    
    // PASS 2: Screen Tracing (Ekran Alanı Yansıma/GI)
    // Girdi: g_dynamicr_scene->gbuffer_fbo
    // Çıktı: screen_tracing_ctx->output_fbo
    fe_screen_tracing_run(g_dynamicr_scene->screen_tracing_ctx, g_dynamicr_scene->gbuffer_fbo);
    FE_LOG_TRACE("DynamicR Pass 2: Screen Tracing Tamamlandı.");
    
    // TODO: PASS 3: Voxel GI Update/Tracing (Sahne Dışı GI)
    // fe_voxel_gi_run(...)
    
    // TODO: PASS 4: Ray Combiner (Nihai Işıklandırma Birleştirme)
    // Girdi: G-Buffer, Screen Tracing Çıktısı, Voxel GI Çıktısı
    // Çıktı: Nihai Render Tamponu (fe_renderer_bind_framebuffer(NULL) ile ana ekrana)
    
    // fe_ray_combiner_run(...) burada çağrılır.
    
    FE_LOG_TRACE("DynamicR Render Karesi Tamamlandı.");
}

// ----------------------------------------------------------------------
// 4. FBO YÖNETİMİ UYGULAMALARI (fe_renderer.c'den gelen çağrılar)
// ----------------------------------------------------------------------

/**
 * Uygulama: fe_dynamicr_bind_framebuffer
 */
void fe_dynamicr_bind_framebuffer(fe_framebuffer_t* fbo) {
    // DynamicR, FBO'ları kendi içinde yönetir.
    // Dışarıdan gelen bu çağrı genellikle sadece G-Buffer'ın veya Final Compose'un 
    // hedefinin bağlanması için kullanılır.
    
    // NOTE: Bu fonksiyon, fe_gl_backend.c'deki karşılığını çağırır.
    glBindFramebuffer(GL_FRAMEBUFFER, fbo ? fbo->fbo_id : 0);
    // Viewport ayarı da burada yapılmalıdır.
}

/**
 * Uygulama: fe_dynamicr_clear_framebuffer
 */
void fe_dynamicr_clear_framebuffer(fe_framebuffer_t* fbo, fe_clear_flags_t flags, float r, float g, float b, float a, float depth) {
    // NOTE: Bu fonksiyon, fe_gl_backend.c'deki karşılığını çağırır.
    fe_gl_bind_framebuffer(fbo); // FBO'yu bağla

    uint32_t gl_clear_bits = 0;
    if (flags & FE_CLEAR_COLOR) gl_clear_bits |= GL_COLOR_BUFFER_BIT;
    if (flags & FE_CLEAR_DEPTH) gl_clear_bits |= GL_DEPTH_BUFFER_BIT;
    
    if (gl_clear_bits != 0) {
        if (flags & FE_CLEAR_COLOR) glClearColor(r, g, b, a);
        if (flags & FE_CLEAR_DEPTH) glClearDepth(depth);
        glClear(gl_clear_bits);
    }
}