// include/graphics/geometryv/fe_gv_tracer.h

#ifndef FE_GV_TRACER_H
#define FE_GV_TRACER_H

#include <stdint.h>
#include "error/fe_error.h"
#include "graphics/fe_render_types.h"
#include "graphics/fe_material_editor.h" 
#include "graphics/geometryv/fe_gv_scene.h" // fe_gv_scene_t için

// ----------------------------------------------------------------------
// 1. IŞIN TAKİP VERİ YAPILARI (R-Buffer)
// ----------------------------------------------------------------------

/**
 * @brief Işın Takip isabet sonuclarini (Ray Hit) tutan yapi.
 * * Bu yapi, G-Buffer'a benzer sekilde, nihai isiklandirma icin temel verileri saglar.
 */
typedef struct fe_gv_rt_buffer {
    fe_texture_id_t position_map_id; // Dünya pozisyonu ve isabet mesafesi (RGBA16F)
    fe_texture_id_t normal_map_id;   // Yüzey normali ve materyal ID'si (RGBA8/RGBA16F)
    fe_texture_id_t albedo_map_id;   // Albedo ve roughness (RGBA8)
    uint32_t width;
    uint32_t height;
} fe_gv_rt_buffer_t;


/**
 * @brief GeometryV Işın Takip (Tracer) baglami.
 */
typedef struct fe_gv_tracer_context {
    fe_gv_rt_buffer_t r_buffer;       // Ray Tracing sonucunun yazilacagi GPU tamponlari
    fe_material_t* trace_material;    // Işın Takip Compute Shader'ını iceren materyal
} fe_gv_tracer_context_t;


// ----------------------------------------------------------------------
// 2. YÖNETİM VE ÇİZİM FONKSİYONLARI
// ----------------------------------------------------------------------

/**
 * @brief GeometryV Tracer sistemini baslatir (R-Buffer ve Shader'i yukler).
 * * @param width Ekran genisligi.
 * @param height Ekran yuksekligi.
 * @return Baslatilan baglam (context) pointer'i. Basarisiz olursa NULL.
 */
fe_gv_tracer_context_t* fe_gv_tracer_init(uint32_t width, uint32_t height);

/**
 * @brief GeometryV Tracer sistemini kapatir ve kaynaklari serbest birakir.
 */
void fe_gv_tracer_shutdown(fe_gv_tracer_context_t* context);

/**
 * @brief Işın Takip pass'ini calistirir.
 * * Bu fonksiyon, bir Compute Shader'ı calistirarak tüm ekran piksellerine birincil isinleri atar.
 * @param context Tracer baglami.
 * @param scene Sahne geometri verilerini iceren GeometryV Scene.
 */
void fe_gv_tracer_run_primary_rays(fe_gv_tracer_context_t* context, 
                                   const fe_gv_scene_t* scene);

#endif // FE_GV_TRACER_H