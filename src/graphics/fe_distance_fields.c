// src/graphics/dynamicr/fe_distance_fields.c

#include "graphics/dynamicr/fe_distance_fields.h"
#include "graphics/opengl/fe_gl_device.h"       // Doku ve Kaynak yönetimi için
#include "graphics/fe_shader_compiler.h"        // Compute Shader yüklemek için
#include "graphics/fe_material_editor.h"        // Materyal oluşturmak için
#include "utils/fe_logger.h"
#include <stdlib.h> // calloc, free için
#include <GL/gl.h>  // Compute Shader komutları için (GL_COMPUTE_SHADER, glDispatchCompute)

// Shader dosya yolları
#define SDF_COMPUTE_PATH "resources/shaders/dynamicr/sdf_builder.comp"


// ----------------------------------------------------------------------
// 1. DAHİLİ YARDIMCI FONKSİYONLAR
// ----------------------------------------------------------------------

/**
 * @brief SDF Volume'u icin 3D dokuyu (Texture Volume) olusturur.
 */
static fe_texture_id_t fe_distance_field_create_volume(uint32_t resolution) {
    fe_texture_id_t volume_id;
    
    // 1. Doku ID'si olustur
    glGenTextures(1, &volume_id);
    glBindTexture(GL_TEXTURE_3D, volume_id);
    
    // 2. Volume Ayarlari
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    // 3. Veri alanını ayır (R32F: 32 bit float, yani uzaklık)
    // GL_R32F veya GL_R16F (16 bit) kullanılabilir.
    glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, 
                 (GLsizei)resolution, (GLsizei)resolution, (GLsizei)resolution, 
                 0, GL_RED, GL_FLOAT, NULL);
    
    glBindTexture(GL_TEXTURE_3D, 0);
    return volume_id;
}


// ----------------------------------------------------------------------
// 2. ARABİRİM UYGULAMALARI
// ----------------------------------------------------------------------

/**
 * Uygulama: fe_distance_field_init
 */
fe_distance_field_t* fe_distance_field_init(fe_vec3_t world_min, fe_vec3_t world_max, uint32_t resolution) {
    FE_LOG_INFO("Distance Field baslatiliyor (Cozunurluk: %u^3)...", resolution);
    
    fe_distance_field_t* df = (fe_distance_field_t*)calloc(1, sizeof(fe_distance_field_t));
    if (!df) return NULL;

    df->world_min = world_min;
    df->world_max = world_max;
    df->resolution = resolution;

    // 1. 3D Uzaklık Volume'unu oluştur
    df->sdf_volume_id = fe_distance_field_create_volume(resolution);
    if (df->sdf_volume_id == 0) goto error_cleanup;

    // 2. Compute Shader'ı yükle
    fe_shader_id_t compute_shader_id = fe_shader_load_compute(SDF_COMPUTE_PATH);
    if (compute_shader_id == 0) goto error_cleanup;
    
    df->creation_material = fe_material_create(compute_shader_id);
    if (!df->creation_material) {
        fe_shader_unload(compute_shader_id);
        goto error_cleanup;
    }

    FE_LOG_INFO("Distance Field Volume ID: %u", df->sdf_volume_id);
    return df;

error_cleanup:
    if (df->sdf_volume_id != 0) { fe_gl_device_destroy_texture(df->sdf_volume_id); }
    if (df->creation_material) { fe_material_destroy(df->creation_material); }
    if (df) { free(df); }
    return NULL;
}

/**
 * Uygulama: fe_distance_field_shutdown
 */
void fe_distance_field_shutdown(fe_distance_field_t* df) {
    if (!df) return;

    // Kaynakları serbest bırak
    if (df->sdf_volume_id != 0) {
        fe_gl_device_destroy_texture(df->sdf_volume_id);
    }
    if (df->creation_material) {
        fe_material_destroy(df->creation_material);
    }

    free(df);
    FE_LOG_DEBUG("Distance Field kapatildi.");
}

/**
 * Uygulama: fe_distance_field_rebuild
 */
void fe_distance_field_rebuild(fe_distance_field_t* df, const fe_mesh_t* const* static_meshes, uint32_t mesh_count) {
    if (!df || !df->creation_material) {
        FE_LOG_ERROR("SDF yeniden olusturulamadi: Gecersiz baglam.");
        return;
    }

    FE_LOG_INFO("Distance Field yeniden olusturuluyor (%u mesh)...", mesh_count);

    // 1. Compute Shader'ı bağla
    fe_material_bind(df->creation_material);

    // 2. Volume'u Image Access olarak bağla (GL_IMAGE_BINDING)
    // Compute Shader'da yazma erişimi için Volume'u bağla (Image Load/Store)
    glBindImageTexture(0, df->sdf_volume_id, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_R32F);
    
    // 3. Uniform'ları Ayarla
    // SDF Shader'a Dünya boyutlarını ve çözünürlüğü gönder
    fe_shader_set_uniform_vec3("u_WorldMin", &df->world_min);
    fe_shader_set_uniform_vec3("u_WorldMax", &df->world_max);
    fe_shader_set_uniform_float("u_VolumeSize", df->world_max.x - df->world_min.x); // Basitlik icin

    // 4. Mesh verilerini Shader'a gönder (Storage Buffer Objects - SSBO ile yapılır)
    // NOTE: Gerçek bir uygulamada mesh verisi SSBO'lara yüklenmeli ve Compute Shader'a iletilmelidir.

    // 5. Compute Shader'ı Çalıştır
    uint32_t group_size = 8; // Compute Shader'daki local_size_x/y/z ile uyumlu olmalı
    uint32_t num_groups = df->resolution / group_size;
    
    // Tüm 3D Volume'u kapsayacak şekilde Dispatch çağrısı yap
    glDispatchCompute(num_groups, num_groups, num_groups);

    // 6. Shader'ın bitmesini bekle (Diğer GL komutlarından önce verinin hazır olması için)
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    // 7. Bağlantıları çöz
    glBindImageTexture(0, 0, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);
    fe_shader_unuse();
    
    FE_LOG_DEBUG("Distance Field olusturma tamamlandi.");
}