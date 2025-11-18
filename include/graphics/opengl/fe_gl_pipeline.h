// include/graphics/opengl/fe_gl_pipeline.h

#ifndef FE_GL_PIPELINE_H
#define FE_GL_PIPELINE_H

#include <stdint.h>
#include <stdbool.h>

// ----------------------------------------------------------------------
// 1. TEMEL ENUM'LAR (GL Durumunu Temsil Eder)
// ----------------------------------------------------------------------

/**
 * @brief Derinlik Testi Karsilastirma Fonksiyonlari.
 * * GL'deki glDepthFunc() karsiliklaridir.
 */
typedef enum fe_depth_func {
    FE_DEPTH_NEVER,
    FE_DEPTH_LESS,
    FE_DEPTH_EQUAL,
    FE_DEPTH_LEQUAL,
    FE_DEPTH_GREATER,
    FE_DEPTH_NOTEQUAL,
    FE_DEPTH_GEQUAL,
    FE_DEPTH_ALWAYS
} fe_depth_func_t;

/**
 * @brief Yuzey Gizleme (Culling) Modlari.
 * * GL'deki glCullFace() karsiliklaridir.
 */
typedef enum fe_cull_mode {
    FE_CULL_NONE,   // Yuzey Gizlemeyi Devre Disi Birak (glDisable(GL_CULL_FACE))
    FE_CULL_BACK,   // Arka Yuzeyleri Gizle (Varsayilan)
    FE_CULL_FRONT,  // On Yuzeyleri Gizle (Gölge Haritasi olusturmak için kullanilir)
    FE_CULL_FRONT_AND_BACK
} fe_cull_mode_t;

/**
 * @brief Karistirma (Blending) Faktorleri.
 * * GL'deki glBlendFunc() karsiliklaridir.
 */
typedef enum fe_blend_factor {
    FE_BLEND_ZERO,
    FE_BLEND_ONE,
    FE_BLEND_SRC_ALPHA,         // Kaynak Alfa
    FE_BLEND_ONE_MINUS_SRC_ALPHA // 1 - Kaynak Alfa
    // ... daha fazla faktor ileride eklenebilir
} fe_blend_factor_t;

// ----------------------------------------------------------------------
// 2. BORU HATTI YÖNETİM FONKSİYONLARI
// ----------------------------------------------------------------------

/**
 * @brief GL Pipeline yonetim sistemini baslatir (Durum onbelleğini sifirlar).
 */
void fe_gl_pipeline_init(void);

/**
 * @brief Derinlik Testini etkinlestirir veya devre disi birakir.
 */
void fe_gl_pipeline_set_depth_test_enabled(bool enabled);

/**
 * @brief Derinlik Karsilastirma Fonksiyonunu ayarlar.
 */
void fe_gl_pipeline_set_depth_func(fe_depth_func_t func);

/**
 * @brief Derinlik Tamponuna yazmayi etkinlestirir veya devre disi birakir.
 * * @param enabled true: glDepthMask(GL_TRUE), false: glDepthMask(GL_FALSE)
 */
void fe_gl_pipeline_set_depth_write_enabled(bool enabled);

/**
 * @brief Yuzey Gizleme (Culling) modunu ayarlar.
 */
void fe_gl_pipeline_set_cull_mode(fe_cull_mode_t mode);

/**
 * @brief Karistirmayi (Blending) etkinlestirir veya devre disi birakir.
 */
void fe_gl_pipeline_set_blend_enabled(bool enabled);

/**
 * @brief Karistirma (Blending) Faktorlerini ayarlar.
 * * @param src_factor Kaynak (Src) Faktorü.
 * @param dst_factor Hedef (Dst) Faktorü.
 */
void fe_gl_pipeline_set_blend_func(fe_blend_factor_t src_factor, fe_blend_factor_t dst_factor);

// TODO: Wireframe (Tel Kafes) modu, Stencil testi ve Makaslama (Scissor) eklenebilir.

#endif // FE_GL_PIPELINE_H