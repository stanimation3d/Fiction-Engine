// src/graphics/geometryv/fe_gv_tracer.c

#include "graphics/geometryv/fe_gv_tracer.h"
#include "graphics/opengl/fe_gl_device.h"       // Doku ve Kaynak yönetimi için
#include "graphics/fe_shader_compiler.h"        // Shader yüklemek için
#include "utils/fe_logger.h"
#include <stdlib.h> // calloc, free için
#include <GL/gl.h>  // OpenGL komutları

// Shader dosya yolu
#define GV_TRACE_CS_PATH  "resources/shaders/geometryv/gv_trace.comp"


// ----------------------------------------------------------------------
// 1. DAHİLİ YARDIMCI FONKSİYONLAR
// ----------------------------------------------------------------------

/**
 * @brief R-Buffer icin 2D cözünürlüklü bir kaplama (texture) olusturur.
 */
static fe_texture_id_t fe_gv_tracer_create_texture_2d(uint32_t width, uint32_t height, uint32_t internal_format) {
    fe_texture_id_t texture_id;
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    GLenum format = (internal_format == GL_R16F || internal_format == GL_RGBA16F) ? GL_RGBA : GL_RGB;
    GLenum type = (internal_format == GL_RGBA16F) ? GL_HALF_FLOAT : GL_UNSIGNED_BYTE;
    
    glTexImage2D(GL_TEXTURE_2D, 0, internal_format, 
                 (GLsizei)width, (GLsizei)height, 
                 0, format, type, NULL);
    
    glBindTexture(GL_TEXTURE_2D, 0);
    return texture_id;
}


// ----------------------------------------------------------------------
// 2. ARABİRİM UYGULAMALARI
// ----------------------------------------------------------------------

/**
 * Uygulama: fe_gv_tracer_init
 */
fe_gv_tracer_context_t* fe_gv_tracer_init(uint32_t width, uint32_t height) {
    FE_LOG_INFO("GeometryV Tracer baslatiliyor (W:%u, H:%u)...", width, height);
    
    fe_gv_tracer_context_t* context = (fe_gv_tracer_context_t*)calloc(1, sizeof(fe_gv_tracer_context_t));
    if (!context) return NULL;

    context->r_buffer.width = width;
    context->r_buffer.height = height;

    // 1. R-Buffer Kaplamalarini Oluştur
    // Pozisyon + Mesafe (HDR)
    context->r_buffer.position_map_id = fe_gv_tracer_create_texture_2d(width, height, GL_RGBA16F); 
    // Normal + Materyal ID
    context->r_buffer.normal_map_id = fe_gv_tracer_create_texture_2d(width, height, GL_RGBA8); 
    // Albedo + Roughness
    context->r_buffer.albedo_map_id = fe_gv_tracer_create_texture_2d(width, height, GL_RGBA8); 
    
    if (context->r_buffer.position_map_id == 0) goto error_cleanup;

    // 2. Compute Shader'ı yükle
    fe_shader_id_t trace_shader = fe_shader_load_compute(GV_TRACE_CS_PATH);
    if (trace_shader == 0) goto error_cleanup;
    context->trace_material = fe_material_create(trace_shader);

    FE_LOG_INFO("GeometryV Tracer R-Buffer hazir.");
    return context;

error_cleanup:
    FE_LOG_FATAL("GeometryV Tracer baslatilirken hata olustu.");
    fe_gv_tracer_shutdown(context);
    return NULL;
}

/**
 * Uygulama: fe_gv_tracer_shutdown
 */
void fe_gv_tracer_shutdown(fe_gv_tracer_context_t* context) {
    if (!context) return;
    
    if (context->trace_material) { fe_material_destroy(context->trace_material); }

    // R-Buffer kaplamalarını sil
    fe_gl_device_destroy_texture(context->r_buffer.position_map_id);
    fe_gl_device_destroy_texture(context->r_buffer.normal_map_id);
    fe_gl_device_destroy_texture(context->r_buffer.albedo_map_id);

    free(context);
    FE_LOG_DEBUG("GeometryV Tracer kapatildi.");
}

/**
 * Uygulama: fe_gv_tracer_run_primary_rays
 */
void fe_gv_tracer_run_primary_rays(fe_gv_tracer_context_t* context, 
                                   const fe_gv_scene_t* scene) {
    if (!context || !scene || scene->hierarchy.node_count == 0) {
        FE_LOG_ERROR("Tracer calistirilamadi: Gecersiz baglam veya bos sahne.");
        return;
    }
    
    FE_LOG_INFO("GeometryV Birincil Işın Takibi Pass'i basladi.");

    // 1. Compute Shader'ı kullan
    fe_material_bind(context->trace_material);

    // 2. Girdi/Çıktı GPU Kaynaklarını Bağla

    // Çıktı R-Buffer'ı Görüntü Birimleri Olarak Bağla (Image Load/Store)
    glBindImageTexture(0, context->r_buffer.position_map_id, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
    glBindImageTexture(1, context->r_buffer.normal_map_id, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);
    glBindImageTexture(2, context->r_buffer.albedo_map_id, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);

    // Girdi Sahne Kaynaklarını Bağla (SSBO'lar)
    // Üçgenler (Bağlama noktası 3)
    fe_gl_cmd_bind_ssbo(scene->triangle_ssbo, 3);
    // Kümeler (Bağlama noktası 4)
    fe_gl_cmd_bind_ssbo(scene->cluster_ssbo, 4);
    // Hiyerarşi (Bağlama noktası 5)
    fe_gl_cmd_bind_ssbo(scene->hierarchy.hierarchy_ssbo, 5);
    
    // 3. Uniform'ları Ayarla
    // Kamera Matrisleri ve Görüntü Boyutu
    fe_shader_set_uniform_mat4("u_View", &scene->view_matrix);
    fe_shader_set_uniform_mat4("u_Projection", &scene->projection_matrix);
    fe_shader_set_uniform_int("u_ScreenWidth", context->r_buffer.width);

    // 4. Compute Shader'ı Çalıştır (Her piksel için bir iş grubu)
    uint32_t local_size = 8; // Compute Shader'daki local_size_x/y ile uyumlu olmalı (8x8 varsayımı)
    uint32_t dispatch_x = (context->r_buffer.width + local_size - 1) / local_size;
    uint32_t dispatch_y = (context->r_buffer.height + local_size - 1) / local_size;

    glDispatchCompute(dispatch_x, dispatch_y, 1);

    // 5. Shader'ın bitmesini bekle ve sonuçların diğer geçişler için kullanılabilir olmasını sağla
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    // 6. Bağlantıları çöz
    fe_gl_cmd_unbind_ssbo(3);
    fe_gl_cmd_unbind_ssbo(4);
    fe_gl_cmd_unbind_ssbo(5);
    
    glBindImageTexture(0, 0, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA16F);
    glBindImageTexture(1, 0, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA8);
    glBindImageTexture(2, 0, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA8);
    fe_shader_unuse();
    
    FE_LOG_DEBUG("GeometryV Birincil Işınlar Gonderildi.");
}