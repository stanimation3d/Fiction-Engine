// src/graphics/dynamicr/fe_ray_combiner.c

#include "graphics/dynamicr/fe_ray_combiner.h"
#include "graphics/fe_renderer.h"               // FBO yonetimi
#include "graphics/fe_shader_compiler.h"        // Shader yonetimi
#include "graphics/opengl/fe_gl_commands.h"     // Çizim komutları (Quad)
#include "utils/fe_logger.h"
#include <stdlib.h> // calloc, free için
#include <GL/gl.h>  // OpenGL komutları

// Shader dosya yolları
#define COMBINER_VS_PATH "resources/shaders/dynamicr/fullscreen_quad.vs"
#define COMBINER_FS_PATH "resources/shaders/dynamicr/ray_combine.fs"

// Voxel GI'daki gibi, DynamicR'ın tam ekran dörtgeni (quad) için statik VAO
extern fe_buffer_id_t g_fullscreen_quad_vao; 


// ----------------------------------------------------------------------
// 1. ARABİRİM UYGULAMALARI
// ----------------------------------------------------------------------

/**
 * Uygulama: fe_ray_combiner_init
 */
fe_ray_combiner_context_t* fe_ray_combiner_init(void) {
    FE_LOG_INFO("Ray Combiner sistemi baslatiliyor...");
    
    fe_ray_combiner_context_t* context = (fe_ray_combiner_context_t*)calloc(1, sizeof(fe_ray_combiner_context_t));
    if (!context) {
        FE_LOG_FATAL("Ray Combiner baglami icin bellek ayrilamadi.");
        return NULL;
    }

    // 1. Shader'i yükle (Vertex ve Fragment Shader)
    context->combine_shader_id = fe_shader_load(COMBINER_VS_PATH, COMBINER_FS_PATH);
    if (context->combine_shader_id == 0) {
        FE_LOG_FATAL("Ray Combiner Shader yuklenemedi: %s", COMBINER_FS_PATH);
        free(context);
        return NULL;
    }

    // 2. Materyali oluştur
    context->combine_material = fe_material_create(context->combine_shader_id);
    if (!context->combine_material) {
        fe_shader_unload(context->combine_shader_id);
        free(context);
        return NULL;
    }

    FE_LOG_INFO("Ray Combiner hazir. Shader ID: %u", context->combine_shader_id);
    return context;
}

/**
 * Uygulama: fe_ray_combiner_shutdown
 */
void fe_ray_combiner_shutdown(fe_ray_combiner_context_t* context) {
    if (!context) return;
    
    if (context->combine_material) {
        fe_material_destroy(context->combine_material);
    }
    if (context->combine_shader_id != 0) {
        fe_shader_unload(context->combine_shader_id);
    }

    free(context);
    FE_LOG_DEBUG("Ray Combiner kapatildi.");
}

/**
 * Uygulama: fe_ray_combiner_run
 */
void fe_ray_combiner_run(fe_ray_combiner_context_t* context, 
                         const fe_framebuffer_t* gbuffer_fbo, 
                         const fe_framebuffer_t* screen_trace_output,
                         fe_texture_id_t voxel_radiance_volume,
                         const fe_mat4_t* view, 
                         const fe_mat4_t* proj) {
    if (!context || !gbuffer_fbo || !screen_trace_output) {
        FE_LOG_ERROR("Ray Combiner calistirilamadi: Gecersiz baglam veya girdi.");
        return;
    }

    // 1. Hedef FBO'yu Bağla (NULL, yani ana ekrana cizim yapılır)
    // DynamicR'daki son geçiş olduğu için, nihai FBO'yu (genellikle ana ekran) bağlarız.
    fe_renderer_bind_framebuffer(NULL);
    fe_renderer_clear(FE_CLEAR_COLOR | FE_CLEAR_DEPTH, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f); // Ekranı temizle

    // 2. Kombine Materyali Bağla (Shader'ı aktif hale getirir)
    fe_material_bind(context->combine_material);

    // 3. Tüm Girdi Kaplamalarını Bağla (Tekstür Birimleri Üzerinden)
    
    // G-Buffer Girişleri:
    fe_gl_cmd_bind_texture(gbuffer_fbo->color_texture_id[0], 0); // Albedo
    fe_shader_set_uniform_int("u_AlbedoMap", 0); 
    
    fe_gl_cmd_bind_texture(gbuffer_fbo->color_texture_id[1], 1); // Normal
    fe_shader_set_uniform_int("u_NormalMap", 1); 
    
    fe_gl_cmd_bind_texture(gbuffer_fbo->depth_texture_id, 2); // Depth
    fe_shader_set_uniform_int("u_DepthMap", 2); 
    
    // Screen Tracing (Ekran Alanı GI/Yansıma)
    fe_gl_cmd_bind_texture(screen_trace_output->color_texture_id[0], 3); // Screen Trace Result
    fe_shader_set_uniform_int("u_SSTraceMap", 3); 
    
    // Voxel GI (Sahne Dışı GI)
    // 3D Doku olduğu için 4. birime bağlanır.
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_3D, voxel_radiance_volume);
    fe_shader_set_uniform_int("u_VoxelRadianceVolume", 4); 
    
    // 4. Uniform'ları Ayarla (Matrisler)
    fe_mat4_t inv_view = fe_mat4_inverse(view); // Tersi alınmış matrisler
    fe_mat4_t inv_proj = fe_mat4_inverse(proj);

    fe_shader_set_uniform_mat4("u_InvView", &inv_view);
    fe_shader_set_uniform_mat4("u_InvProj", &inv_proj); 

    // 5. Tam Ekran Dörtgeni Çiz (Tüm ekran piksellerini çalıştır)
    if (g_fullscreen_quad_vao != 0) {
        fe_gl_cmd_bind_vao(g_fullscreen_quad_vao);
        fe_gl_cmd_draw_indexed(6, 0); 
        fe_gl_cmd_unbind_vao();
    } else {
        FE_LOG_ERROR("Global Quad VAO bulunamadi, Ray Combiner Pass atlandi.");
    }
    
    // 6. Shader/Materyal Bağlantısını Çöz
    fe_shader_unuse();
    
    // 7. Tekstür Bağlantılarını Çöz (Temizlik)
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_3D, 0);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    FE_LOG_TRACE("Ray Combiner Pass Tamamlandı (Nihai Görüntü Oluşturuldu).");
}