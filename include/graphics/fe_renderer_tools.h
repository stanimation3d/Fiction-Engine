// include/graphics/fe_renderer_tools.h

#ifndef FE_RENDERER_TOOLS_H
#define FE_RENDERER_TOOLS_H

#include <stdint.h>
#include <stdbool.h>
#include "error/fe_error.h"

// ----------------------------------------------------------------------
// 1. GENEL AYAR YAPISI
// ----------------------------------------------------------------------

/**
 * @brief Genel renderer ayarlarini tutan yapi.
 */
typedef struct fe_render_settings {
    float exposure;             // Görüntü parlakligi ayari
    bool vsync_enabled;         // V-Sync durumu
    bool debug_overlay_enabled; // Hata ayiklama katmaninin gösterilip gösterilmedigi
    uint32_t shadow_map_resolution; // Tüm gölge haritalari icin çözünürlük
    
    // Geçis/Katman görüntüleme ayarları (Hata ayıklama için)
    uint32_t debug_view_mode;   // 0: Final Color, 1: Albedo, 2: Normals, vb.
} fe_render_settings_t;

// ----------------------------------------------------------------------
// 2. DYNAMICR ÖZEL AYARLAR
// ----------------------------------------------------------------------

/**
 * @brief DynamicR backend'ine özgü ayarlar.
 */
typedef struct fe_dynamicr_settings {
    bool ssr_enabled;           // Screen Space Reflection (Ekran Alanı Yansımaları)
    bool ssaa_enabled;          // Screen Space Ambient Occlusion (Ekran Alanı Ortam Kapatma)
    uint32_t hrtr_sample_count; // Hibrit Ray Tracing'de kullanilacak ornek sayisi
    float gi_downsample_factor; // GI pass'inin çözünürlük faktörü (Örn: 0.5)
} fe_dynamicr_settings_t;

// ----------------------------------------------------------------------
// 3. GEOMETRYV ÖZEL AYARLAR
// ----------------------------------------------------------------------

/**
 * @brief GeometryV backend'ine özgü ayarlar.
 */
typedef struct fe_geometryv_settings {
    uint32_t max_bvh_depth;     // BVH (Hiyerarşi) için maksimum derinlik
    uint32_t primary_ray_samples; // Birincil ışınlar için ornek sayisi (Anti-aliasing)
    bool secondary_rays_enabled; // İkincil ışınların (gölge, yansıma) etkinlestirilmesi
    float cluster_size_factor;  // Geometri kümelemede kullanilacak boyut faktörü
} fe_geometryv_settings_t;


// ----------------------------------------------------------------------
// 4. YÖNETİM FONKSİYONLARI
// ----------------------------------------------------------------------

/**
 * @brief Renderer Tools sistemini baslatir (Ayarlari varsayilanlara sifirlar).
 */
void fe_renderer_tools_init(void);

/**
 * @brief Renderer Tools sistemini kapatir.
 */
void fe_renderer_tools_shutdown(void);

/**
 * @brief Mevcut genel render ayarlarini döndürür.
 */
fe_render_settings_t* fe_renderer_tools_get_settings(void);

/**
 * @brief Aktif backend'e (DynamicR) özgü ayarlar yapisini döndürür.
 * * Sadece aktif backend DynamicR ise geçerlidir.
 */
fe_dynamicr_settings_t* fe_renderer_tools_get_dynamicr_settings(void);

/**
 * @brief Aktif backend'e (GeometryV) özgü ayarlar yapisini döndürür.
 * * Sadece aktif backend GeometryV ise geçerlidir.
 */
fe_geometryv_settings_t* fe_renderer_tools_get_geometryv_settings(void);

/**
 * @brief Genel ve backend'e özgü ayarlari GPU'ya senkronize eder.
 * * Bu, her kare basinda veya ayar degistirildiginde çagrilir.
 */
void fe_renderer_tools_sync_gpu_settings(void);


#endif // FE_RENDERER_TOOLS_H