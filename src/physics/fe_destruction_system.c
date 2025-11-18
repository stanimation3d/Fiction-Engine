// src/physics/fe_destruction_system.c

#include "physics/fe_destruction_system.h"
#include "utils/fe_logger.h"
#include <stdlib.h> // malloc, free
#include <math.h>   // sqrtf, fabsf, fmaxf

// Harici Fonksiyon Bildirimleri (fe_physics_manager'dan gelmesi beklenir)
// Bu fonksiyonlar, yıkım sonrası parçaları ana simülasyona eklemek için zorunludur.
void fe_physics_manager_remove_rigid_body(fe_rigid_body_t* rb);
void fe_physics_manager_add_rigid_body(fe_rigid_body_t* rb);

// ----------------------------------------------------------------------
// 1. YÖNETİM UYGULAMALARI
// ----------------------------------------------------------------------

static uint32_t g_next_dest_comp_id = 1;

/**
 * Uygulama: fe_destruction_create_component
 */
fe_destructible_component_t* fe_destruction_create_component(
    fe_rigid_body_t* target_body, 
    fe_fracture_mesh_t* fracture_data, 
    float health, 
    float threshold) 
{
    if (!target_body || !fracture_data) {
        FE_LOG_ERROR("Yikilabilir bilesen olusturmak icin hedef ve yikim verisi gereklidir.");
        return NULL;
    }
    
    fe_destructible_component_t* comp = 
        (fe_destructible_component_t*)calloc(1, sizeof(fe_destructible_component_t));
        
    if (!comp) {
        FE_LOG_FATAL("Yikilabilir bilesen icin bellek ayrilamadi.");
        return NULL;
    }

    comp->id = g_next_dest_comp_id++;
    comp->target_body = target_body;
    comp->fracture_data = fracture_data;
    comp->health = fmaxf(1.0f, health);
    comp->impulse_threshold = fmaxf(0.1f, threshold);
    comp->kinetic_energy_threshold = 0.0f; // Varsayılan olarak kullanılmaz

    FE_LOG_INFO("Yikilabilir Bilesen %u olusturuldu. Sağlık: %.1f", comp->id, comp->health);
    return comp;
}

/**
 * Uygulama: fe_destruction_destroy_component
 */
void fe_destruction_destroy_component(fe_destructible_component_t* dest_comp) {
    if (dest_comp) {
        // Not: fracture_data'nin ömrü başka bir sistemde yönetilmelidir.
        free(dest_comp);
        FE_LOG_TRACE("Yikilabilir Bilesen yok edildi.");
    }
}

// ----------------------------------------------------------------------
// 2. HASAR VE KONTROL
// ----------------------------------------------------------------------

/**
 * Uygulama: fe_destruction_apply_impulse
 * * Ana rigid body'nin çarpışma anında hasar alması
 */
void fe_destruction_apply_impulse(fe_destructible_component_t* dest_comp, float impulse_magnitude) {
    if (dest_comp->is_destroyed || dest_comp->is_pending_destruction) return;

    // Hasar al (Darbeyi enerji olarak al)
    dest_comp->health -= impulse_magnitude * 0.1f; // Darbeye göre basit hasar modeli

    // Eşik kontrolü
    if (impulse_magnitude >= dest_comp->impulse_threshold || dest_comp->health <= 0.0f) {
        dest_comp->is_pending_destruction = true;
        FE_LOG_WARNING("Yikim tetiklendi! Darbe: %.1f", impulse_magnitude);
    }
}

/**
 * Uygulama: fe_destruction_check_and_process
 */
void fe_destruction_check_and_process(fe_destructible_component_t* dest_comp) {
    if (dest_comp->is_destroyed) return;

    // A. Kinetik Enerji ile kontrol (Eğer ayarlanmışsa)
    if (dest_comp->kinetic_energy_threshold > 0.0f) {
        fe_rigid_body_t* rb = dest_comp->target_body;
        // Kinetik Enerji: KE = 0.5 * m * |v|^2
        float velocity_sq = fe_vec3_dot(rb->linear_velocity, rb->linear_velocity);
        float kinetic_energy = 0.5f * rb->mass * velocity_sq;

        if (kinetic_energy >= dest_comp->kinetic_energy_threshold) {
            dest_comp->is_pending_destruction = true;
            FE_LOG_WARNING("Yikim Kinetik Enerji ile tetiklendi: %.1f", kinetic_energy);
        }
    }

    // B. Yıkım işlemi zaten tetiklenmişse
    if (dest_comp->is_pending_destruction) {
        fe_destruction_perform(dest_comp);
    }
}


// ----------------------------------------------------------------------
// 3. YIKIM İŞLEMİ
// ----------------------------------------------------------------------

/**
 * Uygulama: fe_destruction_perform
 */
void fe_destruction_perform(fe_destructible_component_t* dest_comp) {
    if (dest_comp->is_destroyed) return;
    
    fe_rigid_body_t* original_rb = dest_comp->target_body;
    fe_fracture_mesh_t* fracture_data = dest_comp->fracture_data;

    // 1. Orijinal Rigid Body'yi Fizik Simülasyonundan Kaldır
    // Bu, fe_physics_manager'ın ana listesinden kaldırılması demektir.
    fe_physics_manager_remove_rigid_body(original_rb);
    
    // Yıkılan objeyi görsel sistemden de kaldırılmak üzere işaretle (burada sadece is_kinematic=true yapalım)
    original_rb->is_kinematic = true;
    original_rb->is_awake = false; 

    // 2. Her bir parçayı simülasyona enjekte et
    size_t chunk_count = fe_array_count(fracture_data->chunks);
    for (size_t i = 0; i < chunk_count; ++i) {
        fe_fracture_chunk_t* chunk = (fe_fracture_chunk_t*)fe_array_get(fracture_data->chunks, i);
        fe_rigid_body_t* chunk_rb = chunk->rigid_body;

        // a. Ana cismin mevcut pozisyon ve hızını parçalara kopyala
        chunk_rb->position = fe_vec3_add(original_rb->position, chunk_rb->position); // Pozisyonu dünya uzayına taşı
        chunk_rb->linear_velocity = original_rb->linear_velocity;
        chunk_rb->angular_velocity = original_rb->angular_velocity;
        chunk_rb->orientation = original_rb->orientation;

        // b. Patlama Efekti (Rastgele dağılma kuvveti/dürtüsü)
        // Patlama merkezini orijinal cismin kütle merkezi yapalım
        fe_vec3_t center = original_rb->position;
        fe_vec3_t to_chunk = fe_vec3_subtract(chunk_rb->position, center);
        
        // Dağılma yönüne patlama dürtüsü uygula
        float explosion_magnitude = 10.0f; // Patlama şiddeti
        fe_vec3_t impulse_dir = fe_vec3_normalize(to_chunk);
        fe_vec3_t impulse = fe_vec3_scale(impulse_dir, explosion_magnitude * chunk_rb->mass);
        
        // Hızı ayarla (Delta V = Impulse / Kütle)
        fe_vec3_t delta_v = fe_vec3_scale(impulse, chunk_rb->inverse_mass);
        chunk_rb->linear_velocity = fe_vec3_add(chunk_rb->linear_velocity, delta_v);

        // c. Parçayı fizik simülasyonuna ekle
        fe_physics_manager_add_rigid_body(chunk_rb);
        chunk_rb->is_awake = true; // Uyanık başlat
    }
    
    dest_comp->is_destroyed = true;
    dest_comp->is_pending_destruction = false;
    FE_LOG_SUCCESS("Yikim tamamlandi. %zu yeni parça eklendi.", chunk_count);
    
    
}