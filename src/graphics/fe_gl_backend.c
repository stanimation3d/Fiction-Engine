// src/graphics/opengl/fe_gl_backend.c

#include "graphics/opengl/fe_gl_backend.h"
#include "graphics/opengl/fe_gl_pipeline.h" // Pipeline durumunu yönetmek için
#include "graphics/opengl/fe_gl_commands.h" // Çizim komutlarını göndermek için
#include "graphics/opengl/fe_gl_device.h"   // Buffer/Texture oluşturma (şimdilik sadece FBO'lar)
#include "graphics/fe_material_editor.h" // fe_clear_flags_t için
#include "utils/fe_logger.h"
#include <raylib.h> // Raylib'in GL yüklemesi ve pencere yonetimi icin (InitWindow, SwapBuffers)
#include <GL/gl.h> // Doğrudan OpenGL komutları


// ----------------------------------------------------------------------
// 1. BACKEND YAŞAM DÖNGÜSÜ UYGULAMALARI
// ----------------------------------------------------------------------

/**
 * Uygulama: fe_gl_init
 */
fe_error_code_t fe_gl_init(void) {
    FE_LOG_INFO("OpenGL Render Backend baslatiliyor...");
    
    // Raylib, burada zaten GL bağlamını oluşturmuş ve yüklemiş olmalıdır (InitWindow ile).

    // Alt modülleri başlat
    fe_gl_pipeline_init(); // Pipeline durum onbellegini baslat

    // Temel GL ayarlarını yap (fe_gl_pipeline_init içinde zaten yapıldı, ama burada tekrar edilebilir)
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    
    FE_LOG_INFO("OpenGL Render Backend hazir.");
    return FE_OK;
}

/**
 * Uygulama: fe_gl_shutdown
 */
void fe_gl_shutdown(void) {
    // Shutdown gerektiren alt modülleri kapat (şimdilik sadece pipeline'ın durumunu sileriz)
    // Raylib'in pencere/GL kapatma islemi burada degil, üst seviyede olmalıdır.
    FE_LOG_INFO("OpenGL Render Backend kapatiliyor.");
}


// ----------------------------------------------------------------------
// 2. KARE YÖNETİMİ UYGULAMALARI
// ----------------------------------------------------------------------

/**
 * Uygulama: fe_gl_begin_frame
 */
void fe_gl_begin_frame(void) {
    // Burada herhangi bir temizleme işlemi yapılmaz. Temizleme işlemi fe_render_pass'te yapılır.
    // Sadece Raylib'e yeni bir kareye başlandığını bildiririz (eğer kullanılıyorsa).
    BeginDrawing(); // Raylib'in BeginDrawing'i aynı zamanda GL durmunu hazırlar.
}

/**
 * Uygulama: fe_gl_end_frame
 */
void fe_gl_end_frame(void) {
    EndDrawing(); // Raylib'in EndDrawing'i, SwapBuffers işlemini yapar.
}


// ----------------------------------------------------------------------
// 3. ÇİZİM İŞLEMLERİ UYGULAMALARI
// ----------------------------------------------------------------------

/**
 * Uygulama: fe_gl_draw_mesh
 */
void fe_gl_draw_mesh(const fe_mesh_t* mesh, uint32_t instance_count) {
    if (!mesh || mesh->vao_id == 0) {
        FE_LOG_WARN("Gecersiz mesh cizimi atlandi.");
        return;
    }
    
    // 1. VAO'yu bağla (fe_gl_commands kullanılarak)
    fe_gl_cmd_bind_vao(mesh->vao_id); 
    
    // 2. Çizim Komutunu Gönder (fe_gl_commands kullanılarak)
    if (instance_count > 1) {
        fe_gl_cmd_draw_indexed_instanced(mesh->index_count, instance_count, 0); // 0 = GL_TRIANGLES
    } else {
        fe_gl_cmd_draw_indexed(mesh->index_count, 0); // 0 = GL_TRIANGLES
    }
    
    // 3. VAO bağlantısını çöz (Temizlik)
    fe_gl_cmd_unbind_vao(); 
}

/**
 * Uygulama: fe_gl_bind_framebuffer
 */
void fe_gl_bind_framebuffer(fe_framebuffer_t* fbo) {
    uint32_t fbo_id = 0;
    if (fbo) {
        fbo_id = fbo->fbo_id;
        // Viewport'u FBO boyutuna ayarla
        glViewport(0, 0, fbo->width, fbo->height);
    } else {
        // Ana ekrana çizim (Default FBO)
        glViewport(0, 0, GetScreenWidth(), GetScreenHeight());
    }
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_id);
}

/**
 * Uygulama: fe_gl_clear_framebuffer
 */
void fe_gl_clear_framebuffer(fe_framebuffer_t* fbo, fe_clear_flags_t flags, float r, float g, float b, float a, float depth) {
    // 1. FBO'yu bağla
    fe_gl_bind_framebuffer(fbo);

    uint32_t gl_clear_bits = 0;
    
    // 2. Temizleme bayraklarını GL sabitlerine çevir
    if (flags & FE_CLEAR_COLOR) {
        gl_clear_bits |= GL_COLOR_BUFFER_BIT;
        glClearColor(r, g, b, a);
    }
    if (flags & FE_CLEAR_DEPTH) {
        gl_clear_bits |= GL_DEPTH_BUFFER_BIT;
        glClearDepth(depth);
    }
    if (flags & FE_CLEAR_STENCIL) {
        gl_clear_bits |= GL_STENCIL_BUFFER_BIT;
        glClearStencil(0); 
    }
    
    // 3. Gerçek temizleme çağrısı
    if (gl_clear_bits != 0) {
        glClear(gl_clear_bits);
    }

    // 4. Bağlantıyı çöz (fe_gl_bind_framebuffer(NULL) çağırmak yerine, genellikle sonraki geçişin bağlaması beklenir)
    // Şimdilik temizleme işleminden sonra bırakalım.
}