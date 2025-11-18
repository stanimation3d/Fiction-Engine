// include/graphics/dynamicr/fe_screen_tracing.h

#ifndef FE_SCREEN_TRACING_H
#define FE_SCREEN_TRACING_H

#include <stdint.h>
#include <stdbool.h>
#include "error/fe_error.h"
#include "graphics/fe_render_types.h"
#include "graphics/fe_material_editor.h" 

// ----------------------------------------------------------------------
// 1. YAPILAR VE KAYNAKLAR
// ----------------------------------------------------------------------

/**
 * @brief Ekran Alanı Işın Takibi (Screen Tracing) Pass'ini yöneten yapi.
 */
typedef struct fe_screen_tracing_context {
    fe_material_t* tracing_material; // Ray Tracing shader'ını iceren materyal
    fe_framebuffer_t* output_fbo;    // Takip sonucunun yazilacagi FBO (Dolayli Aydınlatma / Yansima Kaplamasi)
    fe_shader_id_t screen_trace_shader_id; // Shader ID
    // TODO: Ilgili Uniform Buffer Object'ler (UBO) buraya eklenebilir.
} fe_screen_tracing_context_t;


// ----------------------------------------------------------------------
// 2. YÖNETİM VE ÇİZİM FONKSİYONLARI
// ----------------------------------------------------------------------

/**
 * @brief Screen Tracing sistemini baslatir (Shader'i yukler ve FBO'yu ayarlar).
 * * @param width Ekran genisligi.
 * @param height Ekran yuksekligi.
 * @return Baslatilan baglam (context) pointer'i. Basarisiz olursa NULL.
 */
fe_screen_tracing_context_t* fe_screen_tracing_init(int width, int height);

/**
 * @brief Screen Tracing sistemini kapatir ve kaynaklari serbest birakir.
 */
void fe_screen_tracing_shutdown(fe_screen_tracing_context_t* context);

/**
 * @brief Ekran Alanı Işın Takibi Pass'ini calistirir.
 * * G-Buffer'dan Derinlik ve Normal verilerini kullanir.
 * @param context Screen Tracing baglami.
 * @param gbuffer_fbo Girdi olarak kullanilacak G-Buffer (Derinlik/Normal Kaplamalarini iceren).
 */
void fe_screen_tracing_run(fe_screen_tracing_context_t* context, const fe_framebuffer_t* gbuffer_fbo);

#endif // FE_SCREEN_TRACING_H