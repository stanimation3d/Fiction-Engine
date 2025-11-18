// include/physics/fe_fluid_simulation.h

#ifndef FE_FLUID_SIMULATION_H
#define FE_FLUID_SIMULATION_H

#include <stdint.h>
#include <stdbool.h>
#include "math/fe_vector.h"
#include "data_structures/fe_array.h" 
#include "physics/fe_physics_world.h" // fe_vec3_t gravity gibi global verilere erişim için

// ----------------------------------------------------------------------
// 1. TEMEL SPH YAPILARI
// ----------------------------------------------------------------------

/**
 * @brief Akışkan simülasyonundaki tek bir parçacigi temsil eder.
 */
typedef struct fe_fluid_particle {
    fe_vec3_t position;         // Mevcut konum
    fe_vec3_t velocity;         // Mevcut hiz
    fe_vec3_t force_accumulator;// Bu adimda uygulanan toplam kuvvet
    
    float mass;                 // Parçacigin kütlesi
    float density;              // Hesaplanan yoğunluk (rho)
    float pressure;             // Hesaplanan basınç (P)
} fe_fluid_particle_t;

/**
 * @brief Bir akışkan hacmini ve SPH parametrelerini tutar.
 */
typedef struct fe_fluid_volume {
    uint32_t id;
    fe_array_t* particles;      // fe_fluid_particle_t türünde
    
    // SPH Ayarlari
    float smoothing_radius_h;   // Etkileşim yaricapi (h)
    float rest_density;         // Akışkanın durağan yoğunluğu (rho0)
    float stiffness_k;          // Basınç hesaplaması için sertlik katsayisi
    float viscosity_mu;         // Viskozite katsayisi
    
    // Akışkan Özellikleri
    fe_vec3_t volume_min;       // Hacim sinirlarinin minimum noktasi
    fe_vec3_t volume_max;       // Hacim sinirlarinin maksimum noktasi
    
    // Komşu Arama Hızlandırması için (Örn: Grid/Hücre)
    void* spatial_hash;         // Uzamsal karma tablosu (Hashing Grid) referansı
} fe_fluid_volume_t;


// ----------------------------------------------------------------------
// 2. YÖNETİM VE SİMÜLASYON FONKSİYONLARI
// ----------------------------------------------------------------------

/**
 * @brief Yeni bir akışkan hacmi olusturur ve parçaciklari baslatir.
 * * @param particle_count Baslatilacak parçacik sayisi.
 * @param volume_size Hacmin boyutlari (fe_vec3_t).
 */
fe_fluid_volume_t* fe_fluid_create_volume(uint32_t particle_count, fe_vec3_t volume_size);

/**
 * @brief Akışkan hacmini bellekten serbest birakir.
 */
void fe_fluid_destroy_volume(fe_fluid_volume_t* volume);

/**
 * @brief Akışkan simülasyonunu tek bir sabit adimda ilerletir.
 * * @param volume Simüle edilecek akışkan hacmi.
 * @param gravity Yerçekimi kuvveti.
 * @param dt Zaman adimi.
 */
void fe_fluid_simulate_step(fe_fluid_volume_t* volume, fe_vec3_t gravity, float dt);

#endif // FE_FLUID_SIMULATION_H