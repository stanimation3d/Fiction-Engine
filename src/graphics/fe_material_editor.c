// src/graphics/fe_material_editor.c

#include "graphics/fe_material_editor.h"
#include "graphics/fe_shader_compiler.h" // Shader programını kullanmak için
#include "graphics/opengl/fe_gl_backend.h" // GL Buffer/Texture ID'lerini kullanmak için
#include "utils/fe_logger.h"
#include <stdlib.h> // malloc, free için
#include <string.h> // memset, memcpy için
#include <raylib.h> // rlim_gl.h veya benzeri GL yardımcıları için (UBO oluşturma)
#include <rlgl.h> // Raylib'in GL sarmalayıcıları için


// ----------------------------------------------------------------------
// 1. YÖNETİM VERİLERİ
// ----------------------------------------------------------------------

// Basit bir malzeme havuzu (ileride hash tablosu veya vektör ile değiştirilir)
#define MAX_MATERIALS 1024
static fe_material_t* s_material_pool[MAX_MATERIALS] = {0};
static uint32_t s_next_material_id = 1;


// ----------------------------------------------------------------------
// 2. DAHİLİ FONKSİYONLAR
// ----------------------------------------------------------------------

/**
 * @brief GPU'da Uniform Buffer Object'i (UBO) başlatır.
 * * UBO'nun yapısı, malzemelerin PBR parametrelerini içerir (renkler, katsayılar).
 */
static fe_buffer_id_t fe_material_init_ubo() {
    // UBO'nun boyutu: albedo_color(vec4), metallic(float), roughness(float), emissive_color(vec3), padding
    size_t ubo_size = (4 * sizeof(float)) + (2 * sizeof(float)) + (3 * sizeof(float)) + (1 * sizeof(float)); // 44 bytes

    // Raylib'in GL sarmalayıcısını kullanarak UBO oluştur
    fe_buffer_id_t ubo_id = rlglGenBuffers();
    rlglBindBuffer(RL_UNIFORM_BUFFER, ubo_id);
    
    // UBO'yu boş veriyle başlat
    rlglBufferData(RL_UNIFORM_BUFFER, (int)ubo_size, NULL, RL_USAGE_DYNAMIC_DRAW); 
    
    rlglBindBuffer(RL_UNIFORM_BUFFER, 0);
    return ubo_id;
}


// ----------------------------------------------------------------------
// 3. ARABİRİM UYGULAMALARI
// ----------------------------------------------------------------------

/**
 * Uygulama: fe_material_create
 */
fe_material_t* fe_material_create(const char* name) {
    if (s_next_material_id >= MAX_MATERIALS) {
        FE_LOG_ERROR("Malzeme havuzu doldu! Yeni malzeme olusturulamiyor: %s", name);
        return NULL;
    }
    
    // Bellek ayırma
    fe_material_t* mat = (fe_material_t*)malloc(sizeof(fe_material_t));
    if (!mat) {
        FE_LOG_FATAL("Malzeme icin bellek tahsisi basarisiz.");
        return NULL;
    }
    
    // Varsayılan Değerler
    memset(mat, 0, sizeof(fe_material_t));
    mat->material_hash_id = s_next_material_id++;
    mat->albedo_color[0] = mat->albedo_color[1] = mat->albedo_color[2] = mat->albedo_color[3] = 1.0f; // Beyaz
    mat->metallic_value = 0.0f;
    mat->roughness_value = 0.8f;
    
    // GPU Kaynağını Başlat
    mat->ubo_id = fe_material_init_ubo();
    if (mat->ubo_id == 0) {
        FE_LOG_ERROR("Malzeme UBO'su olusturulamadi: %s", name);
        free(mat);
        return NULL;
    }

    // Havuza ekle
    s_material_pool[mat->material_hash_id - 1] = mat; 
    
    FE_LOG_DEBUG("Malzeme olusturuldu: %s (ID: %u)", name, mat->material_hash_id);
    
    // Başlangıç değerlerini GPU'ya yükle
    fe_material_upload_to_gpu(mat);
    
    return mat;
}

/**
 * Uygulama: fe_material_destroy
 */
void fe_material_destroy(fe_material_t* material) {
    if (!material) return;

    // GPU Kaynaklarını Serbest Bırak
    if (material->ubo_id != 0) {
        rlglDeleteBuffers(material->ubo_id);
    }
    
    // Havuzdan Kaldır ve Belleği Serbest Bırak
    if (material->material_hash_id > 0 && material->material_hash_id <= MAX_MATERIALS) {
        s_material_pool[material->material_hash_id - 1] = NULL;
    }
    
    free(material);
    FE_LOG_DEBUG("Malzeme yok edildi (ID: %u).", material->material_hash_id);
}

/**
 * Uygulama: fe_material_upload_to_gpu
 */
fe_error_code_t fe_material_upload_to_gpu(fe_material_t* material) {
    if (!material || material->ubo_id == 0) {
        return FE_ERR_INVALID_ARGUMENT;
    }

    // Uniform Buffer Veri Yapısı (Manuel hizalama gerektirebilir)
    // Şimdilik sadece verileri ardı ardına kopyalıyoruz
    struct {
        float albedo[4]; 
        float metallic; 
        float roughness; 
        float emissive[3];
        float padding; // Hizalama için (vec3'ten sonra)
    } gpu_data;
    
    memcpy(gpu_data.albedo, material->albedo_color, sizeof(float) * 4);
    gpu_data.metallic = material->metallic_value;
    gpu_data.roughness = material->roughness_value;
    memcpy(gpu_data.emissive, material->emissive_color, sizeof(float) * 3);
    gpu_data.padding = 0.0f; 

    // UBO'ya veriyi yükle
    rlglBindBuffer(RL_UNIFORM_BUFFER, material->ubo_id);
    rlglBufferSubData(RL_UNIFORM_BUFFER, 0, (int)sizeof(gpu_data), &gpu_data);
    rlglBindBuffer(RL_UNIFORM_BUFFER, 0);
    
    return FE_OK;
}

/**
 * Uygulama: fe_material_bind
 */
void fe_material_bind(const fe_material_t* material) {
    if (!material) return;

    // 1. Shader Programını Kullanıma Al (fe_shader_compiler'dan)
    if (material->shader_program_id != 0) {
        fe_shader_use(material->shader_program_id);
        
        // 2. Uniform Buffer'ı Bağla
        // UBO'nun 0. bağlama noktasına bağlandığını varsayıyoruz.
        rlglBindBufferBase(RL_UNIFORM_BUFFER, 0, material->ubo_id);
        
        // 3. Kaplamaları Bağla
        for (int i = 0; i < FE_TEX_SLOT_COUNT; i++) {
            if (material->texture_ids[i] != 0) {
                // Her slot için farklı bir doku birimi kullanılır
                rlglActiveTexture(RL_TEXTURE_UNIT_MAX + i); 
                rlglBindTexture(RL_TEXTURE_2D, material->texture_ids[i]);
                
                // Shader'da sampler uniform'u ayarla (fe_shader_compiler'da yapılır)
                // fe_shader_set_uniform_int(material->shader_program_id, material_texture_uniform_names[i], i);
            }
        }
    } else {
        FE_LOG_WARN("Malzeme (ID: %u) icin gecerli bir Shader ID'si yok!", material->material_hash_id);
    }
}

/**
 * Uygulama: fe_material_set_texture
 */
void fe_material_set_texture(fe_material_t* material, fe_material_texture_slot_t slot, fe_texture_id_t texture_id) {
    if (!material) return;
    if (slot < 0 || slot >= FE_TEX_SLOT_COUNT) {
        FE_LOG_WARN("Gecersiz doku slotu (%d) ayarlandi.", slot);
        return;
    }
    
    material->texture_ids[slot] = texture_id;
    FE_LOG_DEBUG("Malzeme (ID: %u) icin doku slotu %d ayarlandi (Tex ID: %u).", 
                 material->material_hash_id, slot, texture_id);
}