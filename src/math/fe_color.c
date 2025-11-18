// src/math/fe_color.c

#include "math/fe_color.h"
#include <math.h>   // fmaxf, fminf için
#include <stdint.h> // uint8_t için
#include <float.h>  // FLT_EPSILON

// ----------------------------------------------------------------------
// 1. SABİT RENK TANIMLAMALARI
// ----------------------------------------------------------------------

const fe_color_t FE_COLOR_WHITE = { .r = 1.0f, .g = 1.0f, .b = 1.0f, .a = 1.0f };
const fe_color_t FE_COLOR_BLACK = { .r = 0.0f, .g = 0.0f, .b = 0.0f, .a = 1.0f };
const fe_color_t FE_COLOR_RED =   { .r = 1.0f, .g = 0.0f, .b = 0.0f, .a = 1.0f };
const fe_color_t FE_COLOR_GREEN = { .r = 0.0f, .g = 1.0f, .b = 0.0f, .a = 1.0f };
const fe_color_t FE_COLOR_BLUE =  { .r = 0.0f, .g = 0.0f, .b = 1.0f, .a = 1.0f };
const fe_color_t FE_COLOR_CLEAR = { .r = 0.0f, .g = 0.0f, .b = 0.0f, .a = 0.0f };


// ----------------------------------------------------------------------
// 2. YARDIMCI VE DÖNÜŞÜM UYGULAMALARI
// ----------------------------------------------------------------------

/**
 * Uygulama: fe_color_create
 */
fe_color_t fe_color_create(float r, float g, float b, float a) {
    // 0.0-1.0 aralığını korumak için sınırlandırma
    r = fmaxf(0.0f, fminf(1.0f, r));
    g = fmaxf(0.0f, fminf(1.0f, g));
    b = fmaxf(0.0f, fminf(1.0f, b));
    a = fmaxf(0.0f, fminf(1.0f, a));
    
    return (fe_color_t){ .r = r, .g = g, .b = b, .a = a };
}

/**
 * Uygulama: fe_color_lerp (Renk Karıştırma)
 */
fe_color_t fe_color_lerp(fe_color_t a, fe_color_t b, float t) {
    // t'yi 0.0 ve 1.0 arasında sıkıştır
    t = fmaxf(0.0f, fminf(1.0f, t));
    
    // Doğrusal enterpolasyon (a + t * (b - a))
    fe_color_t result;
    result.r = a.r + t * (b.r - a.r);
    result.g = a.g + t * (b.g - a.g);
    result.b = a.b + t * (b.b - a.b);
    result.a = a.a + t * (b.a - a.a);
    
    return result;
}

/**
 * Uygulama: fe_color_u8_to_float
 */
fe_color_t fe_color_u8_to_float(fe_color_u8_t c) {
    // Tam sayı (0-255) değerini 255.0f'ye bölerek float (0.0-1.0) değerine çevir
    const float INV_255 = 1.0f / 255.0f;
    
    return (fe_color_t){
        .r = (float)c.r * INV_255,
        .g = (float)c.g * INV_255,
        .b = (float)c.b * INV_255,
        .a = (float)c.a * INV_255
    };
}

/**
 * Uygulama: fe_color_float_to_u8
 */
fe_color_u8_t fe_color_float_to_u8(fe_color_t c) {
    // float (0.0-1.0) değerini 255.0f ile çarpıp en yakın tam sayıya yuvarla
    
    // 0.0-1.0 aralığını sağlamak için sıkıştır
    float r = fmaxf(0.0f, fminf(1.0f, c.r));
    float g = fmaxf(0.0f, fminf(1.0f, c.g));
    float b = fmaxf(0.0f, fminf(1.0f, c.b));
    float a = fmaxf(0.0f, fminf(1.0f, c.a));

    return (fe_color_u8_t){
        // 0.5f ekleyerek yuvarlama (round) işlemi gerçekleştirilir
        .r = (uint8_t)(r * 255.0f + 0.5f),
        .g = (uint8_t)(g * 255.0f + 0.5f),
        .b = (uint8_t)(b * 255.0f + 0.5f),
        .a = (uint8_t)(a * 255.0f + 0.5f)
    };
}