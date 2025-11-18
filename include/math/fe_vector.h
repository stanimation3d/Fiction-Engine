// include/math/fe_vector.h

#ifndef FE_VECTOR_H
#define FE_VECTOR_H

#include <math.h> // sqrtf, fabsf gibi matematik fonksiyonları için

// ----------------------------------------------------------------------
// 1. VEKTÖR YAPILARI (fe_vecN_t)
// ----------------------------------------------------------------------

/**
 * @brief 2 Boyutlu Vektör (fe_vec2_t).
 */
typedef union fe_vec2 {
    struct { float x, y; };
    float v[2];
} fe_vec2_t;

/**
 * @brief 3 Boyutlu Vektör (fe_vec3_t).
 */
typedef union fe_vec3 {
    struct { float x, y, z; };
    struct { float r, g, b; }; // Renkler için alias
    float v[3];
} fe_vec3_t;

/**
 * @brief 4 Boyutlu Vektör veya Kuaterniyon (fe_vec4_t).
 */
typedef union fe_vec4 {
    struct { float x, y, z, w; };
    struct { float r, g, b, a; }; // Renk/Homojen koordinatlar için alias
    float v[4];
} fe_vec4_t;


// ----------------------------------------------------------------------
// 2. SABİT VEKTÖRLER (FE_VECN_...)
// ----------------------------------------------------------------------

// Harici sabit vektör bildirimleri
extern const fe_vec2_t FE_VEC2_ZERO;
extern const fe_vec2_t FE_VEC2_ONE;

extern const fe_vec3_t FE_VEC3_ZERO;
extern const fe_vec3_t FE_VEC3_ONE;
extern const fe_vec3_t FE_VEC3_FORWARD;  // (0, 0, -1) veya (0, 0, 1) motor konvansiyonuna göre
extern const fe_vec3_t FE_VEC3_UP;       // (0, 1, 0)

extern const fe_vec4_t FE_VEC4_ZERO;


// ----------------------------------------------------------------------
// 3. 3D VEKTÖR İŞLEMLERİ (En çok kullanılanlar)
// ----------------------------------------------------------------------

// Oluşturma
fe_vec3_t fe_vec3_create(float x, float y, float z);

// Temel Aritmetik
fe_vec3_t fe_vec3_add(fe_vec3_t a, fe_vec3_t b);
fe_vec3_t fe_vec3_subtract(fe_vec3_t a, fe_vec3_t b);
fe_vec3_t fe_vec3_scale(fe_vec3_t v, float s);
fe_vec3_t fe_vec3_negate(fe_vec3_t v);

// Uzunluk ve Normalizasyon
float fe_vec3_length_sq(fe_vec3_t v); // Uzunluğun karesi (Daha hızlı karşılaştırma için)
float fe_vec3_length(fe_vec3_t v);
fe_vec3_t fe_vec3_normalize(fe_vec3_t v);

// Çarpımlar
float fe_vec3_dot(fe_vec3_t a, fe_vec3_t b);
fe_vec3_t fe_vec3_cross(fe_vec3_t a, fe_vec3_t b);

// Diğer yardımcılar
fe_vec3_t fe_vec3_lerp(fe_vec3_t start, fe_vec3_t end, float t);
float fe_vec3_distance(fe_vec3_t a, fe_vec3_t b);


// ----------------------------------------------------------------------
// 4. DİĞER VEKTÖRLERİN İŞLEMLERİ (Prototipler)
// ----------------------------------------------------------------------

// 2D Vektör (fe_vec2_t) işlemlerinin prototipleri (fe_vector.c'de uygulanacak)
fe_vec2_t fe_vec2_add(fe_vec2_t a, fe_vec2_t b);
// ...

// 4D Vektör (fe_vec4_t) işlemlerinin prototipleri (fe_vector.c'de uygulanacak)
fe_vec4_t fe_vec4_add(fe_vec4_t a, fe_vec4_t b);
// ...

#endif // FE_VECTOR_H