// src/physics/fe_hair_physics.c

#include "physics/fe_hair_physics.h"
#include "utils/fe_logger.h"
#include <stdlib.h> // malloc, free
#include <math.h>   // sqrtf, fmaxf, fabsf

// ----------------------------------------------------------------------
// 1. YÖNETİM VE YAŞAM DÖNGÜSÜ
// ----------------------------------------------------------------------

static uint32_t g_next_hair_comp_id = 1;

/**
 * Uygulama: fe_hair_create_component
 */
fe_hair_component_t* fe_hair_create_component(fe_rigid_body_t* head_rb) {
    if (!head_rb) {
        FE_LOG_ERROR("Saç bileseni olusturmak icin kafa Rigid Body referansi gereklidir.");
        return NULL;
    }
    
    fe_hair_component_t* comp = (fe_hair_component_t*)calloc(1, sizeof(fe_hair_component_t));
    if (!comp) return NULL;
    
    comp->id = g_next_hair_comp_id++;
    comp->head_rb = head_rb;
    comp->strands = fe_array_create(sizeof(fe_hair_strand_t));
    
    // Varsayılan Ayarlar
    comp->stiffness_stretch = 0.8f; 
    comp->stiffness_bend = 0.3f;
    comp->damping_factor = 0.98f;
    comp->iteration_count = 3; // Kısıtlama çözücüsü için 3-5 yeterli olabilir.
    comp->is_active = true;

    FE_LOG_INFO("Saç Bileseni %u olusturuldu.", comp->id);
    return comp;
}

/**
 * Uygulama: fe_hair_destroy_component
 */
void fe_hair_destroy_component(fe_hair_component_t* comp) {
    if (comp) {
        // Her teli serbest bırak
        for (size_t i = 0; i < fe_array_count(comp->strands); ++i) {
            fe_hair_strand_t* strand = (fe_hair_strand_t*)fe_array_get(comp->strands, i);
            if (strand->particles) fe_array_destroy(strand->particles);
        }
        fe_array_destroy(comp->strands);
        free(comp);
        FE_LOG_TRACE("Saç Bileseni yok edildi.");
    }
}

/**
 * Uygulama: fe_hair_add_strand
 */
fe_hair_strand_t* fe_hair_add_strand(fe_hair_component_t* comp, fe_vec3_t* initial_positions, uint32_t count) {
    if (count < 2 || count > FE_MAX_STRAND_PARTICLES) {
        FE_LOG_ERROR("Gecersiz parça sayisi (%u). Tel en az 2, en fazla %d parça içermelidir.", count, FE_MAX_STRAND_PARTICLES);
        return NULL;
    }

    fe_hair_strand_t new_strand = {0};
    new_strand.particles = fe_array_create(sizeof(fe_hair_particle_t));
    
    // Parçacıkları başlat
    for (uint32_t i = 0; i < count; ++i) {
        fe_hair_particle_t p = {0};
        p.position = p.prev_position = initial_positions[i];
        p.mass = 0.01f; // 10 gram
        p.inverse_mass = 1.0f / p.mass;
        fe_array_push(new_strand.particles, &p);
    }
    
    // Uzunluk kısıtlamalarını hesapla
    for (uint32_t i = 0; i < count - 1; ++i) {
        fe_vec3_t delta = fe_vec3_subtract(initial_positions[i], initial_positions[i + 1]);
        new_strand.rest_lengths[i] = fe_vec3_length(delta);
    }
    
    fe_array_push(comp->strands, &new_strand);
    return (fe_hair_strand_t*)fe_array_get(comp->strands, fe_array_count(comp->strands) - 1);
}

// ----------------------------------------------------------------------
// 3. KISITLAMA ÇÖZÜMÜ
// ----------------------------------------------------------------------

/**
 * @brief İki komşu parçacık arasındaki uzunluk (uzama) kısıtlamasını çözer (PBD).
 * * @param p1, p2 Parçacık pointer'ları.
 * @param rest_length İdeal uzunluk.
 * @param stiffness Sertlik çarpanı (0.0 - 1.0).
 */
static void fe_hair_resolve_stretch_constraint(fe_hair_particle_t* p1, fe_hair_particle_t* p2, float rest_length, float stiffness) {
    fe_vec3_t delta = fe_vec3_subtract(p1->position, p2->position);
    float current_length = fe_vec3_length(delta);
    
    if (current_length < 0.0001f) return; // Sıfır uzunluk
    
    // Hata: C = |p1-p2| - L0
    // C'nin türevi (grad): delta / |p1-p2|
    
    // Konum Düzeltme Vektörü: lambda = C / (|grad C|^2)
    float correction_ratio = (current_length - rest_length) / current_length * stiffness;
    fe_vec3_t correction = fe_vec3_scale(delta, correction_ratio);

    // Kök parça (p1) kinematik ise, p2 düzeltilir.
    // Saç simülasyonunda sadece ilk parça (kök) kinematiktir.
    if (p1->inverse_mass == 0.0f) {
        p2->position = fe_vec3_add(p2->position, correction);
    } 
    // Diğer durumda, her iki parça da eşit olarak düzeltilir (Basit yaklaşım)
    else {
        fe_vec3_t half_correction = fe_vec3_scale(correction, 0.5f);
        p1->position = fe_vec3_subtract(p1->position, half_correction);
        p2->position = fe_vec3_add(p2->position, half_correction);
    }
}

/**
 * @brief Üç parçacık (p1, p2, p3) arasındaki bükülme kısıtlamasını çözer.
 * * p2 ortadaki noktadır.
 */
static void fe_hair_resolve_bend_constraint(fe_hair_particle_t* p1, fe_hair_particle_t* p2, fe_hair_particle_t* p3, float stiffness) {
    // Genellikle kuaterniyonlar ve hedef açılar kullanılarak daha karmaşık çözülür.
    // Basit bir yaklaşımla, p1-p2 ve p2-p3 vektörleri arasındaki açıyı korumaya çalışalım.

    // A. Bükülme kısıtlamasını basitleştirilmiş yay modeli ile çöz
    // (PBD için daha karmaşık matris türevleri gerekir, burada uzunluk tabanlı yaklaşımla ilerleyelim.)
    // Saçta bükülme kısıtlaması genellikle komşu parçacıklar arasındaki uzama kısıtlamasından sonra uygulanır.
    
    // Bükülme için basit bir itme/çekme uygulanması:
    // Bu kısım, gerçekçi bir saç bükülmesi için oldukça karmaşık PBD koduna ihtiyaç duyar.
    // Simülasyon iskeleti gereklilikleri nedeniyle, bu fonksiyonun sadece yer tutucu olduğunu varsayalım.
    
    // Tork uygulama mantığı:
    if (stiffness > 0.0f) {
        // p1-p2 ve p2-p3 arasındaki açıyı ideal açısına yaklaştır
        // Bu, p1 ve p3'ü içeri/dışarı doğru hareket ettirmeyi gerektirir.
    }
}

// ----------------------------------------------------------------------
// 4. SİMÜLASYON DÖNGÜSÜ
// ----------------------------------------------------------------------

/**
 * Uygulama: fe_hair_simulate_step
 */
void fe_hair_simulate_step(fe_hair_component_t* comp, fe_vec3_t gravity, float dt) {
    if (!comp->is_active) return;
    
    fe_rigid_body_t* head = comp->head_rb;
    float dt_sq = dt * dt;

    // Her kılavuz telini işle
    for (size_t k = 0; k < fe_array_count(comp->strands); ++k) {
        fe_hair_strand_t* strand = (fe_hair_strand_t*)fe_array_get(comp->strands, k);
        size_t count = fe_array_count(strand->particles);

        if (count < 2) continue;

        // 1. Kök Noktayı (İlk Parçacık) Başın Pozisyonuna Sabitle
        // Kafa hareketine göre kök noktanın (P0) konumunu güncelle (Kinematik girdi)
        fe_hair_particle_t* root_p = (fe_hair_particle_t*)fe_array_get(strand->particles, 0);
        
        // Bu pozisyon, kafa rigid body'sinin (head) pozisyonu ve rotasyonu ile hesaplanmalıdır.
        // Basitçe: root_p->position = fe_transform_from_head(head, root_initial_local_pos);
        root_p->prev_position = root_p->position; // Önceki pozisyonu koru
        root_p->position = head->position; // Basitleştirilmiş kafa pozisyonu (Gerçekte local transform)


        // 2. Parçacıkların Yeni Konumunu Tahmin Et (Verlet Entegrasyonu)
        for (size_t i = 1; i < count; ++i) {
            fe_hair_particle_t* p = (fe_hair_particle_t*)fe_array_get(strand->particles, i);
            
            // Kuvvetler (Sadece Yerçekimi)
            fe_vec3_t acceleration = fe_vec3_scale(gravity, 1.0f);
            
            // Yeni Konum Tahmini (P_new = 2*P_curr - P_prev + a * dt^2)
            fe_vec3_t temp_pos = p->position;
            fe_vec3_t new_pos = fe_vec3_subtract(fe_vec3_scale(p->position, 2.0f), p->prev_position);
            new_pos = fe_vec3_add(new_pos, fe_vec3_scale(acceleration, dt_sq));
            
            // Hız Sönümleme (Damping): Hızı konum farkından hesapla ve uygula
            fe_vec3_t velocity = fe_vec3_scale(fe_vec3_subtract(new_pos, p->position), comp->damping_factor);
            new_pos = fe_vec3_add(p->position, velocity); // damped velocity * dt
            
            p->prev_position = temp_pos;
            p->position = new_pos;
        }

        // 3. Kısıtlama Çözümü (İteratif)
        for (uint32_t iter = 0; iter < comp->iteration_count; ++iter) {
            // A. Uzunluk Kısıtlamaları (Yapısal)
            for (uint32_t i = 0; i < count - 1; ++i) {
                fe_hair_particle_t* p1 = (fe_hair_particle_t*)fe_array_get(strand->particles, i);
                fe_hair_particle_t* p2 = (fe_hair_particle_t*)fe_array_get(strand->particles, i + 1);
                
                fe_hair_resolve_stretch_constraint(p1, p2, strand->rest_lengths[i], comp->stiffness_stretch);
            }
            
            // B. Bükülme Kısıtlamaları (Daha sert saçlar için)
            for (uint32_t i = 0; i < count - 2; ++i) {
                fe_hair_particle_t* p1 = (fe_hair_particle_t*)fe_array_get(strand->particles, i);
                fe_hair_particle_t* p2 = (fe_hair_particle_t*)fe_array_get(strand->particles, i + 1);
                fe_hair_particle_t* p3 = (fe_hair_particle_t*)fe_array_get(strand->particles, i + 2);
                
                fe_hair_resolve_bend_constraint(p1, p2, p3, comp->stiffness_bend); 
            }
        }
        
        // 4. Çarpışma Çözümü (Basit Sınır veya Kafa Çarpışması)
        // TODO: Her bir parçacığın kafa rigid body'si ile çarpışmasını kontrol et.
        
    }
    
    
}