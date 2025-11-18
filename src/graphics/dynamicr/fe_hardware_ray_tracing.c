// src/graphics/dynamicr/fe_hardware_ray_tracing.c

#include "graphics/dynamicr/fe_hardware_ray_tracing.h"
#include "graphics/opengl/fe_gl_device.h" // Buffer yönetimi için
#include "utils/fe_logger.h"
#include <stdlib.h> // calloc, free için
#include <GL/gl.h>

// ----------------------------------------------------------------------
// 1. UZANTI FONKSİYON İŞARETÇİLERİ (NV/AMD)
// ----------------------------------------------------------------------

// Gerçek bir motor için, bu fonksiyonlar uzantı yükleme aşamasında (fe_gl_backend_init) 
// fe_gl_backend'de tanimlanip burada 'extern' ile cagrilmalidir.
// Basitlik icin, burada soyut isimler kullaniyoruz.

typedef fe_buffer_id_t (*PFNGLCREATEACCELERATIONSTRUCTURENV)(uint32_t flags);
typedef void (*PFNGLBUILDACCELERATIONSTRUCTURENV)(fe_buffer_id_t as, uint32_t flags, uint32_t mesh_count, ...);
typedef uint64_t (*PFNGLGETACCELERATIONSTRUCTUREHANDLENV)(fe_buffer_id_t as);
typedef void (*PFNGLDISPATCHRAYSNV)(uint32_t hit_shader_count, uint32_t miss_shader_count, uint32_t width, uint32_t height);

// Varsayımsal Fonksiyon İşaretçileri (Gerçekte uzantıdan yüklenir)
// extern PFNGLCREATEACCELERATIONSTRUCTURENV glCreateAccelerationStructureNV;
// extern PFNGLBUILDACCELERATIONSTRUCTURENV glBuildAccelerationStructureNV;
// extern PFNGLGETACCELERATIONSTRUCTUREHANDLENV glGetAccelerationStructureHandleNV;
// extern PFNGLDISPATCHRAYSNV glDispatchRaysNV;

// ----------------------------------------------------------------------
// 2. ARABİRİM UYGULAMALARI
// ----------------------------------------------------------------------

/**
 * Uygulama: fe_hrt_init
 */
fe_hrt_context_t* fe_hrt_init(void) {
    FE_LOG_INFO("Hardware Ray Tracing sistemi baslatiliyor...");
    
    // Gerçek uygulamada: Uzantı kontrolü ve fonksiyon işaretçisi yükleme burada yapılmalıdır.
    // if (!glCreateAccelerationStructureNV) { ... hata logla ... }
    
    fe_hrt_context_t* context = (fe_hrt_context_t*)calloc(1, sizeof(fe_hrt_context_t));
    if (!context) {
        FE_LOG_FATAL("HRT baglami icin bellek ayrilamadi.");
        return NULL;
    }

    // TLAS varsayilan sifirla (TLAS, fe_hrt_update_tlas'te olusturulacaktir)
    context->tlas.tlas_buffer_id = 0;
    
    FE_LOG_INFO("HRT baglami olusturuldu. Uzantilarin yüklendigi varsayildi.");
    return context;
}

/**
 * Uygulama: fe_hrt_shutdown
 */
void fe_hrt_shutdown(fe_hrt_context_t* context) {
    if (!context) return;
    
    // TLAS tamponunu sil (BLAS'lar ayri ayri silinmelidir)
    if (context->tlas.tlas_buffer_id != 0) {
        // glDeleteAccelerationStructuresNV(1, &context->tlas.tlas_buffer_id); // Varsayimsal silme
        fe_gl_device_destroy_buffer(context->tlas.tlas_buffer_id); // Veya GL tamponunu sil
    }

    free(context);
    FE_LOG_DEBUG("HRT kapatildi.");
}

/**
 * Uygulama: fe_hrt_create_blas
 */
fe_blas_t fe_hrt_create_blas(const fe_mesh_t* mesh) {
    fe_blas_t blas = {0};

    if (!mesh || mesh->vertex_buffer_id == 0) {
        FE_LOG_ERROR("BLAS olusturulamadi: Gecersiz mesh.");
        return blas;
    }

    // 1. BLAS tamponu oluştur (fe_gl_device veya uzantı çağrısı ile)
    // blas.blas_buffer_id = glCreateAccelerationStructureNV(0); // Varsayimsal olarak AS tamponu
    // Şimdilik sadece bir yer tutucu Buffer ID atayalım:
    blas.blas_buffer_id = fe_gl_device_create_buffer(1024 * 1024, NULL, FE_BUFFER_USAGE_STATIC);

    // 2. BLAS'i inşa et (VBO/EBO'dan veriyi alarak)
    // glBuildAccelerationStructureNV(blas.blas_buffer_id, ... mesh verisi, GL_BUILD_MODE_UPDATE/REBUILD ...)

    // 3. GPU Handle'ını al
    // blas.gpu_handle = glGetAccelerationStructureHandleNV(blas.blas_buffer_id);
    blas.gpu_handle = (uint64_t)blas.blas_buffer_id; // Simülasyon

    FE_LOG_TRACE("BLAS olusturuldu (ID: %u, Handle: %llu)", blas.blas_buffer_id, blas.gpu_handle);
    return blas;
}

/**
 * Uygulama: fe_hrt_update_tlas
 */
void fe_hrt_update_tlas(fe_hrt_context_t* context, const fe_blas_t* blas_array, 
                        const fe_mat4_t* transform_array, uint32_t count) {
    if (!context) return;
    
    if (context->tlas.tlas_buffer_id == 0) {
        // TLAS'i ilk kez olustur
        // context->tlas.tlas_buffer_id = glCreateAccelerationStructureNV(0); // Varsayimsal olarak AS tamponu
        context->tlas.tlas_buffer_id = fe_gl_device_create_buffer(1024 * 1024 * 4, NULL, FE_BUFFER_USAGE_STATIC);
        // context->tlas.gpu_handle = glGetAccelerationStructureHandleNV(context->tlas.tlas_buffer_id);
    }

    // 1. Instance (Örnek) verilerini hazırla (BLAS Handle'ları ve Transform Matrisleri)
    // Instancelar SSBO veya özel bir GL tamponunda tutulur.

    // 2. TLAS'i inşa et/güncelle (Instance verilerini kullanarak)
    // glBuildAccelerationStructureNV(context->tlas.tlas_buffer_id, ... instance verisi, GL_BUILD_MODE_UPDATE/REBUILD ...)

    context->tlas.instance_count = count;
    FE_LOG_TRACE("TLAS guncellendi (Instance sayisi: %u).", count);
}

/**
 * Uygulama: fe_hrt_dispatch_rays
 */
void fe_hrt_dispatch_rays(fe_hrt_context_t* context, fe_framebuffer_t* output_fbo, 
                          uint32_t width, uint32_t height) {
    if (!context || context->tlas.tlas_buffer_id == 0) {
        FE_LOG_ERROR("Işin takibi gonderilemedi: TLAS mevcut degil.");
        return;
    }
    
    // 1. Hedef FBO'yu bağla
    fe_renderer_bind_framebuffer(output_fbo);
    
    // 2. Ray Tracing Pipeline'i bağla (RayGen/Miss/Hit shader'larını içeren)
    fe_shader_bind_ray_pipeline(context->ray_pipeline_id);

    // 3. Shader Binding Table (SBT) ve TLAS'i bağla
    fe_shader_set_tlas(context->tlas.tlas_buffer_id); 
    fe_shader_bind_sbt(context->sbt_buffer);

    // 4. Işınları gönder
    glDispatchRaysNV(1, 1, width, height); // RayGen sayisi, Miss sayisi, Genislik, Yukseklik
    
    FE_LOG_DEBUG("Isin Takibi Gonderildi (W: %u, H: %u).", width, height);

    // 5. Pipeline'ı çöz ve FBO'dan ayır.
}