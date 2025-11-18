// include/math/fe_xrcamera3d.h

#ifndef FE_XR_CAMERA3D_H
#define FE_XR_CAMERA3D_H

#include <stdint.h>
#include <stdbool.h>
#include "math/fe_vector.h" 
#include "math/fe_matrix.h" 

// ----------------------------------------------------------------------
// 1. XR GÖZ YAPISI
// ----------------------------------------------------------------------

/**
 * @brief Stereoskopik goruntuleme icin tek bir gozun render verilerini tutar.
 */
typedef struct fe_xr_eye {
    fe_mat4_t view_matrix;       // Gözün gorunum matrisi
    fe_mat4_t projection_matrix; // Gözün projeksiyon matrisi
    fe_mat4_t view_proj_matrix;  // View * Projection
    
    fe_vec3_t offset;            // Merkeze gore goz pozisyonu ofseti (IPD'nin yarisi)
    // TODO: Ileride lens distorsiyon parametreleri eklenebilir.
} fe_xr_eye_t;


// ----------------------------------------------------------------------
// 2. XR KAMERA YAPISI
// ----------------------------------------------------------------------

/**
 * @brief Genisletilmis Gerceklik (XR) icin tasarlanmis kamera.
 * * Kafa pozisyonu (Head Pose) ve iki gozun matrislerini yonetir.
 */
typedef struct fe_xrcamera3d {
    // Kafa Pozu (Kafa Izleme Sisteminden gelen veriler)
    fe_mat4_t head_pose;         // Bas pozisyonu ve rotasyonu (Dunya Uzayinda)
    fe_vec3_t world_position;    // Kafa pozisyonunun dünya uzayindaki konumu (Çıkarılan veri)

    // Optik Ayarlar
    float ipd;                   // Gozler Arasi Mesafe (Interpupillary Distance), metre
    float fov_y;                 // Dikey Görüs Alani (Radyan)
    float aspect_ratio;          // En boy orani (Genislik / Yukseklik)
    float near_plane;
    float far_plane;

    // Göz Verileri (Stereo Veriler)
    fe_xr_eye_t left_eye;
    fe_xr_eye_t right_eye;

} fe_xrcamera3d_t;


// ----------------------------------------------------------------------
// 3. YÖNETİM VE HESAPLAMA FONKSİYONLARI
// ----------------------------------------------------------------------

/**
 * @brief Yeni bir XR kamera yapisini baslatir.
 * * @param ipd Gozler arasi mesafe (metre, genellikle 0.063m civari).
 * @param fov_y Dikey FOV (Radyan).
 * @param aspect_ratio Ekran en boy orani.
 * @return Baslatilmis kamera pointer'i. Basarisiz olursa NULL.
 */
fe_xrcamera3d_t* fe_xrcamera3d_create(float ipd, float fov_y, float aspect_ratio, float near_p, float far_p);

/**
 * @brief Kamera yapisini bellekten serbest birakir.
 */
void fe_xrcamera3d_destroy(fe_xrcamera3d_t* camera);

/**
 * @brief Kamera Kafa Pozunu (Head Pose) gunceller ve goz matrislerini yeniden hesaplar.
 * * Bu, her karede kafa izleme verisi alindiginda çagrilir.
 * * @param camera Kamera baglami.
 * @param new_head_pose Kafa izleme sisteminden gelen en son 4x4 matris.
 */
void fe_xrcamera3d_update(fe_xrcamera3d_t* camera, const fe_mat4_t* new_head_pose);

/**
 * @brief Gözün View ve Projection matrislerini hesaplar.
 * * Bu fonksiyon, fe_xrcamera3d_update icinde çagrilir.
 * * @param camera Kamera baglami.
 */
void fe_xrcamera3d_recalculate_eye_matrices(fe_xrcamera3d_t* camera);


#endif // FE_XR_CAMERA3D_H