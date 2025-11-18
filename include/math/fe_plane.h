// include/math/fe_plane.h

#ifndef FE_PLANE_H
#define FE_PLANE_H

#include <stdint.h>
#include "fe_vector.h" // fe_vec3_t kullanmak için

// ----------------------------------------------------------------------
// 1. DÜZLEM YAPISI (fe_plane_t)
// ----------------------------------------------------------------------

/**
 * @brief 3D Uzayda bir Düzlemi temsil eder.
 * * Denklemi: Ax + By + Cz + D = 0
 */
typedef struct fe_plane {
    fe_vec3_t normal;   // (A, B, C) - Düzleme dik olan birim normal vektör
    float d;            // (D) - Düzlemin orijinden uzaklığı (Normal doğrultusunda)
} fe_plane_t;


// ----------------------------------------------------------------------
// 2. TEMEL İŞLEMLER
// ----------------------------------------------------------------------

/**
 * @brief Normal ve orijine uzaklik d ile bir düzlem olusturur.
 */
fe_plane_t fe_plane_create(fe_vec3_t normal, float d);

/**
 * @brief Üç noktadan (p1, p2, p3) geçen bir düzlem olusturur.
 * * Normali sag el kurali ile hesaplanir.
 */
fe_plane_t fe_plane_from_points(fe_vec3_t p1, fe_vec3_t p2, fe_vec3_t p3);

/**
 * @brief Bir düzlem denklemi için Normali birim vektör yapar ve d'yi günceller.
 * * Zaten birim normal olduğu varsayılır.
 */
fe_plane_t fe_plane_normalize(fe_plane_t p);

/**
 * @brief Bir noktanin düzlem üzerindeki veya hangi tarafinda olduğunu hesaplar.
 * * @param point Kontrol edilecek 3D nokta.
 * @return Noktanin düzlemden isaretli uzakligi (Normalin gösterdigi taraf pozitif).
 */
float fe_plane_distance_to_point(fe_plane_t plane, fe_vec3_t point);


// ----------------------------------------------------------------------
// 3. KONUM SABİTLERİ
// ----------------------------------------------------------------------

// fe_plane_distance_to_point veya benzeri fonksiyonlarin döndürebilecegi tipler
typedef enum fe_plane_side {
    FE_PLANE_FRONT = 1,     // Nokta düzlemin önünde (Normalin gösterdiği yön)
    FE_PLANE_BACK = -1,     // Nokta düzlemin arkasında
    FE_PLANE_ON = 0         // Nokta düzlemin üzerinde
} fe_plane_side_t;

/**
 * @brief Bir noktanin düzleme göre konumunu hesaplar (ön, arka veya üzerinde).
 */
fe_plane_side_t fe_plane_check_point_side(fe_plane_t plane, fe_vec3_t point, float tolerance);

#endif // FE_PLANE_H