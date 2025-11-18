// include/graphics/fe_renderer.h

#ifndef FE_RENDERER_H
#define FE_RENDERER_H

#include <stdint.h>
#include <stdbool.h>
#include "error/fe_error.h"
#include "graphics/fe_render_types.h"
#include "math/fe_matrix.h"

// Dinamik olarak yüklenen backend arayüzleri
#include "graphics/dynamicr/fe_dynamicr_backend.h" 
#include "graphics/geometryv/fe_geometryv_backend.h"

// ----------------------------------------------------------------------
// 1. RENDER BACKEND SEÇİMİ
// ----------------------------------------------------------------------

/**
 * @brief Mevcut render backend tipleri.
 */
typedef enum fe_render_backend_type {
    FE_BACKEND_NONE = 0,
    FE_BACKEND_OPENGL,        // Geleneksel OpenGL (Varsayılan/Yedek)
    FE_BACKEND_DYNAMICR,      // Hibrit Işın Takibi Backend (DynamicR)
    FE_BACKEND_GEOMETRYV,     // Cluster-tabanlı Işın Takibi Backend (GeometryV)
    FE_BACKEND_COUNT
} fe_render_backend_type_t;


// ----------------------------------------------------------------------
// 2. RENDER YAŞAM DÖNGÜSÜ
// ----------------------------------------------------------------------

/**
 * @brief Render sistemini baslatir ve belirtilen backend'i ayarlar.
 * * @param width Ekran genisligi.
 * @param height Ekran yuksekligi.
 * @param backend_type Kullanilacak render backend tipi.
 */
fe_error_code_t fe_renderer_init(int width, int height, fe_render_backend_type_t backend_type);

/**
 * @brief Render sistemini ve aktif backend'i kapatir.
 */
void fe_renderer_shutdown(void);


// ----------------------------------------------------------------------
// 3. KARE VE ÇİZİM YÖNETİMİ
// ----------------------------------------------------------------------

/**
 * @brief Yeni bir render karesine baslar.
 */
void fe_renderer_begin_frame(void);

/**
 * @brief Render karesini bitirir ve sonuçlari ekrana sunar (swap buffers).
 */
void fe_renderer_end_frame(void);

/**
 * @brief Bir mesh'i aktif render backend'ine gonderir.
 * * @param mesh Cizilecek mesh.
 * @param instance_count Ornek sayisi.
 */
void fe_renderer_draw_mesh(const fe_mesh_t* mesh, uint32_t instance_count);

/**
 * @brief Aktif backend'in render pass'lerini calistirir (Örn: G-Buffer, Ray Tracing, Illumination).
 * @param view Kamera View matrisi.
 * @param proj Kamera Projection matrisi.
 */
void fe_renderer_execute_passes(const fe_mat4_t* view, const fe_mat4_t* proj);


// ----------------------------------------------------------------------
// 4. DURUM VE FBO YÖNETİMİ
// ----------------------------------------------------------------------

/**
 * @brief Belirtilen FBO'yu baglar veya NULL ise varsayilan FBO'yu (ana ekran) baglar.
 */
void fe_renderer_bind_framebuffer(fe_framebuffer_t* fbo);

/**
 * @brief Bagli olan FBO'yu temizler.
 */
void fe_renderer_clear(fe_clear_flags_t flags, float r, float g, float b, float a, float depth);

/**
 * @brief Sahne geometrisini statik olarak backend'e yukler (Örn: GeometryV için).
 */
void fe_renderer_load_scene_geometry(const fe_mesh_t* const* meshes, uint32_t mesh_count);


#endif // FE_RENDERER_H