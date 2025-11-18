// src/physics/fe_cloth_physics.c

#include "physics/fe_cloth_physics.h"
#include "utils/fe_logger.h"
#include <stdlib.h> // malloc, free
#include <math.h>   // sqrtf, fmaxf

// Benzersiz kimlik sayacı
static uint32_t g_next_cloth_id = 1;

// ----------------------------------------------------------------------
// 1. OLUŞTURMA VE KURULUM
// ----------------------------------------------------------------------

/**
 * @brief İki parçacik arasina bir yay kısıtlaması ekler.
 */
static void fe_cloth_add_constraint(fe_cloth_t* cloth, uint32_t i, uint32_t j, float stiffness) {
    if (i == j) return;

    // Uzaklığı hesapla ve rest_length olarak kullan
    fe_cloth_particle_t* p1 = (fe_cloth_particle_t*)fe_array_get(cloth->particles, i);
    fe_cloth_particle_t* p2 = (fe_cloth_particle_t*)fe_array_get(cloth->particles, j);
    
    if (!p1 || !p2) return;

    fe_vec3_t delta = fe_vec3_subtract(p1->position, p2->position);
    float rest_length = fe_vec3_length(delta);

    fe_cloth_constraint_t new_constraint = {
        .p1_index = i,
        .p2_index = j,
        .rest_length = rest_length,
        .stiffness = stiffness
    };
    fe_array_push(cloth->constraints, &new_constraint);
}

/**
 * Uygulama: fe_cloth_create_plane
 */
fe_cloth_t* fe_cloth_create_plane(uint32_t w, uint32_t h, float size_x, float size_y) {
    fe_cloth_t* cloth = (fe_cloth_t*)calloc(1, sizeof(fe_cloth_t));
    if (!cloth) return NULL;
    
    cloth->id = g_next_cloth_id++;
    cloth->particles = fe_array_create(sizeof(fe_cloth_particle_t));
    cloth->constraints = fe_array_create(sizeof(fe_cloth_constraint_t));
    cloth->damping_factor = 0.99f;
    cloth->constraint_iterations = 4; // Tipik olarak 4-8 arası kullanılır
    cloth->wind_velocity = (fe_vec3_t){0.0f, 0.0f, 0.0f};

    float particle_mass = 1.0f / (float)(w * h); // Toplam kütleyi 1.0f yap

    // 1. Parçacıkları Başlat
    for (uint32_t j = 0; j < h; ++j) {
        for (uint32_t i = 0; i < w; ++i) {
            fe_cloth_particle_t p = {0};
            p.mass = particle_mass;
            p.prev_position = p.position = (fe_vec3_t){
                (float)i * (size_x / (float)(w - 1)),
                0.0f,
                (float)j * (size_y / (float)(h - 1))
            };
            fe_array_push(cloth->particles, &p);
        }
    }

    // 2. Kısıtlamaları (Yayları) Ekle
    // Genel stiffness değerleri
    float structural_stiffness = 0.5f;
    float shear_stiffness = 0.3f;
    float bending_stiffness = 0.2f;

    for (uint32_t j = 0; j < h; ++j) {
        for (uint32_t i = 0; i < w; ++i) {
            uint32_t current_idx = i + j * w;
            
            // A. Yapısal (Structural) - Sağ ve Aşağı
            if (i < w - 1) fe_cloth_add_constraint(cloth, current_idx, current_idx + 1, structural_stiffness);
            if (j < h - 1) fe_cloth_add_constraint(cloth, current_idx, current_idx + w, structural_stiffness);

            // B. Makaslama (Shear) - Köşegenler (Çapraz)
            if (i < w - 1 && j < h - 1) {
                fe_cloth_add_constraint(cloth, current_idx, current_idx + w + 1, shear_stiffness); // Sağ-Aşağı
                fe_cloth_add_constraint(cloth, current_idx + 1, current_idx + w, shear_stiffness); // Sol-Aşağı (next row, prev col)
            }

            // C. Bükülme (Bending) - İki boşluk atlama
            if (i < w - 2) fe_cloth_add_constraint(cloth, current_idx, current_idx + 2, bending_stiffness); // Yatay
            if (j < h - 2) fe_cloth_add_constraint(cloth, current_idx, current_idx + 2 * w, bending_stiffness); // Dikey
        }
    }

    FE_LOG_INFO("Kumas %u olusturuldu. Parçacik: %zu, Kısıtlama: %zu.", 
                cloth->id, fe_array_count(cloth->particles), fe_array_count(cloth->constraints));
    return cloth;
}

/**
 * Uygulama: fe_cloth_destroy
 */
void fe_cloth_destroy(fe_cloth_t* cloth) {
    if (cloth) {
        if (cloth->particles) fe_array_destroy(cloth->particles);
        if (cloth->constraints) fe_array_destroy(cloth->constraints);
        free(cloth);
        FE_LOG_TRACE("Kumas %u yok edildi.", cloth->id);
    }
}

/**
 * Uygulama: fe_cloth_fix_particle
 */
void fe_cloth_fix_particle(fe_cloth_t* cloth, uint32_t p_index) {
    if (p_index >= fe_array_count(cloth->particles)) return;
    fe_cloth_particle_t* p = (fe_cloth_particle_t*)fe_array_get(cloth->particles, p_index);
    if (p) {
        p->is_fixed = true;
        p->mass = 0.0f; // Sabit cisimler için kütle sıfır
    }
}

// ----------------------------------------------------------------------
// 2. SİMÜLASYON DÖNGÜSÜ
// ----------------------------------------------------------------------

/**
 * @brief Tek bir kısıtlamayı çözer (Yay uzunluğunu zorla korur).
 * * Basit Position Based Dynamics (PBD) yaklaşımını kullanır.
 */
static void fe_cloth_resolve_constraint(fe_cloth_t* cloth, fe_cloth_constraint_t* constraint) {
    fe_cloth_particle_t* p1 = (fe_cloth_particle_t*)fe_array_get(cloth->particles, constraint->p1_index);
    fe_cloth_particle_t* p2 = (fe_cloth_particle_t*)fe_array_get(cloth->particles, constraint->p2_index);

    if (!p1 || !p2) return;
    
    fe_vec3_t delta = fe_vec3_subtract(p1->position, p2->position);
    float current_length = fe_vec3_length(delta);
    
    // Uzaklık hatası: length_diff = (current - rest)
    float length_diff = current_length - constraint->rest_length;
    
    if (fabsf(length_diff) < 0.0001f) return; // Hata yok

    // Hata yönü
    fe_vec3_t direction = fe_vec3_normalize(delta);

    // Düzeltme faktörü (PBD yaklaşıma benziyor)
    // C = (P2 - P1) * (1 - L0/L)
    // S = C * 0.5 * stiffness (Basit yaklaşımla)
    float correction_magnitude = length_diff * 0.5f * constraint->stiffness;
    fe_vec3_t correction_vector = fe_vec3_scale(direction, correction_magnitude);

    // P1'i düzelt
    if (!p1->is_fixed) {
        p1->position = fe_vec3_subtract(p1->position, correction_vector);
    }
    // P2'yi düzelt (Ters yönde)
    if (!p2->is_fixed) {
        p2->position = fe_vec3_add(p2->position, correction_vector);
    }
}

/**
 * Uygulama: fe_cloth_simulate_step
 */
void fe_cloth_simulate_step(fe_cloth_t* cloth, fe_vec3_t gravity, float dt) {
    if (!cloth) return;

    // 1. Kuvvetleri Uygula (Yerçekimi ve Rüzgar)
    for (size_t i = 0; i < fe_array_count(cloth->particles); ++i) {
        fe_cloth_particle_t* p = (fe_cloth_particle_t*)fe_array_get(cloth->particles, i);
        if (p->is_fixed) continue;

        p->force_accumulator = (fe_vec3_t){0};

        // Yerçekimi kuvveti
        fe_vec3_t grav_force = fe_vec3_scale(gravity, p->mass);
        p->force_accumulator = fe_vec3_add(p->force_accumulator, grav_force);

        // TODO: Rüzgar kuvveti ekle (Kumaşın normaline ve hızına bağlı olarak daha karmaşık hesaplama gerekir)
    }

    // 2. Entegrasyon (Verlet/Konum tabanlı)
    float dt_sq = dt * dt;
    for (size_t i = 0; i < fe_array_count(cloth->particles); ++i) {
        fe_cloth_particle_t* p = (fe_cloth_particle_t*)fe_array_get(cloth->particles, i);
        if (p->is_fixed) continue;

        // Geçici bir konum tabanlı entegrasyon (Basit Euler/Verlet karışımı)
        fe_vec3_t acceleration = fe_vec3_scale(p->force_accumulator, 1.0f / p->mass);
        
        // Yeni konum (Verlet integratörü)
        fe_vec3_t new_pos = p->position;
        new_pos = fe_vec3_subtract(fe_vec3_scale(new_pos, 2.0f), p->prev_position); // 2*P(t) - P(t-dt)
        new_pos = fe_vec3_add(new_pos, fe_vec3_scale(acceleration, dt_sq));       // + a * dt^2
        
        // Hızı hesapla ve sönümle (Damping)
        p->velocity = fe_vec3_scale(fe_vec3_subtract(new_pos, p->position), 1.0f / dt);
        p->velocity = fe_vec3_scale(p->velocity, cloth->damping_factor);
        
        p->prev_position = p->position;
        p->position = new_pos;
    }

    // 3. Kısıtlama Çözümü (İteratif)
    for (uint32_t iter = 0; iter < cloth->constraint_iterations; ++iter) {
        for (size_t i = 0; i < fe_array_count(cloth->constraints); ++i) {
            fe_cloth_constraint_t* constraint = (fe_cloth_constraint_t*)fe_array_get(cloth->constraints, i);
            fe_cloth_resolve_constraint(cloth, constraint);
        }
    }

    // TODO: Çarpışma Tespiti ve Çözümü (Kumaş-Dünya, Kumaş-Katı Cisim, Kumaş-Kumaş)
    
    // 4. Hızı Konum Farkından Güncelle (PBD'de Konum -> Hız)
    for (size_t i = 0; i < fe_array_count(cloth->particles); ++i) {
        fe_cloth_particle_t* p = (fe_cloth_particle_t*)fe_array_get(cloth->particles, i);
        if (p->is_fixed) continue;
        
        // Hız: (P_new - P_old) / dt
        p->velocity = fe_vec3_scale(fe_vec3_subtract(p->position, p->prev_position), 1.0f / dt);
    }
    
    
}
