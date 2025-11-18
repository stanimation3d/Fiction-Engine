// src/graphics/fe_renderer.c

#include "graphics/fe_renderer.h"
#include "graphics/opengl/fe_gl_backend.h" // OpenGL yedek backend'i (varsayalım mevcut)
#include "graphics/dynamicr/fe_dynamicr_backend.h"
#include "graphics/geometryv/fe_geometryv_backend.h"
#include "utils/fe_logger.h"
#include <stdlib.h>
#include <GL/gl.h> // OpenGL komutları

// ----------------------------------------------------------------------
// 1. RENDERER GLOBAL DURUMU VE İŞLEV İŞARETÇİLERİ
// ----------------------------------------------------------------------

// Global durum yapısı
static struct {
    fe_render_backend_type_t active_backend;
    int screen_width;
    int screen_height;
} g_renderer_state = { FE_BACKEND_NONE, 0, 0 };

// Backend'e özgü işlev işaretçisi tablosu (Renderer Interface)
typedef struct fe_backend_interface {
    fe_error_code_t (*init)(int width, int height);
    void (*shutdown)(void);
    void (*begin_frame)(void);
    void (*end_frame)(void);
    void (*draw_mesh)(const fe_mesh_t* mesh, uint32_t instance_count);
    void (*execute_passes)(const fe_mat4_t* view, const fe_mat4_t* proj);
    void (*bind_framebuffer)(fe_framebuffer_t* fbo);
    void (*clear_framebuffer)(fe_framebuffer_t* fbo, fe_clear_flags_t flags, float r, float g, float b, float a, float depth);
    void (*load_scene_geometry)(const fe_mesh_t* const* meshes, uint32_t mesh_count);
} fe_backend_interface_t;


// Mevcut backendlere ait arayüz yapıları
// NOTE: fe_gl_backend'in burada tanımlı olduğu varsayılır.
static const fe_backend_interface_t g_gl_interface = {
    // fe_gl_backend.c'deki ilgili fonksiyon isimleri
    fe_gl_init, fe_gl_shutdown, fe_gl_begin_frame, fe_gl_end_frame, 
    fe_gl_draw_mesh, fe_gl_execute_passes, fe_gl_bind_framebuffer, 
    fe_gl_clear_framebuffer, NULL // OpenGL için özel sahne yükleme gerekmez
};

static const fe_backend_interface_t g_dynamicr_interface = {
    fe_dynamicr_init, fe_dynamicr_shutdown, fe_dynamicr_begin_frame, fe_dynamicr_end_frame, 
    fe_dynamicr_draw_mesh, fe_dynamicr_execute_passes, fe_dynamicr_bind_framebuffer, 
    fe_dynamicr_clear_framebuffer, NULL // DynamicR için özel sahne yükleme gerekmez
};

static const fe_backend_interface_t g_geometryv_interface = {
    fe_geometryv_init, fe_geometryv_shutdown, fe_geometryv_begin_frame, fe_geometryv_end_frame, 
    fe_geometryv_draw_mesh, fe_geometryv_execute_passes, fe_geometryv_bind_framebuffer, 
    fe_geometryv_clear_framebuffer, fe_geometryv_load_scene_geometry
};

// Aktif arayüze işaretçi
static const fe_backend_interface_t* g_active_interface = NULL;

// ----------------------------------------------------------------------
// 2. RENDER YAŞAM DÖNGÜSÜ UYGULAMALARI
// ----------------------------------------------------------------------

/**
 * Uygulama: fe_renderer_init
 */
fe_error_code_t fe_renderer_init(int width, int height, fe_render_backend_type_t backend_type) {
    FE_LOG_INFO("Renderer baslatiliyor (Backend: %d)...", backend_type);
    
    g_renderer_state.screen_width = width;
    g_renderer_state.screen_height = height;
    
    // 1. Backend arayüzünü seç
    switch (backend_type) {
        case FE_BACKEND_DYNAMICR:
            g_active_interface = &g_dynamicr_interface;
            break;
        case FE_BACKEND_GEOMETRYV:
            g_active_interface = &g_geometryv_interface;
            break;
        case FE_BACKEND_OPENGL:
        default:
            g_active_interface = &g_gl_interface;
            backend_type = FE_BACKEND_OPENGL;
            break;
    }
    
    // 2. Aktif Backend'i başlat
    if (g_active_interface && g_active_interface->init) {
        fe_error_code_t result = g_active_interface->init(width, height);
        if (result == FE_OK) {
            g_renderer_state.active_backend = backend_type;
            FE_LOG_INFO("Renderer ve Backend (%d) baslatma basarili.", backend_type);
            return FE_OK;
        } else {
            FE_LOG_FATAL("Backend (%d) baslatilirken hata olustu: %d", backend_type, result);
            g_active_interface = NULL; // Başlatma başarısız oldu
            return result;
        }
    }
    
    FE_LOG_FATAL("Gecersiz veya desteklenmeyen render backend tipi: %d", backend_type);
    return FE_ERR_INVALID_ARGUMENT;
}

/**
 * Uygulama: fe_renderer_shutdown
 */
void fe_renderer_shutdown(void) {
    if (g_active_interface && g_active_interface->shutdown) {
        g_active_interface->shutdown();
        FE_LOG_INFO("Aktif Renderer Backend (%d) kapatildi.", g_renderer_state.active_backend);
    }
    g_active_interface = NULL;
    g_renderer_state.active_backend = FE_BACKEND_NONE;
}

// ----------------------------------------------------------------------
// 3. KARE VE ÇİZİM YÖNETİMİ UYGULAMALARI
// ----------------------------------------------------------------------

/**
 * Uygulama: fe_renderer_begin_frame
 */
void fe_renderer_begin_frame(void) {
    if (g_active_interface && g_active_interface->begin_frame) {
        g_active_interface->begin_frame();
    }
}

/**
 * Uygulama: fe_renderer_end_frame
 */
void fe_renderer_end_frame(void) {
    if (g_active_interface && g_active_interface->end_frame) {
        g_active_interface->end_frame();
    }
    // Ana uygulama, görüntü tamponunu burada takas etmelidir (fe_gl_swap_buffers çağrısı).
}

/**
 * Uygulama: fe_renderer_draw_mesh
 */
void fe_renderer_draw_mesh(const fe_mesh_t* mesh, uint32_t instance_count) {
    if (g_active_interface && g_active_interface->draw_mesh) {
        g_active_interface->draw_mesh(mesh, instance_count);
    }
}

/**
 * Uygulama: fe_renderer_execute_passes
 */
void fe_renderer_execute_passes(const fe_mat4_t* view, const fe_mat4_t* proj) {
    if (g_active_interface && g_active_interface->execute_passes) {
        g_active_interface->execute_passes(view, proj);
    }
}

// ----------------------------------------------------------------------
// 4. DURUM VE FBO YÖNETİMİ UYGULAMALARI
// ----------------------------------------------------------------------

/**
 * Uygulama: fe_renderer_bind_framebuffer
 */
void fe_renderer_bind_framebuffer(fe_framebuffer_t* fbo) {
    if (g_active_interface && g_active_interface->bind_framebuffer) {
        g_active_interface->bind_framebuffer(fbo);
    } else {
        // Yedek olarak OpenGL varsayılanını çağır
        glBindFramebuffer(GL_FRAMEBUFFER, fbo ? fbo->fbo_id : 0);
    }
}

/**
 * Uygulama: fe_renderer_clear
 */
void fe_renderer_clear(fe_clear_flags_t flags, float r, float g, float b, float a, float depth) {
    // Clear işlemini gerçekleştirmek için aktif backend'i çağırır
    if (g_active_interface && g_active_interface->clear_framebuffer) {
        // Clear işlemi için FBO'nun zaten bağlı olduğu varsayılır
        g_active_interface->clear_framebuffer(NULL, flags, r, g, b, a, depth); 
    }
}

/**
 * Uygulama: fe_renderer_load_scene_geometry
 */
void fe_renderer_load_scene_geometry(const fe_mesh_t* const* meshes, uint32_t mesh_count) {
    // Bu fonksiyon sadece bazı backendlere (Örn: GeometryV) özgüdür.
    if (g_active_interface && g_active_interface->load_scene_geometry) {
        g_active_interface->load_scene_geometry(meshes, mesh_count);
    } else {
        FE_LOG_DEBUG("Aktif backend bu ozel sahne yukleme islevini desteklemiyor.");
    }
}