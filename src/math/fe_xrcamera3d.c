// src/math/fe_xrcamera3d.c

#include "math/fe_xrcamera3d.h"
#include "math/fe_vector.h"
#include "math/fe_matrix.h"
#include "utils/fe_logger.h"
#include <stdlib.h> // malloc, free için
#include <string.h> // memcpy için
#include <math.h>   // tanf için

// ----------------------------------------------------------------------
// 1. DAHİLİ YARDIMCI FONKSİYONLAR
// ----------------------------------------------------------------------

/**
 * @brief Perspektif Projeksiyon Matrisini hesaplar.
 * * (fe_mat4_perspective'den alinmistir, ancak stereo icin gereklidir)
 */
static fe_mat4_t fe_mat4_perspective_stereo(float fov_y, float aspect, float n, float f) {
    fe_mat4_t result = FE_MAT4_IDENTITY;
    float tan_half_fov = tanf(fov_y / 2.0f);
    
    // Projeksiyon Matrisi Hesaplaması
    // m[0][0] = 1.0f / (aspect * tan_half_fov)
    // m[1][1] = 1.0f / tan_half_fov
    // m[2][2] = -(f + n) / (f - n)
    // m[2][3] = -1.0f
    // m[3][2] = -(2.0f * f * n) / (f - n)

    result.m[0][0] = 1.0f / (aspect * tan_half_fov);
    result.m[1][1] = 1.0f / tan_half_fov;
    result.m[2][2] = -(f + n) / (f - n);
    result.m[2][3] = -1.0f;
    result.m[3][2] = -(2.0f * f * n) / (f - n);
    result.m[3][3] = 0.0f;
    
    return result;
}

// ----------------------------------------------------------------------
// 2. XR KAMERA YÖNETİMİ
// ----------------------------------------------------------------------

/**
 * Uygulama: fe_xrcamera3d_create
 */
fe_xrcamera3d_t* fe_xrcamera3d_create(float ipd, float fov_y, float aspect_ratio, float near_p, float far_p) {
    fe_xrcamera3d_t* camera = (fe_xrcamera3d_t*)calloc(1, sizeof(fe_xrcamera3d_t));
    if (!camera) {
        FE_LOG_FATAL("XR Kamera yapisi icin bellek ayrilamadi.");
        return NULL;
    }

    // Optik Ayarlar
    camera->ipd = ipd;
    camera->fov_y = fov_y;
    camera->aspect_ratio = aspect_ratio;
    camera->near_plane = near_p;
    camera->far_plane = far_p;
    
    // Varsayılan kafa pozu (Dünya orijininde)
    camera->head_pose = FE_MAT4_IDENTITY;
    camera->world_position = (fe_vec3_t){0.0f, 0.0f, 0.0f};

    // Göz Ofsetleri (Yarım IPD, X ekseninde)
    float half_ipd = ipd / 2.0f;
    camera->left_eye.offset = (fe_vec3_t){-half_ipd, 0.0f, 0.0f};
    camera->right_eye.offset = (fe_vec3_t){half_ipd, 0.0f, 0.0f};

    // İlk matrisleri hesapla (Gereklidir)
    fe_xrcamera3d_recalculate_eye_matrices(camera);
    
    FE_LOG_DEBUG("XR Kamera olusturuldu. IPD: %.4f m, FOV: %.2f rad", ipd, fov_y);
    return camera;
}

/**
 * Uygulama: fe_xrcamera3d_destroy
 */
void fe_xrcamera3d_destroy(fe_xrcamera3d_t* camera) {
    if (camera) {
        free(camera);
        FE_LOG_DEBUG("XR Kamera yok edildi.");
    }
}

/**
 * Uygulama: fe_xrcamera3d_recalculate_eye_matrices
 */
void fe_xrcamera3d_recalculate_eye_matrices(fe_xrcamera3d_t* camera) {
    // 1. Projection Matrisi Hesaplaması (Stereo için aynı varsayılır)
    fe_mat4_t projection = fe_mat4_perspective_stereo(
        camera->fov_y, 
        camera->aspect_ratio, 
        camera->near_plane, 
        camera->far_plane
    );

    // Göz pozisyon ofsetleri (IPD)
    float half_ipd = camera->ipd / 2.0f;
    
    // 2. Göz View Matrislerini Hesapla
    
    // Sol Göz
    // Önce kafa pozunu al, sonra yerel X (IPD) ofsetini ekle
    fe_mat4_t left_eye_translation = fe_mat4_translate(camera->left_eye.offset); // (-IPD/2, 0, 0)
    fe_mat4_t left_view_transform = fe_mat4_multiply(camera->head_pose, left_eye_translation); 
    
    // View matrisi, dünya pozundan ters çevrilmiş halidir
    camera->left_eye.view_matrix = fe_mat4_inverse(left_view_transform);
    
    // Sol ViewProjection
    camera->left_eye.projection_matrix = projection; // Stereo projeksiyon
    camera->left_eye.view_proj_matrix = fe_mat4_multiply(projection, camera->left_eye.view_matrix);
    
    // Sağ Göz
    fe_mat4_t right_eye_translation = fe_mat4_translate(camera->right_eye.offset); // (+IPD/2, 0, 0)
    fe_mat4_t right_view_transform = fe_mat4_multiply(camera->head_pose, right_eye_translation);
    
    // View matrisi
    camera->right_eye.view_matrix = fe_mat4_inverse(right_view_transform);
    
    // Sağ ViewProjection
    camera->right_eye.projection_matrix = projection; // Stereo projeksiyon
    camera->right_eye.view_proj_matrix = fe_mat4_multiply(projection, camera->right_eye.view_matrix);
    
    // 
}

/**
 * Uygulama: fe_xrcamera3d_update
 */
void fe_xrcamera3d_update(fe_xrcamera3d_t* camera, const fe_mat4_t* new_head_pose) {
    // 1. Kafa Pozunu Güncelle
    memcpy(&camera->head_pose, new_head_pose, sizeof(fe_mat4_t));
    
    // 2. Dünya Pozisyonunu Çıkar (Matrisin 4. sütununun ilk 3 bileşeni)
    camera->world_position.x = new_head_pose->m[3][0];
    camera->world_position.y = new_head_pose->m[3][1];
    camera->world_position.z = new_head_pose->m[3][2];
    
    // 3. Göz Matrislerini Yeniden Hesapla
    fe_xrcamera3d_recalculate_eye_matrices(camera);
}