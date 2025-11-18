// include/physics/fe_radial_force_component.h

#ifndef FE_RADIAL_FORCE_COMPONENT_H
#define FE_RADIAL_FORCE_COMPONENT_H

#include <stdint.h>
#include <stdbool.h>
#include "math/fe_vector.h"
#include "physics/fe_rigid_body.h"

// ----------------------------------------------------------------------
// 1. KUVVET TİPLERİ
// ----------------------------------------------------------------------

/**
 * @brief Radyal kuvvetin uygulama seklini tanımlar.
 */
typedef enum fe_radial_force_type {
    FE_RF_TYPE_IMPULSE,      // Anlık darbe (patlama) - Hiza etki eder.
    FE_RF_TYPE_CONTINUOUS,   // Sürekli kuvvet (manyetik alan) - Kuvvet birikimine etki eder.
    FE_RF_TYPE_COUNT
} fe_radial_force_type_t;

// ----------------------------------------------------------------------
// 2. RADYAL KUVVET YAPISI
// ----------------------------------------------------------------------

/**
 * @brief Bir noktadan yayılan radyal kuvvet/darbe bileşenini temsil eder.
 */
typedef struct fe_radial_force_component {
    uint32_t id;
    fe_radial_force_type_t type;
    
    fe_vec3_t position;         // Kuvvetin kaynagi (Dünya uzayi)
    float max_radius;           // Kuvvetin etki ettigi maksimum yariçap
    float strength;             // Merkezdeki maksimum kuvvet/darbe büyüklügü
    float falloff_exponent;     // Kuvvet azalmasinin egriligi (1.0 = dogrusal, 2.0 = karesel)
    
    bool pulls;                 // True ise merkeze çeker (implosion), False ise iter (explosion).
    bool is_active;
    
    // Yalnizca Impuls (Patlama) için kullanilir:
    bool applied;               // Impuls uygulandi mi? (True ise bilesen devre disi kalir)
} fe_radial_force_component_t;


// ----------------------------------------------------------------------
// 3. YÖNETİM VE UYGULAMA FONKSİYONLARI
// ----------------------------------------------------------------------

/**
 * @brief Yeni bir radyal kuvvet bileseni olusturur ve baslatir.
 */
fe_radial_force_component_t* fe_radial_force_create(
    fe_vec3_t position, 
    float max_radius, 
    float strength,
    fe_radial_force_type_t type);

/**
 * @brief Radyal kuvvet bilesenini bellekten serbest birakir.
 */
void fe_radial_force_destroy(fe_radial_force_component_t* rf_comp);

/**
 * @brief Bir katı cisme radyal kuvvet/darbe uygular.
 * * Bu, fe_physics_manager_step içinde tüm rigid body'ler için çagrilacaktir.
 */
void fe_radial_force_apply(
    fe_radial_force_component_t* rf_comp, 
    fe_rigid_body_t* rb, 
    float dt);

#endif // FE_RADIAL_FORCE_COMPONENT_H