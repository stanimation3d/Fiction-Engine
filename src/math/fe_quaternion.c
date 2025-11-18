// src/math/fe_quaternion.c

#include "math/fe_quaternion.h"
#include <math.h>   // sinf, cosf, sqrtf
#include <float.h>  // FLT_EPSILON

// ----------------------------------------------------------------------
// 1. SABİT KUATERNİYON TANIMLAMALARI
// ----------------------------------------------------------------------

// Birim Kuaterniyon (Identity): Dönüşüm yok (0, 0, 0, 1)
const fe_quat_t FE_QUAT_IDENTITY = { .x = 0.0f, .y = 0.0f, .z = 0.0f, .w = 1.0f };

// ----------------------------------------------------------------------
// 2. TEMEL İŞLEMLER UYGULAMALARI
// ----------------------------------------------------------------------

/**
 * Uygulama: fe_quat_create
 */
fe_quat_t fe_quat_create(float x, float y, float z, float w) {
    return (fe_quat_t){ .x = x, .y = y, .z = z, .w = w };
}

/**
 * Uygulama: fe_quat_multiply (Hamilton Çarpımı)
 * Q_out = Q_a * Q_b
 */
fe_quat_t fe_quat_multiply(fe_quat_t a, fe_quat_t b) {
    // Vektör kısmı (v): a.w*b.v + b.w*a.v + a.v x b.v
    // Skaler kısım (w): a.w*b.w - a.v . b.v
    
    fe_quat_t result;
    
    // w (Skaler)
    result.w = a.w * b.w - (a.x * b.x + a.y * b.y + a.z * b.z);
    
    // x (Vektör)
    result.x = a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y;
    
    // y (Vektör)
    result.y = a.w * b.y - a.x * b.z + a.y * b.w + a.z * b.x;
    
    // z (Vektör)
    result.z = a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w;
    
    return result;
}

/**
 * Uygulama: fe_quat_length_sq
 */
float fe_quat_length_sq(fe_quat_t q) {
    return q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w;
}

/**
 * Uygulama: fe_quat_normalize
 */
fe_quat_t fe_quat_normalize(fe_quat_t q) {
    float len_sq = fe_quat_length_sq(q);
    
    if (len_sq < FLT_EPSILON) {
        return FE_QUAT_IDENTITY; 
    }
    
    float inv_len = 1.0f / sqrtf(len_sq);
    return fe_quat_create(q.x * inv_len, q.y * inv_len, q.z * inv_len, q.w * inv_len);
}

/**
 * Uygulama: fe_quat_conjugate (Eşlenik)
 */
fe_quat_t fe_quat_conjugate(fe_quat_t q) {
    // Eşlenik: Vektör kısmı işaret değiştirir, skaler kısmı aynı kalır.
    return fe_quat_create(-q.x, -q.y, -q.z, q.w);
}

/**
 * Uygulama: fe_quat_inverse (Ters)
 */
fe_quat_t fe_quat_inverse(fe_quat_t q) {
    // Q_inv = Q_konj / |Q|^2
    float len_sq = fe_quat_length_sq(q);
    
    if (len_sq < FLT_EPSILON) {
        return FE_QUAT_IDENTITY; 
    }
    
    float inv_len_sq = 1.0f / len_sq;
    fe_quat_t conj = fe_quat_conjugate(q);
    
    return fe_quat_create(conj.x * inv_len_sq, conj.y * inv_len_sq, conj.z * inv_len_sq, conj.w * inv_len_sq);
}


// ----------------------------------------------------------------------
// 3. DÖNÜŞÜM İŞLEMLERİ UYGULAMALARI
// ----------------------------------------------------------------------

/**
 * Uygulama: fe_quat_from_axis_angle
 */
fe_quat_t fe_quat_from_axis_angle(fe_vec3_t axis, float angle_rad) {
    fe_quat_t result;
    
    // Ekseni normalleştir
    fe_vec3_t norm_axis = fe_vec3_normalize(axis);
    
    float half_angle = angle_rad * 0.5f;
    float s = sinf(half_angle);
    
    result.x = norm_axis.x * s;
    result.y = norm_axis.y * s;
    result.z = norm_axis.z * s;
    result.w = cosf(half_angle);
    
    return result;
}

/**
 * Uygulama: fe_quat_rotate_vec3
 */
fe_vec3_t fe_quat_rotate_vec3(fe_quat_t q, fe_vec3_t v) {
    // v'yi saf kuaterniyona çevir: p = (v.x, v.y, v.z, 0.0)
    fe_quat_t p = { .x = v.x, .y = v.y, .z = v.z, .w = 0.0f };
    
    // Rotasyon: v' = q * p * q_inv
    fe_quat_t q_inv = fe_quat_inverse(q);
    fe_quat_t temp = fe_quat_multiply(q, p);
    fe_quat_t rotated = fe_quat_multiply(temp, q_inv);
    
    // Sonucun vektör kısmını al
    return (fe_vec3_t){ .x = rotated.x, .y = rotated.y, .z = rotated.z };
}

/**
 * Uygulama: fe_quat_to_mat4
 */
fe_mat4_t fe_quat_to_mat4(fe_quat_t q) {
    // Kuaterniyonun normalleştirildiği varsayılır.
    float x2 = q.x * q.x;
    float y2 = q.y * q.y;
    float z2 = q.z * q.z;
    float xy = q.x * q.y;
    float xz = q.x * q.z;
    float xw = q.x * q.w;
    float yz = q.y * q.z;
    float yw = q.y * q.w;
    float zw = q.z * q.w;

    fe_mat4_t m;
    
    // Sütun-Major Siralama (OpenGL/Vulkan konvansiyonu)
    
    // Sütun 0
    m.m[0] = 1.0f - 2.0f * (y2 + z2);
    m.m[1] = 2.0f * (xy + zw);
    m.m[2] = 2.0f * (xz - yw);
    m.m[3] = 0.0f;

    // Sütun 1
    m.m[4] = 2.0f * (xy - zw);
    m.m[5] = 1.0f - 2.0f * (x2 + z2);
    m.m[6] = 2.0f * (yz + xw);
    m.m[7] = 0.0f;

    // Sütun 2
    m.m[8] = 2.0f * (xz + yw);
    m.m[9] = 2.0f * (yz - xw);
    m.m[10] = 1.0f - 2.0f * (x2 + y2);
    m.m[11] = 0.0f;

    // Sütun 3
    m.m[12] = 0.0f;
    m.m[13] = 0.0f;
    m.m[14] = 0.0f;
    m.m[15] = 1.0f;

    return m;
}


// ----------------------------------------------------------------------
// 4. ENTERPOLASYON UYGULAMALARI
// ----------------------------------------------------------------------

/**
 * Uygulama: fe_quat_slerp
 */
fe_quat_t fe_quat_slerp(fe_quat_t start, fe_quat_t end, float t) {
    // Başlangıç ve bitişin normalleştirildiği varsayılır.
    float dot = start.x * end.x + start.y * end.y + start.z * end.z + start.w * end.w;
    
    // Eğer nokta çarpım negatifse, ters yönde slerp yaparak daha kısa yolu bul
    if (dot < 0.0f) {
        end.x = -end.x;
        end.y = -end.y;
        end.z = -end.z;
        end.w = -end.w;
        dot = -dot;
    }

    if (dot > 0.9995f) {
        // Kuaterniyonlar çok yakın, Linear Interpolation (Lerp) kullan.
        fe_vec4_t lerp_v = fe_vec4_lerp(start.v4, end.v4, t);
        fe_quat_t lerp_q = { .v4 = lerp_v };
        return fe_quat_normalize(lerp_q);
    }

    float theta_0 = acosf(dot);
    float theta = theta_0 * t;
    float sin_theta = sinf(theta);
    float sin_theta_0 = sinf(theta_0);

    float scale_s = cosf(theta) - dot * sin_theta / sin_theta_0;
    float scale_e = sin_theta / sin_theta_0;
    
    // Q_slerp = scale_s * start + scale_e * end
    fe_quat_t result;
    result.x = scale_s * start.x + scale_e * end.x;
    result.y = scale_s * start.y + scale_e * end.y;
    result.z = scale_s * start.z + scale_e * end.z;
    result.w = scale_s * start.w + scale_e * end.w;
    
    return result;
}