// src/graphics/fe_shader_compiler.c

#include "graphics/fe_shader_compiler.h"
#include "utils/fe_logger.h"
#include <raylib.h>     // LoadShader, UnloadShader, BeginShaderMode, vb. için
#include <rlgl.h>       // rlglGetUniformLocation, rlSetShaderValue gibi düşük seviye GL sarmalayıcıları için
#include <string.h>     // strcmp için
#include <stdlib.h>     // calloc için

// ----------------------------------------------------------------------
// 1. SHADER YÖNETİM HAVUZU
// ----------------------------------------------------------------------

// Basit bir Shader havuzu (ileride hash tablosu veya vektör ile değiştirilir)
#define MAX_SHADERS 128
static fe_shader_t s_shader_pool[MAX_SHADERS] = {0};
static fe_shader_id_t s_next_shader_id = 1;

// Harici kütüphanelerden gelen ID'yi alabilmek için geçici bir değişken
static Shader s_current_raylib_shader = {0}; 


// ----------------------------------------------------------------------
// 2. ARABİRİM UYGULAMALARI
// ----------------------------------------------------------------------

/**
 * Uygulama: fe_shader_load
 */
fe_shader_id_t fe_shader_load(const char* vs_path, const char* fs_path) {
    if (s_next_shader_id >= MAX_SHADERS) {
        FE_LOG_ERROR("Shader havuzu doldu! Yeni shader derlenemiyor: %s / %s", vs_path, fs_path);
        return 0;
    }
    
    // Raylib'in Shader yükleyicisini kullan
    Shader rl_shader = LoadShader(vs_path, fs_path);

    // Raylib, derleme başarısız olursa ID=0 (varsayılan shader) döndürür
    if (rl_shader.id == 0) {
        FE_LOG_ERROR("Shader derleme/baglama hatasi: %s / %s", vs_path, fs_path);
        return 0;
    }

    // Havuza yeni bir fe_shader_t örneği oluştur
    fe_shader_t* new_shader = &s_shader_pool[s_next_shader_id];
    new_shader->id = rl_shader.id;
    
    // Adı kaydet (dosya adlarını birleştirerek)
    snprintf(new_shader->name, sizeof(new_shader->name), "%s_%s", GetFileName(vs_path), GetFileName(fs_path));
    
    // Motorun kendi ID'sini döndür
    FE_LOG_INFO("Shader derlendi ve baglandi: %s (ID: %u, GL ID: %u)", 
                new_shader->name, s_next_shader_id, new_shader->id);
    
    return s_next_shader_id++;
}

/**
 * Uygulama: fe_shader_unload
 */
void fe_shader_unload(fe_shader_id_t shader_id) {
    if (shader_id == 0 || shader_id >= s_next_shader_id) {
        FE_LOG_WARN("Gecersiz Shader ID'si (%u) kapatilmaya calisildi.", shader_id);
        return;
    }
    
    fe_shader_t* shader = &s_shader_pool[shader_id];
    
    // Raylib'in Shader kapatıcısını kullan
    UnloadShader(s_current_raylib_shader); 

    // Havuzdaki yeri temizle
    memset(shader, 0, sizeof(fe_shader_t));
    FE_LOG_DEBUG("Shader kapatildi (ID: %u).", shader_id);
    
    // Dikkat: Bu basit havuzlama, ID'leri yeniden kullanmaz. Gelişmiş sistemler bunu yapar.
}

/**
 * Uygulama: fe_shader_use
 */
void fe_shader_use(fe_shader_id_t shader_id) {
    if (shader_id == 0 || shader_id >= s_next_shader_id) {
        FE_LOG_ERROR("Gecersiz Shader ID'si (%u) kullanilmaya calisildi.", shader_id);
        return;
    }
    
    fe_shader_t* shader = &s_shader_pool[shader_id];
    
    // Raylib'in Shader kullanma fonksiyonunu çağır (rlglUseProgram kullanır)
    BeginShaderMode(s_current_raylib_shader); 
    
    s_current_raylib_shader.id = shader->id; // Global Raylib değişkenini motorun ID'si ile güncelle
}

/**
 * Uygulama: fe_shader_unuse
 */
void fe_shader_unuse(void) {
    // Raylib'in Shader kullanmayi sonlandirma fonksiyonunu çağır (glUseProgram(0) kullanır)
    EndShaderMode();
    s_current_raylib_shader.id = 0;
}


// ----------------------------------------------------------------------
// 3. UNIFORM AYARLAMA UYGULAMALARI (Aktif Shader'da calisir)
// ----------------------------------------------------------------------

/**
 * Uniform Konumunu bulma yardimci fonksiyonu
 */
static int fe_shader_get_location(const char* name) {
    if (s_current_raylib_shader.id == 0) {
        FE_LOG_ERROR("Uniform ayarlanmadan once shader aktif degil! (%s)", name);
        return -1;
    }
    // Raylib'in dahili fonksiyonu uniform konumunu bulur.
    return rlglGetLocationUniform(s_current_raylib_shader.id, name);
}

/**
 * Uygulama: fe_shader_set_uniform_float
 */
void fe_shader_set_uniform_float(const char* name, float value) {
    int loc = fe_shader_get_location(name);
    if (loc >= 0) {
        // Raylib'in uniform ayarlama fonksiyonunu kullan
        SetShaderValue(s_current_raylib_shader, loc, &value, SHADER_UNIFORM_FLOAT);
    }
}

/**
 * Uygulama: fe_shader_set_uniform_int
 */
void fe_shader_set_uniform_int(const char* name, int value) {
    int loc = fe_shader_get_location(name);
    if (loc >= 0) {
        // Raylib'in uniform ayarlama fonksiyonunu kullan (sampler'lar genellikle int'tir)
        SetShaderValue(s_current_raylib_shader, loc, &value, SHADER_UNIFORM_INT);
    }
}
