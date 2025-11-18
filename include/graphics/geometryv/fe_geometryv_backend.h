// include/graphics/geometryv/fe_geometryv_backend.h

#ifndef FE_GEOMETRYV_BACKEND_H
#define FE_GEOMETRYV_BACKEND_H

#include <stdint.h>
#include <stdbool.h>
#include "error/fe_error.h"
#include "graphics/fe_render_types.h"
#include "math/fe_matrix.h"

// ----------------------------------------------------------------------
// 1. BACKEND YAŞAM DÖNGÜSÜ
// ----------------------------------------------------------------------

/**
 * @brief GeometryV render backend'ini baslatir (Scene, Tracer ve Illuminator).
 * * @param width Ekran genisligi.
 * @param height Ekran yuksekligi.
 */
fe_error_code_t fe_geometryv_init(int width, int height);

/**
 * @brief GeometryV render backend'ini kapatir ve kaynaklari serbest birakir.
 */
void fe_geometryv_shutdown(void);


// ----------------------------------------------------------------------
// 2. SAHNE YÖNETİMİ VE KARE ÇİZİMİ
// ----------------------------------------------------------------------

/**
 * @brief Yeni mesh'leri GeometryV sahnesine yukler ve hiyerarsiyi insa eder.
 * * Bu, genellikle sahne yuklemesi sirasinda bir kez yapilir.
 * * @param meshes Yuklenecek mesh listesi.
 * @param mesh_count Mesh sayisi.
 */
void fe_geometryv_load_scene_geometry(const fe_mesh_t* const* meshes, uint32_t mesh_count);

/**
 * @brief GeometryV boru hattinin temel gecislerini (Ray Tracing, Illumination) calistirir.
 * @param view Kamera View matrisi.
 * @param proj Kamera Projection matrisi.
 */
void fe_geometryv_execute_passes(const fe_mat4_t* view, const fe_mat4_t* proj);

/**
 * @brief GeometryV'nin render karesine baslar (Şu an için basit bir yer tutucu).
 */
void fe_geometryv_begin_frame(void);

/**
 * @brief GeometryV'nin render karesini sonlandirir (Şu an için basit bir yer tutucu).
 */
void fe_geometryv_end_frame(void);

/**
 * @brief GeometryV, G-Buffer kullanmadigi icin bu fonksiyonda islem yapilmaz.
 */
void fe_geometryv_draw_mesh(const fe_mesh_t* mesh, uint32_t instance_count);


// ----------------------------------------------------------------------
// 3. FBO YÖNETİMİ
// ----------------------------------------------------------------------

void fe_geometryv_bind_framebuffer(fe_framebuffer_t* fbo);
void fe_geometryv_clear_framebuffer(fe_framebuffer_t* fbo, fe_clear_flags_t flags, float r, float g, float b, float a, float depth);


#endif // FE_GEOMETRYV_BACKEND_H
