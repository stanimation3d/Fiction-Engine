// src/graphics/dynamicr/fe_voxel_gi.c

#include "graphics/dynamicr/fe_voxel_gi.h"
#include "graphics/opengl/fe_gl_device.h"       // Doku ve Kaynak yönetimi için
#include "graphics/opengl/fe_gl_commands.h"     // Çizim komutları için
#include "graphics/fe_shader_compiler.h"        // Shader yüklemek için
#include "utils/fe_logger.h"
#include <stdlib.h> // calloc, free için
#include <GL/gl.h>  // OpenGL komutları

// Shader dosya yolları
#define VOXEL_VS_PATH  "resources/shaders/dynamicr/voxel_geom.vs"
#define VOXEL_GS_PATH  "resources/shaders/dynamicr/voxel_geom.gs" // Geometry Shader sart!
#define VOXEL_FS_PATH  "resources/shaders/dynamicr/voxel_geom.fs"

#define INJECT_CS_PATH "resources/shaders/dynamicr/voxel_inject.comp"
#define TRACING_CS_PATH "resources/shaders/dynamicr/voxel_trace.comp"


// ----------------------------------------------------------------------
// 1. DAHİLİ YARDIMCI FONKSİYONLAR
// ----------------------------------------------------------------------

/**
 * @brief Bos bir 3D doku Volume'u olusturur (GL_R8/GL_R16F/GL_RGBA8 gibi formatlarla).
 */
static fe_texture_id_t fe_voxel_create_volume(uint32_t resolution, uint32_t internal_format) {
    fe_texture_id_t volume_id;
    glGenTextures(1, &volume_id);
    glBindTexture(GL_TEXTURE_3D, volume_id);
    
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    // Veri alanını ayır (örneğin GL_R8 (opaklık) veya GL_RGBA16F (radiance/ışıma))
    GLenum format = (internal_format == GL_R8) ? GL_RED : GL_RGBA;
    GLenum type = (internal_format == GL_RGBA16F) ? GL_HALF_FLOAT : GL_UNSIGNED_BYTE;
    
    glTexImage3D(GL_TEXTURE_3D, 0, internal_format, 
                 (GLsizei)resolution, (GLsizei)resolution, (GLsizei)resolution, 
                 0, format, type, NULL);
    
    glBindTexture(GL_TEXTURE_3D, 0);
    return volume_id;
}

/**
 * @brief Voxel Volume'larını sıfırlar.
 */
static void fe_voxel_clear_volumes(const fe_voxel_grid_t* grid) {
    // Opaklık Volume'u sıfırla (0: boş)
    glClearTexImage(grid->opacity_volume_id, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);
    // Radiance Volume'u sıfırla (0: karanlık)
    glClearTexImage(grid->radiance_volume_id, 0, GL_RGBA, GL_HALF_FLOAT, NULL);
}


// ----------------------------------------------------------------------
// 2. ARABİRİM UYGULAMALARI
// ----------------------------------------------------------------------

/**
 * Uygulama: fe_voxel_gi_init
 */
fe_voxel_gi_context_t* fe_voxel_gi_init(fe_vec3_t world_min, fe_vec3_t world_max) {
    FE_LOG_INFO("Voxel GI sistemi baslatiliyor (Cozunurluk: %u^3)...", VOXEL_GRID_RESOLUTION);
    
    fe_voxel_gi_context_t* context = (fe_voxel_gi_context_t*)calloc(1, sizeof(fe_voxel_gi_context_t));
    if (!context) return NULL;

    context->grid.resolution = VOXEL_GRID_RESOLUTION;
    context->grid.world_min = world_min;
    context->grid.world_max = world_max;

    // 1. Volume'ları Oluştur (fe_voxel_create_volume dahili çağrılır)
    context->grid.opacity_volume_id = fe_voxel_create_volume(VOXEL_GRID_RESOLUTION, GL_R8); // Opaklık (8-bit)
    context->grid.radiance_volume_id = fe_voxel_create_volume(VOXEL_GRID_RESOLUTION, GL_RGBA16F); // Işıma (HDR 16-bit float)
    if (context->grid.opacity_volume_id == 0 || context->grid.radiance_volume_id == 0) goto error_cleanup;

    // 2. Voxelization Shader'ı yükle (Vertex, Geometry ve Fragment/Null)
    fe_shader_id_t voxel_shader = fe_shader_load_geometry_shader(VOXEL_VS_PATH, VOXEL_FS_PATH, VOXEL_GS_PATH);
    if (voxel_shader == 0) goto error_cleanup;
    context->voxelization_material = fe_material_create(voxel_shader);

    // 3. Injection Compute Shader'ı yükle
    fe_shader_id_t inject_shader = fe_shader_load_compute(INJECT_CS_PATH);
    if (inject_shader == 0) goto error_cleanup;
    context->inject_material = fe_material_create(inject_shader);

    // 4. Tracing Compute Shader'ı yükle
    fe_shader_id_t tracing_shader = fe_shader_load_compute(TRACING_CS_PATH);
    if (tracing_shader == 0) goto error_cleanup;
    context->tracing_material = fe_material_create(tracing_shader);

    FE_LOG_INFO("Voxel GI Volume ID'leri: Opacity=%u, Radiance=%u", 
                context->grid.opacity_volume_id, context->grid.radiance_volume_id);
    return context;

error_cleanup:
    FE_LOG_FATAL("Voxel GI baslatilirken hata olustu.");
    fe_voxel_gi_shutdown(context);
    return NULL;
}

/**
 * Uygulama: fe_voxel_gi_shutdown
 */
void fe_voxel_gi_shutdown(fe_voxel_gi_context_t* context) {
    if (!context) return;
    
    // Shader'ları/Materyalleri temizle
    if (context->voxelization_material) { fe_material_destroy(context->voxelization_material); }
    if (context->inject_material) { fe_material_destroy(context->inject_material); }
    if (context->tracing_material) { fe_material_destroy(context->tracing_material); }

    // Volume'ları sil
    if (context->grid.opacity_volume_id != 0) {
        fe_gl_device_destroy_texture(context->grid.opacity_volume_id);
    }
    if (context->grid.radiance_volume_id != 0) {
        fe_gl_device_destroy_texture(context->grid.radiance_volume_id);
    }

    free(context);
    FE_LOG_DEBUG("Voxel GI kapatildi.");
}

/**
 * Uygulama: fe_voxel_gi_voxelize_scene
 */
void fe_voxel_gi_voxelize_scene(fe_voxel_gi_context_t* context, 
                                 const fe_mesh_t* const* meshes, 
                                 uint32_t mesh_count) {
    if (!context) return;
    
    FE_LOG_INFO("Sahne Voxelization Pass'i basladi (%u mesh)...", mesh_count);

    // 1. Radiance ve Opaklık Volume'larını sıfırla
    fe_voxel_clear_volumes(&context->grid);
    
    // 2. Voxelization Pipeline'ı ayarla (3D görüntüye render etmek için)
    glDisable(GL_CULL_FACE); // Voxelization için tüm yüzleri çiz
    glDisable(GL_DEPTH_TEST);
    
    // 3. Opaklık ve Radiance Volume'larını görüntü yazma birimleri olarak bağla (Image Load/Store)
    glBindImageTexture(0, context->grid.opacity_volume_id, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_R8);
    glBindImageTexture(1, context->grid.radiance_volume_id, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA16F);

    // 4. Voxelization Shader'ı kullan
    fe_material_bind(context->voxelization_material);

    // 5. Shader Uniform'larını Ayarla (Örneğin, projeksiyon/izgara boyutu)
    // fe_shader_set_uniform_vec3("u_GridMin", &context->grid.world_min);
    // fe_shader_set_uniform_float("u_GridSize", size); 
    // Voxelizasyon için özel bir ortografik projeksiyon matrisi kurulur.
    
    // 6. Mesh'leri çiz (Geometry Shader, her üçgeni 3 boyutlu bir alana dönüştürür)
    for (uint32_t i = 0; i < mesh_count; ++i) {
        fe_gl_draw_mesh(meshes[i], 1);
    }

    // 7. Bağlantıları çöz ve Pipeline durumunu geri yükle
    glEnable(GL_CULL_FACE); 
    glEnable(GL_DEPTH_TEST);
    glBindImageTexture(0, 0, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R8);
    glBindImageTexture(1, 0, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA16F);
    fe_shader_unuse();
    
    // 8. Voxelization'ın bitmesini bekle
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    FE_LOG_DEBUG("Voxelization tamamlandi.");
}

/**
 * Uygulama: fe_voxel_gi_inject_radiance
 */
void fe_voxel_gi_inject_radiance(fe_voxel_gi_context_t* context, fe_vec3_t camera_position) {
    if (!context || !context->inject_material) return;

    FE_LOG_TRACE("Radiance Injection Pass'i basladi.");

    // 1. Compute Shader'ı kullan
    fe_material_bind(context->inject_material);

    // 2. Volume'ları bağla (Okuma/Yazma)
    // Radiance Volume'una yazma (Işık eklemesi için)
    glBindImageTexture(0, context->grid.radiance_volume_id, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA16F);
    
    // Opaklık Volume'undan okuma (Işığın engellenip engellenmediğini kontrol etmek için)
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, context->grid.opacity_volume_id);
    // fe_shader_set_uniform_int("u_OpacityVolume", 0);

    // 3. Işık Uniform'larını ve Kamera Konumunu ayarla
    // fe_shader_set_uniform_vec3("u_CameraPos", &camera_position); 
    // Işık listesi (SSBO) de burada bağlanmalıdır.

    // 4. Compute Shader'ı Çalıştır (Tüm Voxel ızgarasını kapsa)
    uint32_t num_groups = context->grid.resolution / 8; // 8x8x8 yerel çalışma grubu boyutu varsayımı
    glDispatchCompute(num_groups, num_groups, num_groups);

    // 5. Shader'ın bitmesini bekle
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    // 6. Bağlantıları çöz
    glBindImageTexture(0, 0, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA16F);
    glBindTexture(GL_TEXTURE_3D, 0);
    fe_shader_unuse();
    
    FE_LOG_TRACE("Radiance Injection tamamlandi.");
}


/**
 * Uygulama: fe_voxel_gi_trace_and_accumulate
 */
void fe_voxel_gi_trace_and_accumulate(fe_voxel_gi_context_t* context) {
    if (!context || !context->tracing_material) return;
    
    FE_LOG_TRACE("Voxel Tracing & Accumulation Pass'i basladi.");

    // Bu aşama, Radiance Volume'unu kullanarak Dolaylı Işıklandırmayı hesaplar (Mipmap ve Cone Tracing).
    
    // 1. Compute Shader'ı kullan
    fe_material_bind(context->tracing_material);

    // 2. Volume'ları bağla (Oku/Yaz)
    // Radiance Volume'una okuma/yazma erişimi
    glBindImageTexture(0, context->grid.radiance_volume_id, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA16F);
    
    // 3. Compute Shader'ı Çalıştır (Sadece ışıklandırılan bölgeyi hedef alabilir)
    uint32_t num_groups = context->grid.resolution / 8;
    glDispatchCompute(num_groups, num_groups, num_groups);
    
    // 4. Shader'ın bitmesini bekle
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    
    // 5. Bağlantıları çöz
    glBindImageTexture(0, 0, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA16F);
    fe_shader_unuse();

    FE_LOG_TRACE("Voxel Tracing tamamlandi.");
}