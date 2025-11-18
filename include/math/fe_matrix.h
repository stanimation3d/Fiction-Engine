// include/math/fe_matrix.h

#ifndef FE_MATRIX_H
#define FE_MATRIX_H

#include "fe_vector.h" // fe_vec3_t, fe_vec4_t kullanmak için

// ----------------------------------------------------------------------
// 1. MATRİS YAPILARI (fe_mat4_t)
// ----------------------------------------------------------------------

/**
 * @brief 4x4 Matris Yapısı (fe_mat4_t).
 * * C dilinde Matrisler genellikle Sütun-Major sirayla (Column-Major) depolanir.
 * * Matrisin (satır, sütun) elemanına erişim için m[sütun_indeksi][satır_indeksi] kullanılır.
 */
typedef union fe_mat4 {
    float m[16];        // Düz 16 elemanlı dizi (GPU'ya göndermek için ideal)
    float mm[4][4];     // Dizi dizisi (Erişim kolaylığı için)
    fe_vec4_t col[4];   // 4 adet 4D vektör (Sütunlar)
} fe_mat4_t;


// ----------------------------------------------------------------------
// 2. SABİT MATRİSLER
// ----------------------------------------------------------------------

extern const fe_mat4_t FE_MAT4_IDENTITY; // Birim Matris

// ----------------------------------------------------------------------
// 3. TEMEL İŞLEMLER
// ----------------------------------------------------------------------

/**
 * @brief Yeni bir 4x4 matris olusturur ve Birim Matris (Identity) yapar.
 */
fe_mat4_t fe_mat4_identity(void);

/**
 * @brief Iki matrisi çarpar: C = A * B.
 */
fe_mat4_t fe_mat4_multiply(fe_mat4_t a, fe_mat4_t b);

/**
 * @brief Matris ile 4D vektörü çarpar: v_out = M * v_in.
 */
fe_vec4_t fe_mat4_multiply_vec4(fe_mat4_t m, fe_vec4_t v);

/**
 * @brief Matris ile 3D vektörü çarpar (w=1.0 varsayılarak).
 * * Dönüşüm (Translation/Rotation/Scale) uygulamak için kullanılır.
 */
fe_vec3_t fe_mat4_multiply_vec3(fe_mat4_t m, fe_vec3_t v);

/**
 * @brief Matrisin tersini (Inverse) hesaplar.
 */
fe_mat4_t fe_mat4_inverse(fe_mat4_t m);

/**
 * @brief Matrisin transpozesini (Transpose) hesaplar.
 */
fe_mat4_t fe_mat4_transpose(fe_mat4_t m);


// ----------------------------------------------------------------------
// 4. DÖNÜŞÜM (TRANSFORM) FONKSİYONLARI
// ----------------------------------------------------------------------

/**
 * @brief Öteleme (Translation) matrisi olusturur.
 */
fe_mat4_t fe_mat4_translate(fe_vec3_t v);

/**
 * @brief Ölçeklendirme (Scale) matrisi olusturur.
 */
fe_mat4_t fe_mat4_scale(fe_vec3_t v);

/**
 * @brief Belirtilen eksen etrafinda döndürme (Rotation) matrisi olusturur.
 * * @param angle Radyan cinsinden açi.
 * @param axis Eksen (fe_vec3_t).
 */
fe_mat4_t fe_mat4_rotate(float angle, fe_vec3_t axis);


// ----------------------------------------------------------------------
// 5. PROJEKSİYON FONKSİYONLARI
// ----------------------------------------------------------------------

/**
 * @brief Perspektif Projeksiyon Matrisi olusturur (Frustum).
 * * @param fov_y Dikey görüș açısı (Radyan).
 * @param aspect En-Boy oranı (width/height).
 * @param near Yakın kesme düzlemi.
 * @param far Uzak kesme düzlemi.
 */
fe_mat4_t fe_mat4_perspective(float fov_y, float aspect, float near, float far);

/**
 * @brief Ortografik Projeksiyon Matrisi olusturur (2D veya sabit derinlikli 3D).
 */
fe_mat4_t fe_mat4_ortho(float left, float right, float bottom, float top, float near, float far);

/**
 * @brief Görünüm Matrisi (View Matrix) olusturur (Kamera dönüşümü).
 * * @param eye Kameranin konumu.
 * @param center Kameranin baktigi nokta.
 * @param up Kameranin yukari vektörü.
 */
fe_mat4_t fe_mat4_look_at(fe_vec3_t eye, fe_vec3_t center, fe_vec3_t up);


#endif // FE_MATRIX_H