// src/math/fe_plane.c

#include "math/fe_plane.h"
#include <math.h>   // fabsf, sqrtf
#include <float.h>  // FLT_EPSILON

// ----------------------------------------------------------------------
// 1. TEMEL İŞLEMLER UYGULAMALARI
// ----------------------------------------------------------------------

/**
 * Uygulama: fe_plane_create
 */
fe_plane_t fe_plane_create(fe_vec3_t normal, float d) {
    // Normalin zaten birim vektör olduğu varsayılır, değilse fe_plane_normalize çağrılmalıdır.
    return (fe_plane_t){ .normal = normal, .d = d };
}

/**
 * Uygulama: fe_plane_from_points
 */
fe_plane_t fe_plane_from_points(fe_vec3_t p1, fe_vec3_t p2, fe_vec3_t p3) {
    // 1. İki kenar vektörünü bul
    fe_vec3_t edge1 = fe_vec3_subtract(p2, p1);
    fe_vec3_t edge2 = fe_vec3_subtract(p3, p1);

    // 2. Normal vektörünü hesapla (Çapraz çarpım)
    fe_vec3_t normal = fe_vec3_cross(edge1, edge2);
    
    // 3. Normali birim vektör yap (Düzlemi normalleştir)
    float len = fe_vec3_length(normal);
    
    if (len < FLT_EPSILON) {
        // Noktalar doğrusal (collinear), düzlem oluşturulamaz.
        return (fe_plane_t){ .normal = FE_VEC3_UP, .d = 0.0f }; // Varsayılan dön
    }
    
    normal = fe_vec3_scale(normal, 1.0f / len);

    // 4. Orijinden uzaklığı (d) hesapla: d = -(N . P)
    // Herhangi bir noktayı (örn: p1) kullanabiliriz.
    float d = -fe_vec3_dot(normal, p1);
    
    return (fe_plane_t){ .normal = normal, .d = d };
}

/**
 * Uygulama: fe_plane_normalize
 */
fe_plane_t fe_plane_normalize(fe_plane_t p) {
    float len = fe_vec3_length(p.normal);
    
    if (len < FLT_EPSILON) {
        return (fe_plane_t){ .normal = FE_VEC3_UP, .d = 0.0f };
    }
    
    float inv_len = 1.0f / len;
    
    p.normal = fe_vec3_scale(p.normal, inv_len);
    p.d *= inv_len; // d'yi de aynı oranda ölçekle
    
    return p;
}

/**
 * Uygulama: fe_plane_distance_to_point
 * Denklemi: Uzaklık = N . P + d
 */
float fe_plane_distance_to_point(fe_plane_t plane, fe_vec3_t point) {
    // Normalin birim vektör olduğu varsayılır
    return fe_vec3_dot(plane.normal, point) + plane.d;
}

/**
 * Uygulama: fe_plane_check_point_side
 */
fe_plane_side_t fe_plane_check_point_side(fe_plane_t plane, fe_vec3_t point, float tolerance) {
    float distance = fe_plane_distance_to_point(plane, point);
    
    if (distance > tolerance) {
        return FE_PLANE_FRONT;
    } else if (distance < -tolerance) {
        return FE_PLANE_BACK;
    } else {
        return FE_PLANE_ON;
    }
}