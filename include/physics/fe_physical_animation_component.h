// include/physics/fe_physical_animation_component.h

#ifndef FE_PHYSICAL_ANIMATION_COMPONENT_H
#define FE_PHYSICAL_ANIMATION_COMPONENT_H

#include <stdint.h>
#include <stdbool.h>
#include "math/fe_vector.h"
#include "math/fe_matrix.h" 
#include "physics/fe_ragdoll_physics.h" // fe_ragdoll_t'yi kullanmak için

// ----------------------------------------------------------------------
// 1. ANİMASYON SÜRÜCÜ YAPILARI
// ----------------------------------------------------------------------

/**
 * @brief Fiziksel animasyondaki bir eklemin/kemigin sürücü ayarlarini tutar.
 */
typedef struct fe_animation_drive_settings {
    float stiffness;            // Hedef pozisyona çekme sertligi (K)
    float damping;              // Salinimi yavaslatma katsayisi (D)
    float max_force;            // Sürücünün uygulayabilecegi maksimum tork/kuvvet limiti
} fe_animation_drive_settings_t;

// ----------------------------------------------------------------------
// 2. ANA BİLEŞEN YAPISI
// ----------------------------------------------------------------------

/**
 * @brief Bir Ragdoll iskeletini animasyon pozisyonuna doğru sürmekten sorumlu bilesen.
 */
typedef struct fe_physical_animation_component {
    uint32_t id;
    fe_ragdoll_t* target_ragdoll;  // Kontrol edilecek Ragdoll referansi
    
    // Genellikle iskeletin her kemiği için farklı bir ayar gerekir.
    // Basitlik için tüm iskelet için tek bir varsayilan sürücü ayari kullanalım.
    fe_animation_drive_settings_t default_settings;

    bool is_active;                // Fiziksel animasyon etkin mi?

    // **Bu alanlar disaridan (Animasyon sisteminden) her karede doldurulur:**
    // Hedef dönüsümlerin (kemiklerin) dizisi. fe_ragdoll_t->rigid_bodies ile eslenir.
    fe_array_t* target_transforms; // fe_mat4_t turunde diziler (Kemiklerin dünya uzayindaki hedef pozisyonu ve yönelimi) 
    
} fe_physical_animation_component_t;


// ----------------------------------------------------------------------
// 3. YÖNETİM VE UYGULAMA FONKSİYONLARI
// ----------------------------------------------------------------------

/**
 * @brief Yeni bir fiziksel animasyon bileseni olusturur.
 */
fe_physical_animation_component_t* fe_physical_anim_create(
    fe_ragdoll_t* ragdoll, 
    fe_animation_drive_settings_t default_settings);

/**
 * @brief Bileseni bellekten serbest birakir.
 */
void fe_physical_anim_destroy(fe_physical_animation_component_t* comp);

/**
 * @brief Her sabit fizik adiminda, Ragdoll'un eklemlerine sürücü kuvvetlerini uygular.
 * * @param comp Bilesen.
 * @param dt Fizik zaman adimi.
 */
void fe_physical_anim_apply_drives(fe_physical_animation_component_t* comp, float dt);

#endif // FE_PHYSICAL_ANIMATION_COMPONENT_H