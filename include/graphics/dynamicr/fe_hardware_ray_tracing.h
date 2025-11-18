// include/graphics/dynamicr/fe_hardware_ray_tracing.h

#ifndef FE_HARDWARE_RAY_TRACING_H
#define FE_HARDWARE_RAY_TRACING_H

#include <stdint.h>
#include <stdbool.h>
#include "error/fe_error.h"
#include "graphics/fe_render_types.h" // fe_buffer_id_t, fe_mesh_t, fe_framebuffer_t için
#include "math/fe_matrix.h" // fe_mat4_t için

// ----------------------------------------------------------------------
// 1. HIZLANDIRMA YAPILARI (Acceleration Structure)
// ----------------------------------------------------------------------

/**
 * @brief Bir mesh'in Alt Seviye Hizlandirma Yapisi (BLAS) verisini tutar.
 */
typedef struct fe_blas {
    fe_buffer_id_t blas_buffer_id;     // BLAS'in GPU'daki tampon ID'si (AS verisini tutar)
    uint64_t gpu_handle;               // GPU uzerindeki benzersiz adresi (Dinamik baglama icin)
} fe_blas_t;

/**
 * @brief Sahnedeki tüm BLAS'lari referans gosteren Ust Seviye Hizlandirma Yapisi (TLAS).
 */
typedef struct fe_tlas {
    fe_buffer_id_t tlas_buffer_id;     // TLAS'in GPU'daki tampon ID'si
    uint64_t gpu_handle;               // TLAS'in GPU adresi
    uint32_t instance_count;           // TLAS icindeki mesh orneklerinin sayisi
} fe_tlas_t;


// ----------------------------------------------------------------------
// 2. HRT BAĞLAMI VE YÖNETİM FONKSİYONLARI
// ----------------------------------------------------------------------

/**
 * @brief Donanımsal Işın Takibi (HRT) sisteminin ana baglami.
 */
typedef struct fe_hrt_context {
    fe_tlas_t tlas;
    // ... Ray Tracing Pipelinedeki Shader Binding Table (SBT) bilgileri
} fe_hrt_context_t;

/**
 * @brief Donanımsal Işın Takibi sistemini baslatir ve uzantilari yuklemeye calisir.
 * @return Yeni olusturulan fe_hrt_context_t pointer'i. Basarisiz olursa NULL.
 */
fe_hrt_context_t* fe_hrt_init(void);

/**
 * @brief HRT sistemini kapatir ve tüm AS kaynaklarini serbest birakir.
 */
void fe_hrt_shutdown(fe_hrt_context_t* context);

/**
 * @brief Bir mesh'ten (VBO/EBO) Alt Seviye Hizlandirma Yapisi (BLAS) olusturur.
 */
fe_blas_t fe_hrt_create_blas(const fe_mesh_t* mesh);

/**
 * @brief TLAS'i gunceller/yeniden olusturur. 
 * * Bu, sahnedeki nesnelerin pozisyonlari degistiginde yapilir.
 * @param blas_array Sahnedeki tum BLAS'lar.
 * @param transform_array Her BLAS'a ait model matrisleri.
 * @param count BLAS sayisi.
 */
void fe_hrt_update_tlas(fe_hrt_context_t* context, const fe_blas_t* blas_array, 
                        const fe_mat4_t* transform_array, uint32_t count);

/**
 * @brief Işınları sahneye gonderir (Dispatch Ray).
 * * Bu, nihai isin takibi hesaplamasini baslatir.
 * @param context HRT baglami.
 * @param output_fbo Isin takibi sonucunun yazilacagi hedef FBO.
 * @param width Cizim genisligi.
 * @param height Cizim yuksekligi.
 */
void fe_hrt_dispatch_rays(fe_hrt_context_t* context, fe_framebuffer_t* output_fbo, 
                          uint32_t width, uint32_t height);

#endif // FE_HARDWARE_RAY_TRACING_H