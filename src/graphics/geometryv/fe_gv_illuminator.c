// src/graphics/geometryv/fe_gv_illuminator.c

#include "graphics/geometryv/fe_gv_illuminator.h"
#include "graphics/fe_renderer.h"               // FBO yönetimi için
#include "graphics/fe_shader_compiler.h"        // Shader yönetimi için
#include "graphics/opengl/fe_gl_commands.h"     // Çizim komutları (Quad) için
#include "utils/fe_logger.h"
#include <stdlib.h> // calloc, free için
#include <GL/gl.h>  // OpenGL komutları

// Shader dosya yolları
#define ILLUMINATOR_VS_PATH "resources/shaders/geometryv/fullscreen_quad.vs" // Tam ekran quad VS'i
#define ILLUMINATOR_FS_PATH "resources/shaders/geometryv/gv_illuminate.fs"

// Global tam ekran dörtgen VAO'sunu dışarıdan al
extern fe_buffer_id_t g_fullscreen_quad_vao; 


// ----------------------------------------------------------------------
// 1. ARABİRİM UYGULAMALARI
// ----------------------------------------------------------------------

/**
 * Uygulama: fe_gv_illuminator_init
 */
fe_gv_illuminator_context_t* fe_gv_illuminator_init(void) {
    FE_LOG_INFO("GeometryV Aydınlatıcı baslatiliyor...");
    
    fe_gv_illuminator_context_t* context = (fe_gv_illuminator_context_t*)calloc(1, sizeof(fe_gv_illuminator_context_t));
    if (!context) {
        FE_LOG_FATAL("Aydınlatıcı baglami icin bellek ayrilamadi.");
        return NULL;
    }

    // 1. Shader'i yükle
    context->illumination_shader_id = fe_shader_load(ILLUMINATOR_VS_PATH, ILLUMINATOR_FS_PATH);
    if (context->illumination_shader_id == 0) {
        FE_LOG_FATAL("Aydınlatıcı Shader yuklenemedi: %s", ILLUMINATOR_FS_PATH);
        free(context);
        return NULL;
    }

    // 2. Materyali oluştur
    context->illumination_material = fe_material_create(context->illumination_shader_id);
    if (!context->illumination_material) {
        fe_shader_unload(context->illumination_shader_id);
        free(context);
        return NULL;
    }

    FE_LOG_INFO("GeometryV Aydınlatıcı hazir. Shader ID: %u", context->illumination_shader_id);
    return context;
}

/**
 * Uygulama: fe_gv_illuminator_shutdown
 */
void fe_gv_illuminator_shutdown(fe_gv_illuminator_context_t* context) {
    if (!context) return;
    
    if (context->illumination_material) {
        fe_material_destroy(context->illumination_material);
    }
    if (context->illumination_shader_id != 0) {
        fe_shader_unload(context->illumination_shader_id);
    }

    free(context);
    FE_LOG_DEBUG("GeometryV Aydınlatıcı kapatildi.");
}

/**
 * Uygulama: fe_gv_illuminator_run
 */
void fe_gv_illuminator_run(fe_gv_illuminator_context_t* context, 
                           const fe_gv_rt_buffer_t* r_buffer, 
                           fe_framebuffer_t* output_fbo,
                           const fe_mat4_t* view, 
                           const fe_mat4_t* proj) {
    if (!context || !r_buffer) {
        FE_LOG_ERROR("Aydınlatıcı calistirilamadi: Gecersiz baglam veya R-Buffer.");
        return;
    }

    // 1. Hedef FBO'yu Bağla (Nihai sonuç buraya yazılır)
    fe_renderer_bind_framebuffer(output_fbo);
    fe_renderer_clear(FE_CLEAR_COLOR | FE_CLEAR_DEPTH, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f); // Temizle

    // 2. Aydınlatma Materyalini Bağla
    fe_material_bind(context->illumination_material);

    // 3. R-Buffer Kaplamalarını Bağla (Tekstür Birimleri Üzerinden)
    
    // Pozisyon + Mesafe
    fe_gl_cmd_bind_texture(r_buffer->position_map_id, 0); 
    fe_shader_set_uniform_int("u_PositionMap", 0); 
    
    // Normal + Materyal ID
    fe_gl_cmd_bind_texture(r_buffer->normal_map_id, 1); 
    fe_shader_set_uniform_int("u_NormalMap", 1); 
    
    // Albedo + Roughness
    fe_gl_cmd_bind_texture(r_buffer->albedo_map_id, 2); 
    fe_shader_set_uniform_int("u_AlbedoMap", 2); 
    
    // 4. Uniform'ları Ayarla
    // Kamera Matrisleri ve Işık Verileri (Işıklar için bir UBO/SSBO bağlanabilir)
    fe_shader_set_uniform_mat4("u_View", view);
    fe_shader_set_uniform_mat4("u_Projection", proj); 
    fe_shader_set_uniform_vec3("u_CameraPosition", &camera_pos); // Kamera pozisyonu
    
    // 5. Tam Ekran Dörtgeni Çiz (Tüm ekran piksellerini ışıklandır)
    if (g_fullscreen_quad_vao != 0) {
        fe_gl_cmd_bind_vao(g_fullscreen_quad_vao);
        fe_gl_cmd_draw_indexed(6, 0); 
        fe_gl_cmd_unbind_vao();
    } else {
        FE_LOG_ERROR("Global Quad VAO bulunamadi, Aydınlatma Pass atlandi.");
    }
    
    // 6. Shader/Materyal Bağlantısını Çöz
    fe_shader_unuse();
    
    // 7. Tekstür Bağlantılarını Çöz (Temizlik)
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    FE_LOG_TRACE("GeometryV Aydınlatma Pass Tamamlandı.");
}