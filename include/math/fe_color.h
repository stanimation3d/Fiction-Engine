// include/math/fe_color.h

#ifndef FE_COLOR_H
#define FE_COLOR_H

#include <stdint.h>
#include "fe_vector.h" // fe_vec4_t ile uyumluluk için

// ----------------------------------------------------------------------
// 1. RENK YAPILARI
// ----------------------------------------------------------------------

/**
 * @brief Yüksek hassasiyetli (32-bit float) Renk Yapısı.
 * * Genellikle grafik boru hattinda (pipeline) kullanilir (0.0 - 1.0 araligi).
 */
typedef union fe_color {
    struct { float r, g, b, a; };
    fe_vec4_t v4; // fe_vec4_t ile ayni bellek duzenini paylasir
    float v[4];
} fe_color_t;

/**
 * @brief Tam sayı (8-bit) Renk Yapısı.
 * * Görüntü yükleme, donanim renkleri veya bellek tasarrufu için kullanilir (0 - 255 araligi).
 */
typedef struct fe_color_u8 {
    uint8_t r, g, b, a;
} fe_color_u8_t;


// ----------------------------------------------------------------------
// 2. SABİT RENKLER (FE_COLOR_...)
// ----------------------------------------------------------------------

extern const fe_color_t FE_COLOR_WHITE;
extern const fe_color_t FE_COLOR_BLACK;
extern const fe_color_t FE_COLOR_RED;
extern const fe_color_t FE_COLOR_GREEN;
extern const fe_color_t FE_COLOR_BLUE;
extern const fe_color_t FE_COLOR_CLEAR; // Tamamen saydam (0,0,0,0)


// ----------------------------------------------------------------------
// 3. YARDIMCI VE DÖNÜŞÜM FONKSİYONLARI
// ----------------------------------------------------------------------

/**
 * @brief Float tabanlı (fe_color_t) bir renk olusturur.
 */
fe_color_t fe_color_create(float r, float g, float b, float a);

/**
 * @brief İki rengi belirli bir oranla karistirir (Linear Interpolation - LERP).
 */
fe_color_t fe_color_lerp(fe_color_t a, fe_color_t b, float t);

/**
 * @brief fe_color_u8_t'yi fe_color_t'ye dönüstürür.
 * * (0-255 -> 0.0-1.0)
 */
fe_color_t fe_color_u8_to_float(fe_color_u8_t c);

/**
 * @brief fe_color_t'yi fe_color_u8_t'ye dönüstürür.
 * * (0.0-1.0 -> 0-255)
 */
fe_color_u8_t fe_color_float_to_u8(fe_color_t c);


#endif // FE_COLOR_H