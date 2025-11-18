// include/graphics/geometryv/fe_gv_illuminator.h

#ifndef FE_GV_ILLUMINATOR_H
#define FE_GV_ILLUMINATOR_H

#include <stdint.h>
#include "error/fe_error.h"
#include "graphics/fe_render_types.h"
#include "graphics/fe_material_editor.h" 
#include "graphics/geometryv/fe_gv_tracer.h" // fe_gv_rt_buffer_t için
#include "math/fe_matrix.h" 

// ----------------------------------------------------------------------
// 1. AYDINLATICI KONTEKS YAPISI
// ----------------------------------------------------------------------

/**
 * @brief GeometryV Işın Aydınlatma (Illuminator) Pass'ini yoneten yapi.
 */
typedef struct fe_gv_illuminator_context {
    fe_material_t* illumination_material; // Işıklandırma işlemini yapan Fragment Shader'ı iceren materyal
    fe_shader_id_t illumination_shader_id; // Shader ID
    // TODO: Gölge haritalari (Shadow Maps) veya Işık bilgileri (UBO) eklenebilir.
} fe_gv_illuminator_context_t;


// ----------------------------------------------------------------------
// 2. YÖNETİM VE ÇİZİM FONKSİYONLARI
// ----------------------------------------------------------------------

/**
 * @brief GeometryV Aydınlatıcı sistemini baslatir (Shader'i yukler).
 * @return Baslatilan baglam (context) pointer'i. Basarisiz olursa NULL.
 */
fe_gv_illuminator_context_t* fe_gv_illuminator_init(void);

/**
 * @brief GeometryV Aydınlatıcı sistemini kapatir ve kaynaklari serbest birakir.
 */
void fe_gv_illuminator_shutdown(fe_gv_illuminator_context_t* context);

/**
 * @brief GeometryV R-Buffer sonuclarini kullanarak nihai sahneyi isiklandirir.
 * * @param context Aydınlatıcı baglami.
 * @param r_buffer fe_gv_tracer'dan gelen ışın takibi isabet verileri (R-Buffer).
 * @param output_fbo Nihai görüntünün yazilacagi hedef FBO. NULL ise ana ekrana yazilir.
 * @param view Kamera View matrisi.
 * @param proj Kamera Projection matrisi.
 */
void fe_gv_illuminator_run(fe_gv_illuminator_context_t* context, 
                           const fe_gv_rt_buffer_t* r_buffer, 
                           fe_framebuffer_t* output_fbo,
                           const fe_mat4_t* view, 
                           const fe_mat4_t* proj);

#endif // FE_GV_ILLUMINATOR_H