// src/graphics/fe_renderer_tools.c

#include "graphics/fe_renderer_tools.h"
#include "graphics/fe_renderer.h"               // Aktif backend tipini ögrenmek için
#include "graphics/opengl/fe_gl_device.h"       // UBO yönetimi için
#include "utils/fe_logger.h"
#include <stdlib.h> // calloc, free için
#include <string.h> // memcpy için

// ----------------------------------------------------------------------
// 1. GLOBAL AYAR DEPOLAMA VE UBO (Uniform Buffer Object)
// ----------------------------------------------------------------------

// Tüm ayarları bir arada tutan merkezi yapı
static struct {
    fe_render_settings_t general;
    fe_dynamicr_settings_t dynamicr;
    fe_geometryv_settings_t geometryv;
    fe_buffer_id_t settings_ubo; // Tüm ayarların yükleneceği GPU tamponu
} g_render_tools_state;

// Uniform Buffer Bağlama Noktası (Her shader'da ayni olmalidir)
#define SETTINGS_UBO_BINDING_POINT 0 

// UBO'nun toplam boyutu (Bu, padding nedeniyle yapi boyutlarindan büyük olabilir)
#define SETTINGS_UBO_SIZE (sizeof(fe_render_settings_t) + sizeof(fe_dynamicr_settings_t) + sizeof(fe_geometryv_settings_t) + 64)


// ----------------------------------------------------------------------
// 2. YÖNETİM FONKSİYONLARI UYGULAMALARI
// ----------------------------------------------------------------------

/**
 * Uygulama: fe_renderer_tools_init
 */
void fe_renderer_tools_init(void) {
    FE_LOG_INFO("Renderer Tools baslatiliyor...");
    
    // Varsayılan Genel Ayarlar
    g_render_tools_state.general.exposure = 1.0f;
    g_render_tools_state.general.vsync_enabled = true;
    g_render_tools_state.general.debug_overlay_enabled = false;
    g_render_tools_state.general.shadow_map_resolution = 1024;
    g_render_tools_state.general.debug_view_mode = 0; 

    // Varsayılan DynamicR Ayarları
    g_render_tools_state.dynamicr.ssr_enabled = true;
    g_render_tools_state.dynamicr.ssaa_enabled = true;
    g_render_tools_state.dynamicr.hrtr_sample_count = 8;
    g_render_tools_state.dynamicr.gi_downsample_factor = 0.5f;
    
    // Varsayılan GeometryV Ayarları
    g_render_tools_state.geometryv.max_bvh_depth = 12;
    g_render_tools_state.geometryv.primary_ray_samples = 1;
    g_render_tools_state.geometryv.secondary_rays_enabled = false;
    g_render_tools_state.geometryv.cluster_size_factor = 0.5f;

    // UBO'yu Oluştur ve Ayır
    g_render_tools_state.settings_ubo = fe_gl_device_create_buffer(
        SETTINGS_UBO_SIZE, NULL, FE_BUFFER_USAGE_DYNAMIC);
        
    if (g_render_tools_state.settings_ubo == 0) {
        FE_LOG_FATAL("Ayarlar UBO'su olusturulamadi!");
    }
    
    FE_LOG_DEBUG("Renderer Tools baslatma tamamlandi. UBO ID: %u", g_render_tools_state.settings_ubo);
}

/**
 * Uygulama: fe_renderer_tools_shutdown
 */
void fe_renderer_tools_shutdown(void) {
    if (g_render_tools_state.settings_ubo != 0) {
        fe_gl_device_destroy_buffer(g_render_tools_state.settings_ubo);
        g_render_tools_state.settings_ubo = 0;
    }
    FE_LOG_DEBUG("Renderer Tools kapatildi.");
}

/**
 * Uygulama: fe_renderer_tools_get_settings
 */
fe_render_settings_t* fe_renderer_tools_get_settings(void) {
    return &g_render_tools_state.general;
}

/**
 * Uygulama: fe_renderer_tools_get_dynamicr_settings
 */
fe_dynamicr_settings_t* fe_renderer_tools_get_dynamicr_settings(void) {
    // Güvenlik kontrolü yapılabilir: if (fe_renderer_get_active_backend() == FE_BACKEND_DYNAMICR) 
    return &g_render_tools_state.dynamicr;
}

/**
 * Uygulama: fe_renderer_tools_get_geometryv_settings
 */
fe_geometryv_settings_t* fe_renderer_tools_get_geometryv_settings(void) {
    // Güvenlik kontrolü yapılabilir: if (fe_renderer_get_active_backend() == FE_BACKEND_GEOMETRYV) 
    return &g_render_tools_state.geometryv;
}

/**
 * Uygulama: fe_renderer_tools_sync_gpu_settings
 */
void fe_renderer_tools_sync_gpu_settings(void) {
    if (g_render_tools_state.settings_ubo == 0) return;
    
    // 1. Tüm ayarları sürekli bir CPU tamponunda birleştir (Gerekirse GPU düzenine göre paketle)
    
    // Basitlik için, CPU belleğindeki tüm ayarları birleştirilmiş bir blok olarak varsayıyoruz.
    // Gerçek uygulamada, UBO padding kurallarına dikkat edilmelidir.
    
    // Bu, UBO'nun GPU'ya aktarılacak kısmını temsil eden bir ara bellek olabilir.
    // NOTE: C'de struct üyelerinin sıralanması önemlidir ve GPU'daki karşılığıyla eşleşmelidir.
    
    char buffer[SETTINGS_UBO_SIZE];
    size_t offset = 0;

    // Genel Ayarlar
    memcpy(buffer + offset, &g_render_tools_state.general, sizeof(fe_render_settings_t));
    offset += sizeof(fe_render_settings_t);
    // Gerekli GPU padding'i ekle (örneğin 16 bayt hizalama için)
    offset = (offset + 15) & ~15; 
    
    // DynamicR Ayarları
    memcpy(buffer + offset, &g_render_tools_state.dynamicr, sizeof(fe_dynamicr_settings_t));
    offset += sizeof(fe_dynamicr_settings_t);
    offset = (offset + 15) & ~15;
    
    // GeometryV Ayarları
    memcpy(buffer + offset, &g_render_tools_state.geometryv, sizeof(fe_geometryv_settings_t));
    offset += sizeof(fe_geometryv_settings_t);
    
    // 2. Veriyi GPU'ya yükle
    fe_gl_device_update_buffer(g_render_tools_state.settings_ubo, buffer, offset);
    
    // 3. UBO'yu Bağla (Tüm shader'ların erişimi için)
    fe_gl_cmd_bind_ubo(g_render_tools_state.settings_ubo, SETTINGS_UBO_BINDING_POINT);

    FE_LOG_TRACE("Renderer ayarları GPU'ya senkronize edildi (UBO: %u, Boyut: %zu).", 
                 g_render_tools_state.settings_ubo, offset);
}