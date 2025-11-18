// include/graphics/opengl/fe_gl_backend.h

#ifndef FE_GL_BACKEND_H
#define FE_GL_BACKEND_H

#include "error/fe_error.h"
#include "graphics/fe_render_types.h" // fe_mesh_t, fe_framebuffer_t vb. için
#include "graphics/opengl/fe_gl_mesh.h" // fe_gl_mesh_t yapilari icin
#include <stdint.h>
#include <stdbool.h>

// ----------------------------------------------------------------------
// 1. BACKEND YAŞAM DÖNGÜSÜ
// ----------------------------------------------------------------------

/**
 * @brief OpenGL render backend'ini baslatir (Context, Pipeline ve Diger Alt Moduller).
 */
fe_error_code_t fe_gl_init(void);

/**
 * @brief OpenGL render backend'ini kapatir ve kaynaklari serbest birakir.
 */
void fe_gl_shutdown(void);


// ----------------------------------------------------------------------
// 2. KARE YÖNETİMİ
// ----------------------------------------------------------------------

/**
 * @brief Yeni bir cizim karesine baslar.
 * * Gerekli GL durumunu ayarlar.
 */
void fe_gl_begin_frame(void);

/**
 * @brief Cizim karesini sonlandirir ve tamponlari gosterir (Swap Buffers).
 */
void fe_gl_end_frame(void);


// ----------------------------------------------------------------------
// 3. ÇİZİM İŞLEMLERİ (Mesh, FBO)
// ----------------------------------------------------------------------

/**
 * @brief Belirtilen mesh'i aktif pipeline durumunu kullanarak cizer.
 * @param mesh Cizilecek mesh yapisi.
 * @param instance_count Kac adet ornek cizilecegi. (1 ise normal cizim).
 */
void fe_gl_draw_mesh(const fe_mesh_t* mesh, uint32_t instance_count);

/**
 * @brief Belirtilen Framebuffer'i temizler.
 * * fe_render_pass.c'deki temizleme islemini uygular.
 */
void fe_gl_clear_framebuffer(fe_framebuffer_t* fbo, fe_clear_flags_t flags, float r, float g, float b, float a, float depth);

/**
 * @brief Framebuffer'i baglar veya baglantisini cozer (NULL ise ana ekran).
 */
void fe_gl_bind_framebuffer(fe_framebuffer_t* fbo);

#endif // FE_GL_BACKEND_H