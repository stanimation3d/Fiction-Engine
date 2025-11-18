// src/physics/fe_physical_animation_component.c

#include "physics/fe_physical_animation_component.h"
#include "utils/fe_logger.h"
#include "data_structures/fe_array.h"
#include <stdlib.h> // malloc, free
#include <math.h>   // fminf, fmaxf

// Dahili Kuaterniyon ve Vektör Yardımcı Fonksiyonları (fe_math'ten geldiği varsayılır)
fe_vec4_t fe_quat_from_mat4(fe_mat4_t mat);
fe_vec4_t fe_quat_inverse(fe_vec4_t q);
fe_vec4_t fe_quat_multiply(fe_vec4_t a, fe_vec4_t b);
fe_vec3_t fe_quat_get_axis_angle(fe_vec4_t q, float* angle_out);
fe_vec3_t fe_vec3_clamp_magnitude(fe_vec3_t v, float max);

// ----------------------------------------------------------------------
// 1. YÖNETİM UYGULAMALARI
// ----------------------------------------------------------------------

static uint32_t g_next_phys_anim_id = 1;

/**
 * Uygulama: fe_physical_anim_create
 */
fe_physical_animation_component_t* fe_physical_anim_create(
    fe_ragdoll_t* ragdoll, 
    fe_animation_drive_settings_t default_settings) 
{
    if (!ragdoll) {
        FE_LOG_ERROR("Fiziksel animasyon icin Ragdoll hedefi gereklidir.");
        return NULL;
    }

    fe_physical_animation_component_t* comp = 
        (fe_physical_animation_component_t*)calloc(1, sizeof(fe_physical_animation_component_t));
        
    if (!comp) {
        FE_LOG_FATAL("Fiziksel Animasyon bileseni icin bellek ayrilamadi.");
        return NULL;
    }
    
    comp->id = g_next_phys_anim_id++;
    comp->target_ragdoll = ragdoll;
    comp->default_settings = default_settings;
    comp->is_active = true;
    
    // Hedef dönüsümleri tutmak için diziyi başlat (ragdoll kemik sayısına esitlenmelidir)
    comp->target_transforms = fe_array_create(sizeof(fe_mat4_t)); 
    
    FE_LOG_INFO("Fiziksel Animasyon Bileseni %u olusturuldu.", comp->id);
    return comp;
}

/**
 * Uygulama: fe_physical_anim_destroy
 */
void fe_physical_anim_destroy(fe_physical_animation_component_t* comp) {
    if (comp) {
        if (comp->target_transforms) {
            fe_array_destroy(comp->target_transforms);
        }
        free(comp);
        FE_LOG_TRACE("Fiziksel Animasyon Bileseni yok edildi.");
    }
}


// ----------------------------------------------------------------------
// 2. KUVVET UYGULAMASI (ANA MANTIK)
// ----------------------------------------------------------------------

/**
 * @brief Bir kemik (rigid body) için gerekli Tork'u hesaplar ve uygular.
 * * Bu, Gelişmiş PID (Oransal-İntegral-Türev) veya P-D (Oransal-Türev) kontrol mantığıdır.
 */
static void fe_physical_anim_apply_torque_drive(
    fe_rigid_body_t* rb, 
    fe_mat4_t target_transform, 
    const fe_animation_drive_settings_t* settings) 
{
    // Cisim kütlesiz, kinematik veya uyuyorsa geç
    if (rb->mass <= 0.0f || rb->is_kinematic || !rb->is_awake) return;

    // 1. Dönüşüm Hata Hesaplaması
    
    // Hedef Quaternion (q_t) ve Mevcut Quaternion (q_c)
    fe_vec4_t q_target = fe_quat_from_mat4(target_transform);
    fe_vec4_t q_current = rb->orientation;

    // Hata Kuaterniyonu: q_error = q_target * q_current_inverse
    // Bu, q_c'den q_t'ye ulaşmak için gereken dönüşüm farkını verir.
    fe_vec4_t q_current_inv = fe_quat_inverse(q_current);
    fe_vec4_t q_error = fe_quat_multiply(q_target, q_current_inv);
    
    // Hata Kuaterniyonundan Açısal Hata Vektörünü (axis * angle) çıkar
    float angle_error;
    fe_vec3_t axis_error = fe_quat_get_axis_angle(q_error, &angle_error);
    
    // Konum Hatası (Pozisyonu da ayarlamak isterseniz, bu kod eklenmeli:
    // fe_vec3_t pos_error = fe_vec3_subtract(target_transform.translation, rb->position); )
    
    // 2. Tork Hesaplaması (P-D Kontrolü)
    
    // A. Oransal Terim (Pozisyon Hatası): T_p = K * hata
    // T_p: Hatanın büyüklüğüne bağlı olarak tork uygular.
    fe_vec3_t proportional_torque = fe_vec3_scale(axis_error, settings->stiffness * angle_error);
    
    // B. Türev Terim (Hız Hatası): T_d = -D * (mevcut açısal hız)
    // T_d: Salınımı yavaşlatmak ve hedef hızı korumak için kullanılır.
    fe_vec3_t damping_torque = fe_vec3_scale(rb->angular_velocity, -settings->damping);
    
    // Toplam Tork = T_p + T_d
    fe_vec3_t total_torque = fe_vec3_add(proportional_torque, damping_torque);

    // 3. Tork Limiti Uygula ve Uygula
    total_torque = fe_vec3_clamp_magnitude(total_torque, settings->max_force);

    fe_rigid_body_apply_torque(rb, total_torque);
}


/**
 * Uygulama: fe_physical_anim_apply_drives
 */
void fe_physical_anim_apply_drives(fe_physical_animation_component_t* comp, float dt) {
    if (!comp->is_active || !comp->target_ragdoll->is_active) return;
    
    // Hedef ve mevcut kemik sayılarının eşit olması GEREKİR.
    size_t rb_count = fe_array_count(comp->target_ragdoll->rigid_bodies);
    size_t target_count = fe_array_count(comp->target_transforms);

    if (rb_count != target_count) {
        FE_LOG_ERROR_THROTTLE("Fiziksel Animasyon: Ragdoll kemik sayisi (%zu) ile hedef donusum sayisi (%zu) eslesmiyor.", rb_count, target_count);
        return;
    }
    
    // Her bir kemik (rigid body) için sürücü kuvvetini uygula
    for (size_t i = 0; i < rb_count; ++i) {
        fe_rigid_body_t** rb_ptr = (fe_rigid_body_t**)fe_array_get(comp->target_ragdoll->rigid_bodies, i);
        fe_mat4_t* target_mat_ptr = (fe_mat4_t*)fe_array_get(comp->target_transforms, i);

        if (!rb_ptr || !(*rb_ptr) || !target_mat_ptr) continue;

        // Tork Sürücüsünü Uygula (Pozisyonu animasyon yönüne zorlar)
        fe_physical_anim_apply_torque_drive(*rb_ptr, *target_mat_ptr, &comp->default_settings);
        
        // TODO: fe_physical_anim_apply_linear_drive (Konum farkı için doğrusal kuvvet)
    }
    
    
}

// ----------------------------------------------------------------------
// 3. YARDIMCI MATEMATİK FONKSİYON YER TUTUCULARI
// (Gerçek kodda fe_math.c'den alınacaktır)
// ----------------------------------------------------------------------
// Sadece örnekte derleme hatası vermemek için basit dönüşler.

fe_vec4_t fe_quat_from_mat4(fe_mat4_t mat) {
    // Karmaşık Quaternion çıkarma mantığı
    return (fe_vec4_t){0.0f, 0.0f, 0.0f, 1.0f}; 
}

fe_vec4_t fe_quat_inverse(fe_vec4_t q) {
    // x, y, z'yi ters çevir (w aynı kalır) ve normalize et
    return (fe_vec4_t){-q.x, -q.y, -q.z, q.w}; 
}

fe_vec4_t fe_quat_multiply(fe_vec4_t a, fe_vec4_t b) {
    // Kuaterniyon çarpımı
    return (fe_vec4_t){a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w}; 
}

fe_vec3_t fe_quat_get_axis_angle(fe_vec4_t q, float* angle_out) {
    // Kuaterniyonu Eksen-Açıya dönüştürme
    *angle_out = acosf(q.w) * 2.0f;
    return (fe_vec3_t){q.x, q.y, q.z};
}

fe_vec3_t fe_vec3_clamp_magnitude(fe_vec3_t v, float max) {
    float mag = fe_vec3_length(v);
    if (mag > max) {
        return fe_vec3_scale(fe_vec3_normalize(v), max);
    }
    return v;
}