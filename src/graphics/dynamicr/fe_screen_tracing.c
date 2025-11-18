// src/graphics/dynamicr/fe_screen_tracing.c

#include "graphics/dynamicr/fe_screen_tracing.h"
#include "graphics/fe_renderer.h"
#include "graphics/fe_shader_compiler.h"
#include "graphics/fe_material_editor.h"
#include "graphics/opengl/fe_gl_commands.h" // Düşük seviye GL komutları için
#include "utils/fe_logger.h"
#include <stdlib.h> // calloc, free için
#include <raylib.h> // GetScreenWidth/Height için (veya motorun kendi pencere verileri)


// ----------------------------------------------------------------------
// 1. DAHİLİ KAYNAKLAR
// ----------------------------------------------------------------------

// DynamicR'ın tam ekran dörtgeni (quad) için statik VAO (fe_gl_mesh.c'den alınmış varsayılır)
extern fe_buffer_id_t g_fullscreen_quad_vao; // Bu, DynamicR backend'inde merkezi olarak tanımlanmalıdır.

// Shader dosya yolları (Gerçek projede bu yollar konfigürasyondan gelmelidir)
#define TRACING_VS_PATH "resources/shaders/dynamicr/fullscreen_quad.vs"
#define TRACING_FS_PATH "resources/shaders/dynamicr/screen_trace.fs"


// ----------------------------------------------------------------------
// 2. ARABİRİM UYGULAMALARI
// ----------------------------------------------------------------------

/**
 * Uygulama: fe_screen_tracing_init
 */
fe_screen_tracing_context_t* fe_screen_tracing_init(int width, int height) {
    FE_LOG_INFO("DynamicR Screen Tracing baslatiliyor (W:%d, H:%d)...", width, height);

    fe_screen_tracing_context_t* context = (fe_screen_tracing_context_t*)calloc(1, sizeof(fe_screen_tracing_context_t));
    if (!context) {
        FE_LOG_FATAL("Screen Tracing baglami icin bellek ayrilamadi.");
        return NULL;
    }

    // 1. Shader'i yükle
    context->screen_trace_shader_id = fe_shader_load(TRACING_VS_PATH, TRACING_FS_PATH);
    if (context->screen_trace_shader_id == 0) {
        FE_LOG_FATAL("Screen Tracing Shader yuklenemedi: %s", TRACING_FS_PATH);
        free(context);
        return NULL;
    }

    // 2. Materyali oluştur (Shader'ı tutmak ve kolay bağlama yapmak için)
    context->tracing_material = fe_material_create(context->screen_trace_shader_id);
    if (!context->tracing_material) {
        fe_shader_unload(context->screen_trace_shader_id);
        free(context);
        return NULL;
    }

    // 3. Çıktı FBO'sunu oluştur (Yansıma/Dolaylı Işık sonucunu tutmak için)
    // Gerçek uygulamada FBO oluşturma fe_gl_device'da yapılmalıdır.
    // Şimdilik sadece FBO pointer'ının ayrıldığını varsayalım.
    // context->output_fbo = fe_gl_device_create_framebuffer_and_texture(width, height, FE_TEXTURE_FORMAT_RGBA16F);

    FE_LOG_INFO("Screen Tracing hazir. Shader ID: %u", context->screen_trace_shader_id);
    return context;
}

/**
 * Uygulama: fe_screen_tracing_shutdown
 */
void fe_screen_tracing_shutdown(fe_screen_tracing_context_t* context) {
    if (!context) return;

    // Shader ve Materyali temizle
    if (context->tracing_material) {
        fe_material_destroy(context->tracing_material);
    }
    if (context->screen_trace_shader_id != 0) {
        fe_shader_unload(context->screen_trace_shader_id);
    }
    // FBO'yu temizle
    // if (context->output_fbo) { fe_gl_device_destroy_framebuffer(context->output_fbo); }

    free(context);
    FE_LOG_DEBUG("Screen Tracing kapatildi.");
}

/**
 * Uygulama: fe_screen_tracing_run
 */
void fe_screen_tracing_run(fe_screen_tracing_context_t* context, const fe_framebuffer_t* gbuffer_fbo) {
    if (!context || !context->tracing_material || !context->output_fbo || !gbuffer_fbo) {
        FE_LOG_ERROR("Screen Tracing calistirilamadi: Gecersiz baglam veya tampon.");
        return;
    }

    // 1. Çıktı FBO'sunu Bağla
    fe_renderer_bind_framebuffer(context->output_fbo);
    fe_renderer_clear(FE_CLEAR_COLOR, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f); // Çıktıyı sıfırla

    // 2. Tracing Materyali Bağla (Shader'ı aktif hale getirir)
    fe_material_bind(context->tracing_material);

    // 3. G-Buffer Girişlerini Bağla
    // Shader'daki sampler'lar ile eşleşen doku birimlerine bağlarız.
    // Doku birimleri (Texture Units) 0'dan başlar.
    
    // G-Buffer Derinlik (Depth)
    fe_gl_cmd_bind_texture(gbuffer_fbo->depth_texture_id, 0); 
    // fe_shader_set_uniform_int("u_DepthMap", 0); // Shader compiler'da bu çağrılmalıdır.

    // G-Buffer Normal
    fe_gl_cmd_bind_texture(gbuffer_fbo->color_texture_id[1], 1); // Varsayım: Normal, 1. renk tamponunda
    // fe_shader_set_uniform_int("u_NormalMap", 1); 

    // TODO: Kamera ve Boru Hattı Uniform'larını ayarla (u_InvProjection, u_ViewMatrix vb.)
    // fe_shader_set_uniform_mat4("u_InvViewProj", fe_camera_get_inverse_view_projection());

    // 4. Tam Ekran Dörtgeni Çiz
    // VAO'nun daha önce fe_gl_mesh.c'de oluşturulup global olarak erişilebilir olduğu varsayılır.
    if (g_fullscreen_quad_vao != 0) {
        fe_gl_cmd_bind_vao(g_fullscreen_quad_vao);
        fe_gl_cmd_draw_indexed(6, 0); // 6 index (2 üçgen)
        fe_gl_cmd_unbind_vao();
    } else {
        FE_LOG_ERROR("Global Quad VAO bulunamadi, Screen Tracing Pass atlandi.");
    }
    
    // 5. Shader/Materyal bağlantısını çöz
    fe_shader_unuse();
    
    // 6. FBO bağlantısını çöz (Bir sonraki geçişin bağlaması beklenir, ancak temizlik iyidir)
    fe_renderer_bind_framebuffer(NULL);
}