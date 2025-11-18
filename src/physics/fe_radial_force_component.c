// src/physics/fe_radial_force_component.c

#include "physics/fe_radial_force_component.h"
#include "utils/fe_logger.h"
#include <stdlib.h> // malloc, free
#include <math.h>   // powf, fmaxf

// Benzersiz kimlik sayacı
static uint32_t g_next_radial_force_id = 1;

// ----------------------------------------------------------------------
// 1. OLUŞTURMA VE YOK ETME
// ----------------------------------------------------------------------

/**
 * Uygulama: fe_radial_force_create
 */
fe_radial_force_component_t* fe_radial_force_create(
    fe_vec3_t position, 
    float max_radius, 
    float strength,
    fe_radial_force_type_t type) 
{
    fe_radial_force_component_t* rf = (fe_radial_force_component_t*)calloc(1, sizeof(fe_radial_force_component_t));
    
    if (!rf) {
        FE_LOG_FATAL("Radyal kuvvet bileseni icin bellek ayrilamadi.");
        return NULL;
    }
    
    rf->id = g_next_radial_force_id++;
    rf->type = type;
    rf->position = position;
    rf->max_radius = fmaxf(0.01f, max_radius); // Sifir veya negatif olmasin
    rf->strength = strength;
    rf->falloff_exponent = 2.0f; // Varsayilan: Karesel azalma (1/r^2)
    rf->pulls = (strength < 0.0f); // Negatif güç çeker, pozitif iter.
    rf->is_active = true;
    rf->applied = false;

    FE_LOG_TRACE("Radyal Kuvvet Bileseni %u olusturuldu (Tip: %d).", rf->id, type);
    return rf;
}

/**
 * Uygulama: fe_radial_force_destroy
 */
void fe_radial_force_destroy(fe_radial_force_component_t* rf_comp) {
    if (rf_comp) {
        free(rf_comp);
        FE_LOG_TRACE("Radyal Kuvvet Bileseni yok edildi.");
    }
}

// ----------------------------------------------------------------------
// 2. HESAPLAMA VE UYGULAMA
// ----------------------------------------------------------------------

/**
 * @brief Uzakliga bagli olarak kuvvet çarpanini hesaplar.
 */
static float fe_radial_force_calculate_factor(const fe_radial_force_component_t* rf_comp, float distance) {
    if (distance >= rf_comp->max_radius) {
        return 0.0f; // Etki alanı dışında
    }
    
    // Normalize edilmiş mesafe (0: Merkez, 1: Yarıçap Sınırı)
    float normalized_dist = distance / rf_comp->max_radius;
    
    // Kuvvet, sınırda 0 olacak şekilde ters çevrilmiş, kuvvetli bir şekilde azalır.
    float factor = 1.0f - powf(normalized_dist, rf_comp->falloff_exponent);
    
    // Kuvvet merkezde 1, sinirda 0 olur (Eşitlik 1'e 1 patlama simülasyonlarında kullanılmaz, 
    // ama bu basit bir fizik bileşeni için iyi bir başlangıçtır).
    return fmaxf(0.0f, factor); 
}

/**
 * Uygulama: fe_radial_force_apply
 */
void fe_radial_force_apply(
    fe_radial_force_component_t* rf_comp, 
    fe_rigid_body_t* rb, 
    float dt) 
{
    if (!rf_comp->is_active || rb->is_kinematic || !rb->is_awake || rb->mass <= 0.0f) return;

    // Impuls ise ve zaten uygulanmışsa, atla
    if (rf_comp->type == FE_RF_TYPE_IMPULSE && rf_comp->applied) return;
    
    // 1. Uzaklık ve Yönü Hesapla
    fe_vec3_t delta = fe_vec3_subtract(rb->position, rf_comp->position);
    float distance = fe_vec3_length(delta);

    if (distance > rf_comp->max_radius) return;
    
    // 2. Kuvvet Yönünü Normalize Et
    fe_vec3_t direction = fe_vec3_normalize(delta);

    // 3. Azalma Çarpanını Hesapla
    float factor = fe_radial_force_calculate_factor(rf_comp, distance);

    // 4. Nihai Kuvvet Büyüklüğünü Hesapla
    float magnitude = fabsf(rf_comp->strength) * factor;

    // Yönü ayarla (Çekme/İtme)
    if (rf_comp->pulls) {
        // Çekme (Merkeze doğru)
        direction = fe_vec3_scale(direction, -1.0f);
    }

    // 5. Kuvveti/Darbeyi Uygula
    if (rf_comp->type == FE_RF_TYPE_IMPULSE) {
        // Darbe (Impulse) = F * dt (Anlık: F * 1, hız değişimi yaratır)
        fe_vec3_t impulse = fe_vec3_scale(direction, magnitude);
        
        // Hız değişimi (Delta V) = Impulse / Kütle
        fe_vec3_t delta_v = fe_vec3_scale(impulse, rb->inverse_mass);
        rb->linear_velocity = fe_vec3_add(rb->linear_velocity, delta_v);
        
        // Darbe uygulandı, devre dışı bırak
        rf_comp->applied = true;
        rf_comp->is_active = false;
        
        FE_LOG_TRACE("Impulse uygulandı: Mag=%.2f, RB: %.2f %.2f", magnitude, rb->position.x, rb->position.y);
    } else {
        // Sürekli Kuvvet (F=ma)
        fe_vec3_t force = fe_vec3_scale(direction, magnitude);
        
        // Kuvveti rigid body'nin toplam kuvvet birikimine ekle
        fe_rigid_body_apply_force(rb, force);
    }
}