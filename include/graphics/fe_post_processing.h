// include/graphics/fe_post_processing.h

#ifndef FE_POST_PROCESSING_H
#define FE_POST_PROCESSING_H

#include <stdint.h>
#include <stdbool.h>
#include "error/fe_error.h"
#include "graphics/fe_render_types.h" // fe_framebuffer_t için

// ----------------------------------------------------------------------
// 1. POST-PROCESSING EFEKT TÜRLERİ
// ----------------------------------------------------------------------

/**
 * @brief Uygulanabilir temel post-processing efektleri.
 * * Her efekt, kendi özel shader programını ve FBO geçişini gerektirir.
 */
typedef enum fe_post_effect_type {
    FE_EFFECT_BLOOM,           // Parlak alanları yayma
    FE_EFFECT_DEPTH_OF_FIELD,  // Alan derinliği
    FE_EFFECT_COLOR_GRADING,   // Renk düzeltme (LUT)
    FE_EFFECT_VIGNETTE,        // Kenar karartma
    FE_EFFECT_FILM_GRAIN,      // Film greni
    FE_EFFECT_COUNT
} fe_post_effect_type_t;

// ----------------------------------------------------------------------
// 2. YÖNETİM FONKSİYONLARI
// ----------------------------------------------------------------------

/**
 * @brief Post-Processing sistemini başlatır.
 * * Ekranı kaplayan çekirdek dörtgeni (Quad) ve temel FBO'lari (Buffer) olusturur.
 * @return Basari durumunda FE_OK.
 */
fe_error_code_t fe_post_processing_init(void);

/**
 * @brief Post-Processing sistemini kapatir ve kaynaklari serbest birakir.
 */
void fe_post_processing_shutdown(void);

/**
 * @brief Bir post-processing efektini boru hattina ekler.
 * * Efektler eklenme sirasina göre uygulanacaktir.
 */
fe_error_code_t fe_post_processing_add_effect(fe_post_effect_type_t effect_type);

/**
 * @brief Post-processing boru hattini calistirir.
 * * Sahne FBO'sunun renk ciktisini alir ve efektleri sirayla uygular.
 * @param scene_color_fbo Render edilmis sahnenin renk tamponu (girdi).
 * @param target_fbo Nihai ciktinin yazilacagi hedef FBO (NULL ise ana ekran).
 */
void fe_post_processing_apply(fe_framebuffer_t* scene_color_fbo, fe_framebuffer_t* target_fbo);

#endif // FE_POST_PROCESSING_H