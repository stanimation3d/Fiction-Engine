// include/graphics/fe_render_pass.h

#ifndef FE_RENDER_PASS_H
#define FE_RENDER_PASS_H

#include "error/fe_error.h"
#include "graphics/fe_render_types.h" // fe_framebuffer_t için
#include <stdint.h>
#include <stdbool.h>

// ----------------------------------------------------------------------
// 1. TEMİZLEME VE GÖLGELEME AYARLARI
// ----------------------------------------------------------------------

/**
 * @brief Geçiş başlangıcında temizlenecek tamponları tanımlar.
 */
typedef enum fe_clear_flags {
    FE_CLEAR_COLOR   = 0x1, // Renk tamponunu temizle
    FE_CLEAR_DEPTH   = 0x2, // Derinlik tamponunu temizle
    FE_CLEAR_STENCIL = 0x4, // Şablon tamponunu temizle
    FE_CLEAR_ALL     = FE_CLEAR_COLOR | FE_CLEAR_DEPTH | FE_CLEAR_STENCIL
} fe_clear_flags_t;

// ----------------------------------------------------------------------
// 2. RENDER GEÇİŞİ YAPISI
// ----------------------------------------------------------------------

/**
 * @brief Bir render boru hattındaki tek bir geçişi (pass) temsil eder.
 */
typedef struct fe_render_pass {
    const char* name;                  // Geçişin adı (Hata ayıklama/Loglama için)
    fe_framebuffer_t* target_fbo;      // Çizimin yapılacağı hedef FBO (NULL ise ana ekran)
    fe_clear_flags_t clear_flags;      // Başlangıçta temizlenecek tamponlar
    float clear_color[4];              // Renk temizleme değeri (R, G, B, A)
    float clear_depth;                 // Derinlik temizleme değeri
    
    // Geçiş ayarları (örneğin, görünüm alanı/viewport, makaslama/scissor vb. ayarları buraya eklenebilir)
} fe_render_pass_t;


// ----------------------------------------------------------------------
// 3. FONKSİYON PROTOTİPLERİ
// ----------------------------------------------------------------------

/**
 * @brief Bir render geçişini başlatır.
 * * Hedef FBO'yu bağlar ve belirtilen tamponları temizler.
 * @param render_pass Başlatılacak geçişin pointer'ı.
 */
void fe_render_pass_begin(const fe_render_pass_t* render_pass);

/**
 * @brief Aktif render geçişini sonlandırır.
 * * Geçerli FBO'nun bağlantısını keser (ana ekrana geri döner).
 */
void fe_render_pass_end(void);

#endif // FE_RENDER_PASS_H