// include/physics/fe_physics_manager.h

#ifndef FE_PHYSICS_MANAGER_H
#define FE_PHYSICS_MANAGER_H

#include <stdint.h>
#include <stdbool.h>
#include "math/fe_vector.h"
#include "physics/fe_rigid_body.h"
#include "data_structures/fe_array.h" // Dinamik dizi yönetimi için (varsayılır)

// ----------------------------------------------------------------------
// 1. SABİT AYARLAR
// ----------------------------------------------------------------------

/**
 * @brief Fizik simülasyonunun sabit zaman adimi (saniye). 
 * * 60 FPS simülasyon hizi icin.
 */
#define FE_PHYSICS_FIXED_DT (1.0f / 60.0f) 

// ----------------------------------------------------------------------
// 2. YÖNETİCİ YAPISI
// ----------------------------------------------------------------------

/**
 * @brief Fizik simülasyonunun ana yöneticisi ve durum deposu.
 */
typedef struct fe_physics_manager {
    // Simülasyon Ayarlari
    fe_vec3_t gravity;             // Dünya yerçekimi vektörü (m/s^2)

    // Cisim Koleksiyonlari
    fe_array_t* rigid_bodies;      // fe_rigid_body_t* turunde isaretciler dizisi
    // fe_array_t* constraints;     // fe_physics_constraint_t*

    // Zamanlama
    float accumulator;             // Fizik adimlarini yakalamak icin birikimci (Fixed Timestep)

} fe_physics_manager_t;


// ----------------------------------------------------------------------
// 3. YÖNETİM VE YAŞAM DÖNGÜSÜ FONKSİYONLARI
// ----------------------------------------------------------------------

/**
 * @brief Fizik sistemini baslatir (Bellek ayirma, varsayilan ayarlari yükleme).
 */
void fe_physics_manager_init(void);

/**
 * @brief Fizik sistemini kapatir (Bellek serbest birakma, cisimleri yok etme).
 */
void fe_physics_manager_shutdown(void);

/**
 * @brief Fizik dünyasina yeni bir katı cisim ekler.
 * * @param rb Eklenen cisim.
 */
void fe_physics_manager_add_rigid_body(fe_rigid_body_t* rb);

/**
 * @brief Fizik dünyasindan bir katı cismi kaldirir (Ama yok etmez).
 * * @param rb Kaldirilan cisim.
 */
void fe_physics_manager_remove_rigid_body(fe_rigid_body_t* rb);

/**
 * @brief Fizik simülasyonunu gunceller (Fixed Timestep mantigi burada calisir).
 * * @param delta_time Uygulama karesinden gelen degisken zaman adimi (saniye).
 */
void fe_physics_manager_update(float delta_time);

/**
 * @brief Fizik motorunun bir tek, sabit adimini yürütür.
 * * Bu fonksiyon fe_physics_manager_update icinden tekrar tekrar çagrilir.
 */
void fe_physics_manager_step(void);

#endif // FE_PHYSICS_MANAGER_H