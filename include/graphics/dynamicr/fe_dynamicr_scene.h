// include/graphics/dynamicr/fe_dynamicr_scene.h

#ifndef FE_DYNAMICR_SCENE_H
#define FE_DYNAMICR_SCENE_H

#include <stdint.h>
#include "error/fe_error.h"
#include "graphics/fe_render_types.h"
#include "graphics/dynamicr/fe_screen_tracing.h" // Screen Tracing Context'i dahil et
#include "math/fe_matrix.h" // fe_mat4_t için

// ----------------------------------------------------------------------
// 1. DYNAMICR SAHNE VERİ YAPILARI
// ----------------------------------------------------------------------

// Basit bir isik yapısı (DynamicR, tum isiklari isin takibinde kullanir)
typedef struct fe_dynamicr_light {
    fe_vec3_t position;
    fe_vec3_t color;
    float intensity;
    // ... diger isik parametreleri
} fe_dynamicr_light_t;

/**
 * @brief DynamicR Render Boru Hattının ihtiyaç duyduğu tüm sahne kaynaklarini ve durumunu tutar.
 */
typedef struct fe_dynamicr_scene {
    // Render Sistemleri
    fe_screen_tracing_context_t* screen_tracing_ctx;
    // fe_voxel_gi_context_t* voxel_gi_ctx; // Ileride eklenecek
    // fe_ray_combiner_context_t* combiner_ctx; // Ileride eklenecek

    // Sahne Verileri
    fe_mesh_t** meshes;            // Sahnedeki tüm mesh'ler (Yüksek seviye motor bu veriyi yonetmeli)
    uint32_t mesh_count;
    
    fe_dynamicr_light_t* lights;   // Isik verileri
    uint32_t light_count;

    // Kamera Matrisleri (Shader'lara gonderilecek en son veriler)
    fe_mat4_t view_matrix;
    fe_mat4_t projection_matrix;
    
    // G-Buffer (Sahne ciziminin ciktisi)
    fe_framebuffer_t* gbuffer_fbo; 
} fe_dynamicr_scene_t;

// ----------------------------------------------------------------------
// 2. YÖNETİM FONKSİYONLARI
// ----------------------------------------------------------------------

/**
 * @brief DynamicR Scene yapisini baslatir, gerekli alt sistemleri (Screen Tracing vb.) olusturur.
 * * @param width Ekran genisligi.
 * @param height Ekran yuksekligi.
 * @return Yeni olusturulan fe_dynamicr_scene_t pointer'i. Basarisiz olursa NULL.
 */
fe_dynamicr_scene_t* fe_dynamicr_scene_init(int width, int height);

/**
 * @brief DynamicR Scene'i kapatir ve tüm GPU/CPU kaynaklarini (FBO, Context) serbest birakir.
 */
void fe_dynamicr_scene_shutdown(fe_dynamicr_scene_t* scene);

/**
 * @brief Her karede sahne verilerini gunceller (Kamera, Isiklar vb.).
 * * Bu, DynamicR'ın render pass'leri baslamadan once cagrılmalıdır.
 */
void fe_dynamicr_scene_update(fe_dynamicr_scene_t* scene, 
                              const fe_mat4_t* view, 
                              const fe_mat4_t* proj);

#endif // FE_DYNAMICR_SCENE_H