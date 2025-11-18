// include/math/fe_camera3d.h

#ifndef FE_CAMERA3D_H
#define FE_CAMERA3D_H

#include <stdint.h>
#include <stdbool.h>
#include "math/fe_vector.h" 
#include "math/fe_matrix.h" 

// ----------------------------------------------------------------------
// 1. KAMERA YAPISI
// ----------------------------------------------------------------------

/**
 * @brief Genel amacli 3D kamera (Perspective).
 */
typedef struct fe_camera3d {
    // Pozisyon ve Yönlendirme (Temel Veriler)
    fe_vec3_t position;      // Kamera'nin dünya uzayindaki pozisyonu
    float yaw;               // Yana açi (yatay dönüs), radyan
    float pitch;             // Eğim açisi (dikey dönüs), radyan

    // Görüs Alani Parametreleri (Projeksiyon)
    float fov_y;             // Dikey Görüs Alani (Field of View), radyan
    float aspect_ratio;      // En boy orani (genislik / yukseklik)
    float near_plane;        // Yakın kesme düzlemi
    float far_plane;         // Uzak kesme düzlemi

    // Hesaplanan Matrisler
    fe_mat4_t view_matrix;       // Dünya koordinatlarindan Görünüm koordinatlarina dönüsüm
    fe_mat4_t projection_matrix; // Görünüm koordinatlarindan Kesme Düzlemi (Clip) koordinatlarina dönüsüm
    fe_mat4_t view_proj_matrix;  // View * Projection
    
    // Kamera Vektörleri (Yön vektörleri, her karede hesaplanir)
    fe_vec3_t forward;       // İleri yön vektörü
    fe_vec3_t right;         // Sağ yön vektörü
    fe_vec3_t up;            // Yukarı yön vektörü
    
    // Hareket Ayarlari
    float move_speed;        // Hareket hizi (birim/saniye)
    float turn_speed;        // Dönüs hizi (radyan/saniye)

} fe_camera3d_t;


// ----------------------------------------------------------------------
// 2. YÖNETİM VE HESAPLAMA FONKSİYONLARI
// ----------------------------------------------------------------------

/**
 * @brief Yeni bir 3D kamera yapisini baslatir ve varsayilan degerleri atar.
 * * @param fov_y Dikey FOV (Radyan).
 * @param aspect_ratio Ekran en boy orani.
 * @param near_p Yakin kesme düzlemi.
 * @param far_p Uzak kesme düzlemi.
 * @return Baslatilmis kamera pointer'i. Basarisiz olursa NULL.
 */
fe_camera3d_t* fe_camera3d_create(float fov_y, float aspect_ratio, float near_p, float far_p);

/**
 * @brief Kamera yapisini bellekten serbest birakir.
 */
void fe_camera3d_destroy(fe_camera3d_t* camera);

/**
 * @brief Kamera yön vektörlerini (forward, right, up) yaw ve pitch acilarina göre gunceller.
 */
void fe_camera3d_update_vectors(fe_camera3d_t* camera);

/**
 * @brief Kamera View ve Projection matrislerini yeniden hesaplar.
 * * fe_camera3d_update_vectors'dan sonra çagrilmalidir.
 */
void fe_camera3d_update_matrices(fe_camera3d_t* camera);

/**
 * @brief Kamerayi verilen dünya pozisyonuna ve yönüne ayarlar.
 */
void fe_camera3d_set_transform(fe_camera3d_t* camera, const fe_vec3_t* position, float yaw, float pitch);

/**
 * @brief Kamerayi verilen yönde hareket ettirir.
 * * @param direction Hareket yonu (ileri/geri, sag/sol, yukari/asagi).
 * @param delta_time Gecen sure (saniye).
 */
void fe_camera3d_move(fe_camera3d_t* camera, const fe_vec3_t* direction, float delta_time);

/**
 * @brief Kamerayi verilen delta açilar kadar döndürür.
 * * @param delta_yaw Yaw açisindaki degisim (radyan).
 * @param delta_pitch Pitch açisindaki degisim (radyan).
 */
void fe_camera3d_rotate(fe_camera3d_t* camera, float delta_yaw, float delta_pitch);

#endif // FE_CAMERA3D_H
