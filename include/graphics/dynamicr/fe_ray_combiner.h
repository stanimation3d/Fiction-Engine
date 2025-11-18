// include/graphics/dynamicr/fe_ray_combiner.h

#ifndef FE_RAY_COMBINER_H
#define FE_RAY_COMBINER_H

#include <stdint.h>
#include "error/fe_error.h"
#include "graphics/fe_render_types.h"
#include "graphics/fe_material_editor.h" 
#include "math/fe_matrix.h" 

// ----------------------------------------------------------------------
// 1. RAY COMBINER KONTEKS YAPISI
// ----------------------------------------------------------------------

/**
 * @brief Işın Birleştirici (Ray Combiner) Pass'ini yoneten yapi.
 */
typedef struct fe_ray_combiner_context {
    fe_material_t* combine_material;  // Birleştirme işlemini yapan Pixel Shader'ı iceren materyal
    fe_shader_id_t combine_shader_id; // Shader ID
    // TODO: Ileride Post-processing FBO'lari eklenebilir.
} fe_ray_combiner_context_t;


// ----------------------------------------------------------------------
// 2. YÖNETİM VE ÇİZİM FONKSİYONLARI
// ----------------------------------------------------------------------

/**
 * @brief Ray Combiner sistemini baslatir (Shader'i yukler).
 * @return Baslatilan baglam (context) pointer'i. Basarisiz olursa NULL.
 */
fe_ray_combiner_context_t* fe_ray_combiner_init(void);

/**
 * @brief Ray Combiner sistemini kapatir ve kaynaklari serbest birakir.
 */
void fe_ray_combiner_shutdown(fe_ray_combiner_context_t* context);

/**
 * @brief DynamicR boru hattindaki tum aydinlatma sonuclarini birlestirir.
 * * @param context Ray Combiner baglami.
 * @param gbuffer_fbo G-Buffer'dan gelen geometrik veri (Normal, Albedo, Depth).
 * @param screen_trace_output Screen Tracing'den gelen ekran alani dolayli isigi.
 * @param voxel_radiance_volume Voxel GI sisteminden gelen 3D Volume (Sahne disi dolayli isik).
 * @param view Kamera View matrisi.
 * @param proj Kamera Projection matrisi.
 */
void fe_ray_combiner_run(fe_ray_combiner_context_t* context, 
                         const fe_framebuffer_t* gbuffer_fbo, 
                         const fe_framebuffer_t* screen_trace_output,
                         fe_texture_id_t voxel_radiance_volume,
                         const fe_mat4_t* view, 
                         const fe_mat4_t* proj);

#endif // FE_RAY_COMBINER_H