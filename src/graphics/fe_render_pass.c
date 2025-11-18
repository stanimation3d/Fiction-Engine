// src/graphics/fe_render_pass.c

#include "graphics/fe_render_pass.h"
#include "graphics/fe_renderer.h"
#include "utils/fe_logger.h"
#include <raylib.h> // GetFrameTime, rlgl.h gibi Raylib yardımcıları için
#include <string.h> // memcpy için
#include <rlgl.h>   // Doğrudan Raylib'in GL sarmalayıcıları için


/**
 * Uygulama: fe_render_pass_begin
 */
void fe_render_pass_begin(const fe_render_pass_t* render_pass) {
    if (!render_pass) {
        FE_LOG_ERROR("Gecersiz (NULL) render gecisi baslatilmaya calisiliyor.");
        return;
    }
    
    FE_LOG_DEBUG("Render Gecisi Basladi: %s", render_pass->name);

    // 1. Hedef FBO'yu Bağla
    if (render_pass->target_fbo && render_pass->target_fbo->fbo_id != 0) {
        // rlgl'nin FBO bağlama fonksiyonu kullanılır
        rlglBindFramebuffer(render_pass->target_fbo->fbo_id);
        
        // Görünüm alanını FBO boyutuna ayarla
        rlViewport(0, 0, render_pass->target_fbo->width, render_pass->target_fbo->height);

    } else {
        // NULL ise veya ID=0 ise, ana ekrana (varsayılan FBO'ya) çizim yap
        rlglBindFramebuffer(0);
        // Görünüm alanını ana pencere boyutuna ayarla
        rlViewport(0, 0, GetScreenWidth(), GetScreenHeight());
    }

    // 2. Temizleme İşlemi (Clear)
    int gl_clear_bits = 0;
    
    // Renk Temizleme
    if (render_pass->clear_flags & FE_CLEAR_COLOR) {
        gl_clear_bits |= GL_COLOR_BUFFER_BIT;
        // Raylib'in temizleme rengi ayarlama fonksiyonu kullanılır
        rlClearColor(
            (unsigned char)(render_pass->clear_color[0] * 255),
            (unsigned char)(render_pass->clear_color[1] * 255),
            (unsigned char)(render_pass->clear_color[2] * 255),
            (unsigned char)(render_pass->clear_color[3] * 255)
        );
    }

    // Derinlik Temizleme
    if (render_pass->clear_flags & FE_CLEAR_DEPTH) {
        gl_clear_bits |= GL_DEPTH_BUFFER_BIT;
        // Derinlik değerini ayarla ve temizleme bitini ekle
        rlClearDepth(render_pass->clear_depth);
    }
    
    // Şablon (Stencil) Temizleme
    if (render_pass->clear_flags & FE_CLEAR_STENCIL) {
        gl_clear_bits |= GL_STENCIL_BUFFER_BIT;
        // glClearStencil(0); // Şablon değerini ayarla (şimdilik varsayılan 0)
    }

    // Gerçek temizleme GL çağrısı
    if (gl_clear_bits != 0) {
        rlClearScreenBuffers(); // rlClearScreenBuffers, glClear(gl_clear_bits) işlemini Raylib üzerinden yapar.
    }
    
    // TODO: 3. Render Ayarlarını Yapılandır (Culling, Depth Test vb.)
    // fe_renderer.c'de yapılan temel ayarların geçiş bazlı geçersiz kılınması buraya gelir.
}


/**
 * Uygulama: fe_render_pass_end
 */
void fe_render_pass_end(void) {
    // FBO bağlantısını sıfırla, böylece sonraki çizim ana ekrana (default framebuffer) yapılır.
    // rlglBindFramebuffer(0); // rlglClearColor ile birlikte zaten yapılıyor olabilir, ancak güvenlik için çağırmak mantıklıdır.
    
    // Varsayılan Frame Buffer'a (0) geri dönülür ve viewport sıfırlanır
    rlViewport(0, 0, GetScreenWidth(), GetScreenHeight());
    rlglBindFramebuffer(0); 

    FE_LOG_DEBUG("Render Gecisi Sonlandi.");
}