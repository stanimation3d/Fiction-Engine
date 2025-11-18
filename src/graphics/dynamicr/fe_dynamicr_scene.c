// src/graphics/dynamicr/fe_dynamicr_scene.c

#include "graphics/dynamicr/fe_dynamicr_scene.h"
#include "graphics/opengl/fe_gl_device.h" // FBO'lari ve Kaplamalari olusturmak icin
#include "utils/fe_logger.h"
#include <stdlib.h> // calloc, free için
#include <string.h> // memcpy için


// ----------------------------------------------------------------------
// 1. DAHİLİ YARDIMCI FONKSİYONLAR
// ----------------------------------------------------------------------

/**
 * @brief DynamicR icin gerekli G-Buffer'i olusturur.
 * * DynamicR'da genellikle 3 veya 4 renk tamponu kullanilir: 
 * * 0: Albedo (RGB) + Roughness/Metalness (A)
 * * 1: Normal (RGB) + Material ID (A)
 * * 2: Emissive/Position (HDR)
 */
static fe_framebuffer_t* fe_dynamicr_scene_create_gbuffer(int width, int height) {
    FE_LOG_INFO("DynamicR G-Buffer olusturuluyor (W:%d, H:%d)...", width, height);
    
    // NOTE: Gercek bir uygulamada bu, fe_gl_device'da soyutlanmis olmalidir.
    // Simdilik, fe_framebuffer_t yapisinin olusturuldugunu varsayalim.
    fe_framebuffer_t* gbuffer = (fe_framebuffer_t*)calloc(1, sizeof(fe_framebuffer_t));
    if (!gbuffer) return NULL;
    
    gbuffer->width = width;
    gbuffer->height = height;
    
    // G-Buffer'i olusturma ve kaplamalari ekleme mantigi buraya gelmelidir.
    // glGenFramebuffers(1, &gbuffer->fbo_id);
    // fe_gl_device_attach_texture_to_fbo(gbuffer->fbo_id, ...);
    
    FE_LOG_INFO("G-Buffer olusturuldu. Renk tamponu sayisi: 3.");
    return gbuffer;
}

static void fe_dynamicr_scene_destroy_gbuffer(fe_framebuffer_t* gbuffer) {
    if (!gbuffer) return;
    // fe_gl_device_destroy_framebuffer(gbuffer->fbo_id);
    // fe_gl_device_destroy_texture(gbuffer->color_texture_id[...]);
    free(gbuffer);
}


// ----------------------------------------------------------------------
// 2. ARABİRİM UYGULAMALARI
// ----------------------------------------------------------------------

/**
 * Uygulama: fe_dynamicr_scene_init
 */
fe_dynamicr_scene_t* fe_dynamicr_scene_init(int width, int height) {
    FE_LOG_INFO("DynamicR Scene baslatiliyor...");
    
    fe_dynamicr_scene_t* scene = (fe_dynamicr_scene_t*)calloc(1, sizeof(fe_dynamicr_scene_t));
    if (!scene) {
        FE_LOG_FATAL("DynamicR Scene icin bellek ayrilamadi.");
        return NULL;
    }

    // 1. G-Buffer'i baslat
    scene->gbuffer_fbo = fe_dynamicr_scene_create_gbuffer(width, height);
    if (!scene->gbuffer_fbo) goto error_cleanup;

    // 2. Alt sistemleri baslat
    scene->screen_tracing_ctx = fe_screen_tracing_init(width, height);
    if (!scene->screen_tracing_ctx) goto error_cleanup;
    
    // TODO: Voxel GI ve Combiner Context'leri de burada baslatilacaktir.
    
    FE_LOG_INFO("DynamicR Scene basariyla baslatildi.");
    return scene;

error_cleanup:
    FE_LOG_FATAL("DynamicR Scene baslatilirken hata olustu. Temizleniyor...");
    if (scene->screen_tracing_ctx) { fe_screen_tracing_shutdown(scene->screen_tracing_ctx); }
    if (scene->gbuffer_fbo) { fe_dynamicr_scene_destroy_gbuffer(scene->gbuffer_fbo); }
    free(scene);
    return NULL;
}

/**
 * Uygulama: fe_dynamicr_scene_shutdown
 */
void fe_dynamicr_scene_shutdown(fe_dynamicr_scene_t* scene) {
    if (!scene) return;

    FE_LOG_INFO("DynamicR Scene kapatiliyor.");
    
    // Alt sistemleri kapat
    if (scene->screen_tracing_ctx) {
        fe_screen_tracing_shutdown(scene->screen_tracing_ctx);
    }
    // TODO: Voxel GI ve Combiner Context'leri de burada kapatilacaktir.

    // G-Buffer'i kapat
    if (scene->gbuffer_fbo) {
        fe_dynamicr_scene_destroy_gbuffer(scene->gbuffer_fbo);
    }
    
    // Mesh ve Işık listelerini serbest birak (Eger yonetimleri buradaysa)
    if (scene->meshes) { free(scene->meshes); }
    if (scene->lights) { free(scene->lights); }

    free(scene);
    FE_LOG_DEBUG("DynamicR Scene kapatildi.");
}

/**
 * Uygulama: fe_dynamicr_scene_update
 */
void fe_dynamicr_scene_update(fe_dynamicr_scene_t* scene, 
                              const fe_mat4_t* view, 
                              const fe_mat4_t* proj) {
    if (!scene) return;

    // Kamera matrislerini guncelle
    memcpy(&scene->view_matrix, view, sizeof(fe_mat4_t));
    memcpy(&scene->projection_matrix, proj, sizeof(fe_mat4_t));

    // NOTE: Dinamik mesh/isik listeleri de burada guncellenir.
    
    // TODO: Voxel GI sistemine ait uniform buffer'lar da burada matrislerle guncellenmelidir.

    FE_LOG_TRACE("DynamicR Scene matrisleri guncellendi.");
}