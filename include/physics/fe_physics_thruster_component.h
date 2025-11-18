// include/physics/fe_physics_thruster_component.h

#ifndef FE_PHYSICS_THRUSTER_COMPONENT_H
#define FE_PHYSICS_THRUSTER_COMPONENT_H

#include <stdint.h>
#include <stdbool.h>
#include "math/fe_vector.h"
#include "physics/fe_rigid_body.h"

// ----------------------------------------------------------------------
// 1. İTİCİ YAPISI
// ----------------------------------------------------------------------

/**
 * @brief Bir katı cisme belirli bir yönde sürekli itme kuvveti uygulayan bilesen.
 */
typedef struct fe_physics_thruster_component {
    uint32_t id;
    fe_rigid_body_t* target_body;   // Kuvvetin uygulanacagi katı cisim (Hedef)
    
    float max_thrust;               // İticinin üretebileceği maksimum kuvvet (Newton)
    
    fe_vec3_t local_position;       // Kuvvetin uygulandigi noktanin cismin yerel uzayindaki konumu
    fe_vec3_t local_direction;      // İtme kuvvetinin yönü (cismin yerel uzayinda)
    
    float throttle_factor;          // Geçerli gaz/itme seviyesi (0.0 - 1.0 arasi çarpan)

    bool is_active;
} fe_physics_thruster_component_t;


// ----------------------------------------------------------------------
// 2. YÖNETİM VE UYGULAMA FONKSİYONLARI
// ----------------------------------------------------------------------

/**
 * @brief Yeni bir itici bileseni olusturur ve baslatir.
 * * @param target_body Kuvvetin uygulanacagi cisim.
 * @param max_thrust Maksimum itme kuvveti.
 * @param local_pos İtme noktasinin yerel konumu.
 * @param local_dir İtme yönünün yerel vektöru.
 * @return Baslatilmis itici pointer'i.
 */
fe_physics_thruster_component_t* fe_thruster_create(
    fe_rigid_body_t* target_body, 
    float max_thrust, 
    fe_vec3_t local_pos,
    fe_vec3_t local_dir);

/**
 * @brief İtici bilesenini bellekten serbest birakir.
 */
void fe_thruster_destroy(fe_physics_thruster_component_t* thruster);

/**
 * @brief İtici gaz seviyesini ayarlar (Kuvvet çarpanini).
 */
void fe_thruster_set_throttle(fe_physics_thruster_component_t* thruster, float factor);

/**
 * @brief İticinin ürettigi kuvveti hesaplar ve hedef cisme uygular.
 * * Bu, fe_physics_manager_step içinde çagrilir.
 */
void fe_thruster_apply_force(fe_physics_thruster_component_t* thruster);

#endif // FE_PHYSICS_THRUSTER_COMPONENT_H