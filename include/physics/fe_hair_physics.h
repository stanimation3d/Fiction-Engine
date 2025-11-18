// include/physics/fe_hair_physics.h

#ifndef FE_HAIR_PHYSICS_H
#define FE_HAIR_PHYSICS_H

#include <stdint.h>
#include <stdbool.h>
#include "math/fe_vector.h"
#include "data_structures/fe_array.h" 
#include "physics/fe_rigid_body.h" // Kafa rigid body'sine bağlamak için

// Tek bir saç telindeki maksimum parça sayısı
#define FE_MAX_STRAND_PARTICLES 30 

// ----------------------------------------------------------------------
// 1. TEMEL YAPILAR
// ----------------------------------------------------------------------

/**
 * @brief Bir saç telindeki tek bir simülasyon parçacığını temsil eder.
 */
typedef struct fe_hair_particle {
    fe_vec3_t position;         // Mevcut konum
    fe_vec3_t prev_position;    // Önceki konum (Verlet/PBD için)
    fe_vec3_t velocity;         // Mevcut hız
    float mass;                 // Parçacığın kütlesi (genellikle sabit)
    float inverse_mass;         // 1/kütle
} fe_hair_particle_t;

/**
 * @brief Tek bir saç telini (kılavuz teli) temsil eder.
 */
typedef struct fe_hair_strand {
    fe_array_t* particles;      // fe_hair_particle_t türünde dizisi
    float rest_lengths[FE_MAX_STRAND_PARTICLES - 1]; // İki komşu parça arasındaki ideal uzunluklar
    
    // Bükülme kısıtlamaları için orijinal açılar veya kuaterniyonlar burada tutulabilir.
    fe_vec3_t initial_tangents[FE_MAX_STRAND_PARTICLES - 2]; 
} fe_hair_strand_t;


// ----------------------------------------------------------------------
// 2. SAÇ FİZİĞİ BİLEŞENİ
// ----------------------------------------------------------------------

/**
 * @brief Bir karakter modeline bağlı tüm saç simülasyonunu yönetir.
 */
typedef struct fe_hair_component {
    uint32_t id;
    fe_rigid_body_t* head_rb;       // Saçın bağlı olduğu kafa Rigid Body'si (kinematik girdi)
    
    fe_array_t* strands;            // fe_hair_strand_t türünde kılavuz teller dizisi
    
    // Simülasyon Ayarları
    float stiffness_stretch;        // Uzunluk kısıtlaması sertliği (0.0 - 1.0)
    float stiffness_bend;           // Bükülme kısıtlaması sertliği (0.0 - 1.0)
    float damping_factor;           // Hız sönümleme çarpanı
    uint32_t iteration_count;       // Kısıtlama çözücü adım sayısı
    
    bool is_active;
} fe_hair_component_t;


// ----------------------------------------------------------------------
// 3. YÖNETİM VE SİMÜLASYON FONKSİYONLARI
// ----------------------------------------------------------------------

/**
 * @brief Yeni bir saç bileşeni olusturur.
 */
fe_hair_component_t* fe_hair_create_component(fe_rigid_body_t* head_rb);

/**
 * @brief Saç bileşenini bellekten serbest birakir.
 */
void fe_hair_destroy_component(fe_hair_component_t* comp);

/**
 * @brief Saç bileşenine yeni bir kılavuz tel (strand) ekler ve başlatır.
 * * @param comp Saç bileşeni.
 * @param initial_positions Parçacıkların başlangıç dünya konumları (fe_vec3_t dizisi).
 */
fe_hair_strand_t* fe_hair_add_strand(fe_hair_component_t* comp, fe_vec3_t* initial_positions, uint32_t count);

/**
 * @brief Saç simülasyonunu tek bir sabit adimda ilerletir.
 * * @param comp Saç bileşeni.
 * @param gravity Yerçekimi kuvveti.
 * @param dt Zaman adimi.
 */
void fe_hair_simulate_step(fe_hair_component_t* comp, fe_vec3_t gravity, float dt);

#endif // FE_HAIR_PHYSICS_H