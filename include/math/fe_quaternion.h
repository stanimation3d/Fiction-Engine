// include/math/fe_quaternion.h

#ifndef FE_QUATERNION_H
#define FE_QUATERNION_H

#include "fe_vector.h" // fe_vec3_t, fe_vec4_t kullanmak için
#include "fe_matrix.h" // Matris dönüşümleri için

// ----------------------------------------------------------------------
// 1. KUATERNİYON YAPISI (fe_quat_t)
// ----------------------------------------------------------------------

/**
 * @brief Kuaterniyon Yapısı (fe_quat_t).
 * * fe_vec4_t ile ayni bellek duzenine sahiptir: (x, y, z, w).
 * * w skaler kisimdir, (x, y, z) vektör kismidir.
 */
typedef union fe_quaternion {
    struct { float x, y, z, w; };
    fe_vec4_t v4;
    float v[4];
} fe_quat_t;


// ----------------------------------------------------------------------
// 2. SABİT KUATERNİYONLAR
// ----------------------------------------------------------------------

extern const fe_quat_t FE_QUAT_IDENTITY; // Birim Kuaterniyon (Döndürme yok)

// ----------------------------------------------------------------------
// 3. TEMEL İŞLEMLER
// ----------------------------------------------------------------------

/**
 * @brief Yeni bir kuaterniyon olusturur.
 */
fe_quat_t fe_quat_create(float x, float y, float z, float w);

/**
 * @brief İki kuaterniyonu çarpar: Q_out = Q_a * Q_b (Döndürme sirasi: b sonra a).
 */
fe_quat_t fe_quat_multiply(fe_quat_t a, fe_quat_t b);

/**
 * @brief Kuaterniyonun uzunlugunun karesini hesaplar.
 */
float fe_quat_length_sq(fe_quat_t q);

/**
 * @brief Kuaterniyonu normalleştirir (Birim kuaterniyon yapar).
 */
fe_quat_t fe_quat_normalize(fe_quat_t q);

/**
 * @brief Kuaterniyonun eşleniğini (Conjugate) hesaplar.
 */
fe_quat_t fe_quat_conjugate(fe_quat_t q);

/**
 * @brief Kuaterniyonun tersini (Inverse) hesaplar.
 */
fe_quat_t fe_quat_inverse(fe_quat_t q);

// ----------------------------------------------------------------------
// 4. DÖNÜŞÜM İŞLEMLERİ (Rotation)
// ----------------------------------------------------------------------

/**
 * @brief Eksen ve açidan (Radyan) kuaterniyon olusturur.
 */
fe_quat_t fe_quat_from_axis_angle(fe_vec3_t axis, float angle_rad);

/**
 * @brief Bir kuaterniyonu kullanarak 3D vektörü döndürür.
 * * v_out = q * v * q_tersi
 */
fe_vec3_t fe_quat_rotate_vec3(fe_quat_t q, fe_vec3_t v);

/**
 * @brief Kuaterniyonu 4x4 rotasyon matrisine dönüstürür.
 */
fe_mat4_t fe_quat_to_mat4(fe_quat_t q);

// ----------------------------------------------------------------------
// 5. ENTERPOLASYON
// ----------------------------------------------------------------------

/**
 * @brief Küresel Doğrusal Enterpolasyon (Spherical Linear Interpolation - SLERP)
 * * İki döndürme arasinda en kisa ve sabit hizli yolu sağlar.
 * @param start Baslangiç kuaterniyonu.
 * @param end Bitiş kuaterniyonu.
 * @param t Enterpolasyon faktörü (0.0 - 1.0).
 */
fe_quat_t fe_quat_slerp(fe_quat_t start, fe_quat_t end, float t);

#endif // FE_QUATERNION_H