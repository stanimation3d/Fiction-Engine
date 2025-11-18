// include/physics/fe_cloth_physics.h

#ifndef FE_CLOTH_PHYSICS_H
#define FE_CLOTH_PHYSICS_H

#include <stdint.h>
#include <stdbool.h>
#include "math/fe_vector.h"
#include "data_structures/fe_array.h" // Parçacık ve kısıtlama koleksiyonları için

// ----------------------------------------------------------------------
// 1. TEMEL YAPILAR
// ----------------------------------------------------------------------

/**
 * @brief Kumas simülasyonundaki tek bir parçacigi temsil eder.
 */
typedef struct fe_cloth_particle {
    fe_vec3_t position;         // Mevcut konum (Dünya uzayi)
    fe_vec3_t prev_position;    // Önceki konum (Konum tabanli entegrasyon için)
    fe_vec3_t velocity;         // Mevcut hiz
    fe_vec3_t force_accumulator;// Bu adimda uygulanan toplam kuvvet
    float mass;                 // Parçacigin kütlesi (0.0 ise sabitlenmis)
    bool is_fixed;              // Parçacik sabitlenmis mi (Örn: Bayrak diregi)
} fe_cloth_particle_t;

/**
 * @brief Iki parçacik arasindaki yay kısıtlamasını temsil eder.
 */
typedef struct fe_cloth_constraint {
    uint32_t p1_index;          // Birinci parçacigin indeksi
    uint32_t p2_index;          // Ikinci parçacigin indeksi
    float rest_length;          // Yayin serbest (ideal) uzunlugu
    float stiffness;            // Yay sertligi
    // Not: PBD yaklasiminda damping genellikle ayri ele alinir.
} fe_cloth_constraint_t;

// ----------------------------------------------------------------------
// 2. KUMAŞ YAPISI
// ----------------------------------------------------------------------

/**
 * @brief Tek bir kumas nesnesini (Mesh) temsil eder.
 */
typedef struct fe_cloth {
    uint32_t id;
    fe_array_t* particles;      // fe_cloth_particle_t türünde
    fe_array_t* constraints;    // fe_cloth_constraint_t türünde
    
    // Simülasyon Ayarlari
    fe_vec3_t wind_velocity;    // Kumaşa etki eden yerel rüzgar hizi
    float damping_factor;       // Hizin sönümleme çarpanı (0.0 - 1.0)
    uint32_t constraint_iterations; // Kısıtlama çözücü adim sayisi (Kaliteyi belirler)

} fe_cloth_t;


// ----------------------------------------------------------------------
// 3. YÖNETİM VE YAŞAM DÖNGÜSÜ FONKSİYONLARI
// ----------------------------------------------------------------------

/**
 * @brief Yeni bir kumas nesnesi olusturur.
 * * @param width_segments Kumasin genislik parçaları.
 * @param height_segments Kumasin yükseklik parçaları.
 * @param size_x Kumasin dünya uzayindaki genisligi.
 * @param size_y Kumasin dünya uzayindaki yüksekligi.
 */
fe_cloth_t* fe_cloth_create_plane(uint32_t width_segments, uint32_t height_segments, float size_x, float size_y);

/**
 * @brief Kumas yapisini bellekten serbest birakir.
 */
void fe_cloth_destroy(fe_cloth_t* cloth);

/**
 * @brief Kumas parçaciklarina bir noktayi sabitler.
 * * @param cloth Kumas nesnesi.
 * @param p_index Sabitlenecek parçacigin indeksi.
 */
void fe_cloth_fix_particle(fe_cloth_t* cloth, uint32_t p_index);

/**
 * @brief Kumas simülasyonunu tek bir sabit adimda ilerletir.
 * * Bu, fe_physics_manager_step içinde tüm kumaslar için çagrilacaktir.
 * @param cloth Kumas nesnesi.
 * @param gravity Yerçekimi kuvveti.
 * @param dt Zaman adimi.
 */
void fe_cloth_simulate_step(fe_cloth_t* cloth, fe_vec3_t gravity, float dt);


#endif // FE_CLOTH_PHYSICS_H
