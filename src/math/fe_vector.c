// src/math/fe_vector.c

#include "math/fe_vector.h"
#include <float.h> // FLT_EPSILON için

// ----------------------------------------------------------------------
// 1. SABİT VEKTÖR TANIMLAMALARI
// ----------------------------------------------------------------------

const fe_vec2_t FE_VEC2_ZERO = { .x = 0.0f, .y = 0.0f };
const fe_vec2_t FE_VEC2_ONE = { .x = 1.0f, .y = 1.0f };

const fe_vec3_t FE_VEC3_ZERO = { .x = 0.0f, .y = 0.0f, .z = 0.0f };
const fe_vec3_t FE_VEC3_ONE = { .x = 1.0f, .y = 1.0f, .z = 1.0f };
const fe_vec3_t FE_VEC3_FORWARD = { .x = 0.0f, .y = 0.0f, .z = 1.0f }; // +Z ileri kabul edelim
const fe_vec3_t FE_VEC3_UP = { .x = 0.0f, .y = 1.0f, .z = 0.0f };

const fe_vec4_t FE_VEC4_ZERO = { .x = 0.0f, .y = 0.0f, .z = 0.0f, .w = 0.0f };


// ----------------------------------------------------------------------
// 2. 3D VEKTÖR İŞLEMLERİ UYGULAMALARI
// ----------------------------------------------------------------------

/**
 * Uygulama: fe_vec3_create
 */
fe_vec3_t fe_vec3_create(float x, float y, float z) {
    return (fe_vec3_t){ .x = x, .y = y, .z = z };
}

/**
 * Uygulama: fe_vec3_add
 */
fe_vec3_t fe_vec3_add(fe_vec3_t a, fe_vec3_t b) {
    return (fe_vec3_t){ .x = a.x + b.x, .y = a.y + b.y, .z = a.z + b.z };
}

/**
 * Uygulama: fe_vec3_subtract
 */
fe_vec3_t fe_vec3_subtract(fe_vec3_t a, fe_vec3_t b) {
    return (fe_vec3_t){ .x = a.x - b.x, .y = a.y - b.y, .z = a.z - b.z };
}

/**
 * Uygulama: fe_vec3_scale
 */
fe_vec3_t fe_vec3_scale(fe_vec3_t v, float s) {
    return (fe_vec3_t){ .x = v.x * s, .y = v.y * s, .z = v.z * s };
}

/**
 * Uygulama: fe_vec3_negate
 */
fe_vec3_t fe_vec3_negate(fe_vec3_t v) {
    return (fe_vec3_t){ .x = -v.x, .y = -v.y, .z = -v.z };
}

/**
 * Uygulama: fe_vec3_length_sq
 */
float fe_vec3_length_sq(fe_vec3_t v) {
    return v.x * v.x + v.y * v.y + v.z * v.z;
}

/**
 * Uygulama: fe_vec3_length
 */
float fe_vec3_length(fe_vec3_t v) {
    return sqrtf(fe_vec3_length_sq(v));
}

/**
 * Uygulama: fe_vec3_normalize
 */
fe_vec3_t fe_vec3_normalize(fe_vec3_t v) {
    float len = fe_vec3_length(v);
    
    // Sıfıra bölme hatasını önle
    if (len < FLT_EPSILON) { 
        return FE_VEC3_ZERO; 
    }
    
    float inv_len = 1.0f / len;
    return fe_vec3_scale(v, inv_len);
}

/**
 * Uygulama: fe_vec3_dot (Nokta Çarpım)
 */
float fe_vec3_dot(fe_vec3_t a, fe_vec3_t b) {
    // a.b = ax*bx + ay*by + az*bz
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

/**
 * Uygulama: fe_vec3_cross (Çapraz Çarpım)
 */
fe_vec3_t fe_vec3_cross(fe_vec3_t a, fe_vec3_t b) {
    // a x b = (ay*bz - az*by, az*bx - ax*bz, ax*by - ay*bx)
    return (fe_vec3_t){ 
        .x = a.y * b.z - a.z * b.y,
        .y = a.z * b.x - a.x * b.z,
        .z = a.x * b.y - a.y * b.x
    };
}

/**
 * Uygulama: fe_vec3_lerp (Doğrusal Enterpolasyon)
 */
fe_vec3_t fe_vec3_lerp(fe_vec3_t start, fe_vec3_t end, float t) {
    // lerp(a, b, t) = a + t * (b - a)
    fe_vec3_t delta = fe_vec3_subtract(end, start);
    fe_vec3_t scaled_delta = fe_vec3_scale(delta, t);
    return fe_vec3_add(start, scaled_delta);
}

/**
 * Uygulama: fe_vec3_distance
 */
float fe_vec3_distance(fe_vec3_t a, fe_vec3_t b) {
    fe_vec3_t delta = fe_vec3_subtract(a, b);
    return fe_vec3_length(delta);
}


// ----------------------------------------------------------------------
// 3. 2D VE 4D VEKTÖR İŞLEMLERİ (Basit Uygulamalar)
// ----------------------------------------------------------------------

// fe_vec2_t toplama
fe_vec2_t fe_vec2_add(fe_vec2_t a, fe_vec2_t b) {
    return (fe_vec2_t){ .x = a.x + b.x, .y = a.y + b.y };
}

// fe_vec4_t toplama
fe_vec4_t fe_vec4_add(fe_vec4_t a, fe_vec4_t b) {
    return (fe_vec4_t){ .x = a.x + b.x, .y = a.y + b.y, .z = a.z + b.z, .w = a.w + b.w };
}

// fe_vec4_t normalizasyonu (Kuaterniyon normalizasyonu için önemli)
fe_vec4_t fe_vec4_normalize(fe_vec4_t v) {
    float len_sq = v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w;
    float len = sqrtf(len_sq);
    
    if (len < FLT_EPSILON) { 
        return FE_VEC4_ZERO; 
    }
    
    float inv_len = 1.0f / len;
    return fe_vec4_scale(v, inv_len);
}

// fe_vec4_t skaler çarpım
fe_vec4_t fe_vec4_scale(fe_vec4_t v, float s) {
    return (fe_vec4_t){ .x = v.x * s, .y = v.y * s, .z = v.z * s, .w = v.w * s };
}