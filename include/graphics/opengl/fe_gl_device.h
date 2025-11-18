// include/graphics/opengl/fe_gl_device.h

#ifndef FE_GL_DEVICE_H
#define FE_GL_DEVICE_H

#include <stdint.h>
#include "error/fe_error.h"
#include "graphics/fe_render_types.h" // fe_buffer_id_t, fe_texture_id_t vb. için

// ----------------------------------------------------------------------
// 1. BUFFER YÖNETİMİ
// ----------------------------------------------------------------------

/**
 * @brief Yeni bir genel GPU Buffer'ı (VBO, EBO, UBO vb.) olusturur ve baslatir.
 * * @param size Buffer'in bayt cinsinden boyutu.
 * @param data Buffer'a yüklenecek başlangıç verisi (NULL olabilir).
 * @param usage fe_buffer_usage_t (STATIC, DYNAMIC, STREAM).
 * @return Yeni olusturulan Buffer ID'si (fe_buffer_id_t). Basarisiz olursa 0.
 */
fe_buffer_id_t fe_gl_device_create_buffer(size_t size, const void* data, fe_buffer_usage_t usage);

/**
 * @brief Bir Buffer'in verisini gunceller (glBufferSubData).
 */
void fe_gl_device_update_buffer(fe_buffer_id_t buffer_id, size_t offset, size_t size, const void* data);

/**
 * @brief Bir Buffer'i GPU'dan siler.
 */
void fe_gl_device_destroy_buffer(fe_buffer_id_t buffer_id);


// ----------------------------------------------------------------------
// 2. KAPLAMA (Texture) YÖNETİMİ
// ----------------------------------------------------------------------

/**
 * @brief Yeni bir 2D Kaplama (Texture) olusturur.
 * * @param width Kaplamanin genisligi.
 * @param height Kaplamanin yuksekligi.
 * @param format fe_texture_format_t (RGBA8, RGB8, vb.).
 * @param data Kaplamaya yüklenecek piksel verisi (NULL ise bos kaplama).
 * @return Yeni olusturulan Kaplama ID'si (fe_texture_id_t). Basarisiz olursa 0.
 */
fe_texture_id_t fe_gl_device_create_texture2d(int width, int height, fe_texture_format_t format, const void* data);

/**
 * @brief Bir Kaplamayi GPU'dan siler.
 */
void fe_gl_device_destroy_texture(fe_texture_id_t texture_id);


// ----------------------------------------------------------------------
// 3. FRAMEBUFFER YÖNETİMİ
// ----------------------------------------------------------------------

/**
 * @brief Yeni bir Framebuffer Object (FBO) olusturur.
 * * @return Yeni olusturulan FBO ID'si. Basarisiz olursa 0.
 */
fe_buffer_id_t fe_gl_device_create_framebuffer(void);

/**
 * @brief Bir FBO'ya Kaplama baglar (Renk veya Derinlik/Stencil).
 * * @param fbo_id Hedef FBO.
 * @param attachment GL baglanti noktasi (GL_COLOR_ATTACHMENT0, GL_DEPTH_ATTACHMENT vb.).
 * @param texture_id Baglanacak kaplama.
 */
void fe_gl_device_attach_texture_to_fbo(fe_buffer_id_t fbo_id, uint32_t attachment, fe_texture_id_t texture_id);

/**
 * @brief Bir FBO'yu siler.
 */
void fe_gl_device_destroy_framebuffer(fe_buffer_id_t fbo_id);

#endif // FE_GL_DEVICE_H