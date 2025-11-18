// include/physics/fe_ragdoll_physics.h

#ifndef FE_RAGDOLL_PHYSICS_H
#define FE_RAGDOLL_PHYSICS_H

#include <stdint.h>
#include <stdbool.h>
#include "physics/fe_rigid_body.h"
#include "physics/fe_physics_constraint_component.h"
#include "data_structures/fe_array.h" // Kemik ve kısıtlamaların koleksiyonu için

// Ragdoll'daki kemiklerin maksimum sayısını tanımlayın (Örnek)
#define FE_MAX_RAGDOLL_BONES 32

// ----------------------------------------------------------------------
// 1. RAGDOLL YAPISI
// ----------------------------------------------------------------------

/**
 * @brief Bir karakter iskeletini temsil eden Ragdoll yapisi.
 */
typedef struct fe_ragdoll {
    uint32_t id;
    
    // Kemik Koleksiyonlari
    fe_array_t* rigid_bodies;      // fe_rigid_body_t* turunde kemikler (fe_physics_manager'a da eklenmeli)
    fe_array_t* constraints;       // fe_physics_constraint_component_t* turunde eklemler

    // Karakterle ilgili veriler (Animasyon sisteminden gelen referanslar)
    void* character_handle;        // Karakter/Animasyon bilesenine bir isaretçi (Gerekirse)

    bool is_active;                // Ragdoll etkin mi? (Fizik devraldi mi?)
    
} fe_ragdoll_t;


// ----------------------------------------------------------------------
// 2. YÖNETİM VE YAŞAM DÖNGÜSÜ FONKSİYONLARI
// ----------------------------------------------------------------------

/**
 * @brief Yeni bir ragdoll yapisi olusturur ve baslatir.
 * * @param character_handle Karakter/Animasyon bilesenine isaretçi (opsiyonel).
 */
fe_ragdoll_t* fe_ragdoll_create(void* character_handle);

/**
 * @brief Bir ragdoll yapisini bellekten serbest birakir.
 * * NOT: Içindeki rigid body ve constraint'ler de yok edilmelidir.
 */
void fe_ragdoll_destroy(fe_ragdoll_t* ragdoll);

/**
 * @brief Ragdoll'u etkinlestirir.
 * * Karakterin mevcut animasyon konumlarini kullanarak rigid body'leri baslatir.
 */
void fe_ragdoll_activate(fe_ragdoll_t* ragdoll);

/**
 * @brief Ragdoll'u devre disi birakir.
 * * Rigid body'leri ve kısıtlamaları durdurur (veya kinematige alir).
 */
void fe_ragdoll_deactivate(fe_ragdoll_t* ragdoll);

/**
 * @brief Ragdoll kemiklerini ve eklemlerini hazirlar (Ana Kurulum).
 * * Normalde bu fonksiyon, karakter iskeleti verilerini (bone transforms, joint limits) okur.
 */
void fe_ragdoll_setup_from_skeleton(fe_ragdoll_t* ragdoll, /* Skeletal veri parametreleri buraya gelecektir */);


#endif // FE_RAGDOLL_PHYSICS_H