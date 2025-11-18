// include/graphics/dynamicr/fe_dynamicr_backend.h

#ifndef FE_DYNAMICR_BACKEND_H
#define FE_DYNAMICR_BACKEND_H

#include <stdint.h>
#include <stdbool.h>
#include "error/fe_error.h"
#include "graphics/fe_render_types.h"
#include "graphics/dynamicr/fe_dynamicr_scene.h"
#include "math/fe_matrix.h"

// ----------------------------------------------------------------------
// 1. BACKEND YAŞAM DÖNGÜSÜ
// ----------------------------------------------------------------------

/**
 * @brief DynamicR render backend'ini baslatir (Sahne, FBO'lar ve Alt Sistemler).
 * * @param width Ekran genisligi.
 * @param height Ekran yuksekligi.
 */
fe_error_code_t fe_dynamicr_init(int width, int height);

/**
 * @brief DynamicR render backend'ini kapatir ve kaynaklari serbest birakir.
 */
void fe_dynamicr_shutdown(void);


// ----------------------------------------------------------------------
// 2. KARE YÖNETİMİ VE ÇİZİM
// ----------------------------------------------------------------------

/**
 * @brief DynamicR'in render karesine baslar.
 */
void fe_dynamicr_begin_frame(void);

/**
 * @brief DynamicR'in render karesini sonlandirir.
 */
void fe_dynamicr_end_frame(void);

/**
 * @brief DynamicR boru hattina bir mesh gonderir. 
 * * Bu mesh, G-Buffer gecisinde kullanilacaktir.
 */
void fe_dynamicr_draw_mesh(const fe_mesh_t* mesh, uint32_t instance_count);

/**
 * @brief DynamicR boru hattinin temel gecislerini (G-Buffer, Screen Tracing, Final Compose) calistirir.
 * @param view Kamera View matrisi.
 * @param proj Kamera Projection matrisi.
 */
void fe_dynamicr_execute_passes(const fe_mat4_t* view, const fe_mat4_t* proj);

// ----------------------------------------------------------------------
// 3. FBO YÖNETİMİ (fe_renderer'a baglamak için)
// ----------------------------------------------------------------------

void fe_dynamicr_bind_framebuffer(fe_framebuffer_t* fbo);
void fe_dynamicr_clear_framebuffer(fe_framebuffer_t* fbo, fe_clear_flags_t flags, float r, float g, float b, float a, float depth);


#endif // FE_DYNAMICR_BACKEND_H