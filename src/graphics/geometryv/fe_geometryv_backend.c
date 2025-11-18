// src/graphics/geometryv/fe_geometryv_backend.c

#include "graphics/geometryv/fe_geometryv_backend.h"
#include "graphics/geometryv/fe_gv_scene.h"
#include "graphics/geometryv/fe_gv_tracer.h"
#include "graphics/geometryv/fe_gv_illuminator.h"
#include "graphics/fe_renderer.h"               // FBO ve Clear islemleri icin
#include "utils/fe_logger.h"
#include <stdlib.h> // calloc, free için
#include <GL/gl.h>  // OpenGL komutları


// ----------------------------------------------------------------------
// 1. GEOMETRYV GLOBAL DURUMU
// ----------------------------------------------------------------------

// GeometryV'nin tüm kalıcı verilerini ve alt sistem bağlamlarını tutan ana yapı.
static fe_gv_scene_t* g_gv_scene = NULL;
static fe_gv_tracer_context_t* g_gv_tracer_ctx = NULL;
static fe_gv_illuminator_context_t* g_gv_illuminator_ctx = NULL;


// ----------------------------------------------------------------------
// 2. BACKEND YAŞAM DÖNGÜSÜ UYGULAMALARI
// ----------------------------------------------------------------------

/**
 * Uygulama: fe_geometryv_init
 */
fe_error_code_t fe_geometryv_init(int width, int height) {
    FE_LOG_INFO("GeometryV Backend baslatiliyor...");

    // 1. Alt Sistemleri Başlat
    
    // Scene (Sahne Veri Yönetimi)
    g_gv_scene = fe_gv_scene_init();
    if (!g_gv_scene) return FE_ERR_FATAL_ERROR;

    // Tracer (Işın Takibi Çekirdeği)
    g_gv_tracer_ctx = fe_gv_tracer_init(width, height);
    if (!g_gv_tracer_ctx) {
        fe_geometryv_shutdown();
        return FE_ERR_FATAL_ERROR;
    }

    // Illuminator (Aydınlatma ve Gölgeleme)
    g_gv_illuminator_ctx = fe_gv_illuminator_init();
    if (!g_gv_illuminator_ctx) {
        fe_geometryv_shutdown();
        return FE_ERR_FATAL_ERROR;
    }
    
    FE_LOG_INFO("GeometryV Backend hazir.");
    return FE_OK;
}

/**
 * Uygulama: fe_geometryv_shutdown
 */
void fe_geometryv_shutdown(void) {
    FE_LOG_INFO("GeometryV Backend kapatiliyor...");
    
    if (g_gv_illuminator_ctx) {
        fe_gv_illuminator_shutdown(g_gv_illuminator_ctx);
        g_gv_illuminator_ctx = NULL;
    }
    
    if (g_gv_tracer_ctx) {
        fe_gv_tracer_shutdown(g_gv_tracer_ctx);
        g_gv_tracer_ctx = NULL;
    }

    if (g_gv_scene) {
        fe_gv_scene_shutdown(g_gv_scene);
        g_gv_scene = NULL;
    }
}

// ----------------------------------------------------------------------
// 3. SAHNE YÖNETİMİ VE RENDER BORU HATTI
// ----------------------------------------------------------------------

/**
 * Uygulama: fe_geometryv_load_scene_geometry
 */
void fe_geometryv_load_scene_geometry(const fe_mesh_t* const* meshes, uint32_t mesh_count) {
    if (!g_gv_scene) {
        FE_LOG_ERROR("Scene yuklenemedi: GeometryV Scene baslatilmamis.");
        return;
    }
    
    // 1. Geometriyi Scene yapisina yukle (Kumelenme ve Ucgen SSBO'su hazirlanir)
    fe_gv_scene_load_geometry(g_gv_scene, meshes, mesh_count);
    
    // 2. Hiyerarsiyi insa et (Işın takibi optimizasyonu)
    fe_gv_scene_build_hierarchy(g_gv_scene);
}

/**
 * Uygulama: fe_geometryv_execute_passes
 */
void fe_geometryv_execute_passes(const fe_mat4_t* view, const fe_mat4_t* proj) {
    if (!g_gv_scene || !g_gv_tracer_ctx || !g_gv_illuminator_ctx) return;

    // 1. Sahne Uniform'larını Güncelle
    fe_gv_scene_update(g_gv_scene, view, proj);

    // --- RENDER PASS'LERİ YÜRÜT ---
    
    // PASS 1: Birincil Işın Takibi (Ray Tracing)
    // Cikti: R-Buffer (fe_gv_tracer_context_t->r_buffer)
    fe_gv_tracer_run_primary_rays(g_gv_tracer_ctx, g_gv_scene);
    FE_LOG_TRACE("GeometryV Pass 1: Işın Takibi Tamamlandı.");
    
    // PASS 2: Aydınlatma ve Gölgeleme (Illumination)
    // Girdi: R-Buffer. Cikti: Nihai ekrana (NULL FBO).
    fe_gv_illuminator_run(g_gv_illuminator_ctx, 
                          &g_gv_tracer_ctx->r_buffer, 
                          NULL, // NULL, nihai sonucu fe_renderer'in ayarladigi ana ekrana yazar.
                          view, 
                          proj);
    FE_LOG_TRACE("GeometryV Pass 2: Aydınlatma Tamamlandı.");
}

/**
 * Uygulama: fe_geometryv_begin_frame / fe_geometryv_end_frame / fe_geometryv_draw_mesh
 * GeometryV, ışın takibi tabanlı olduğu için, bu fonksiyonlar boş kalır veya yer tutucu olarak kullanılır.
 */
void fe_geometryv_begin_frame(void) {}
void fe_geometryv_end_frame(void) {}
void fe_geometryv_draw_mesh(const fe_mesh_t* mesh, uint32_t instance_count) {
    // Işın takibi sistemleri, render pass'i sırasında tüm geometriyi yeniden işler. 
    // Bu çağrı yok sayılır, ancak geometri fe_geometryv_load_scene_geometry'de yüklenmiş olmalıdır.
}

// ----------------------------------------------------------------------
// 4. FBO YÖNETİMİ UYGULAMALARI (fe_renderer.c'den gelen çağrılar)
// ----------------------------------------------------------------------

/**
 * Uygulama: fe_geometryv_bind_framebuffer
 */
void fe_geometryv_bind_framebuffer(fe_framebuffer_t* fbo) {
    // NOTE: Bu fonksiyon, fe_gl_backend.c'deki karşılığını çağırır.
    glBindFramebuffer(GL_FRAMEBUFFER, fbo ? fbo->fbo_id : 0);
}

/**
 * Uygulama: fe_geometryv_clear_framebuffer
 */
void fe_geometryv_clear_framebuffer(fe_framebuffer_t* fbo, fe_clear_flags_t flags, float r, float g, float b, float a, float depth) {
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
