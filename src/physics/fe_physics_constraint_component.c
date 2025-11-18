// src/physics/fe_physics_constraint_component.c

#include "physics/fe_physics_constraint_component.h"
#include "utils/fe_logger.h"
#include <stdlib.h> // malloc, free
#include <string.h> // memset

// Benzersiz kimlik sayacı
static uint32_t g_next_constraint_id = 1;


/**
 * Uygulama: fe_constraint_create
 */
fe_physics_constraint_component_t* fe_constraint_create(
    fe_constraint_type_t type, 
    fe_rigid_body_t* body_a, 
    fe_rigid_body_t* body_b) 
{
    if (!body_a) {
        FE_LOG_ERROR("Kısıtlama olusturmak için body_a gereklidir.");
        return NULL;
    }

    fe_physics_constraint_component_t* constraint = 
        (fe_physics_constraint_component_t*)calloc(1, sizeof(fe_physics_constraint_component_t));
        
    if (!constraint) {
        FE_LOG_FATAL("Kısıtlama icin bellek ayrilamadi.");
        return NULL;
    }

    constraint->id = g_next_constraint_id++;
    constraint->type = type;
    constraint->body_a = body_a;
    constraint->body_b = body_b;
    constraint->is_active = true;
    
    // Varsayilan anchor'lar: Cisimlerin kütle merkezleri
    constraint->anchor_a_local = (fe_vec3_t){0.0f, 0.0f, 0.0f};
    
    if (body_b) {
        constraint->anchor_b_local = (fe_vec3_t){0.0f, 0.0f, 0.0f};
    }
    
    FE_LOG_TRACE("Kısıtlama %u olusturuldu (Tip: %d).", constraint->id, type);
    return constraint;
}

/**
 * Uygulama: fe_constraint_destroy
 */
void fe_constraint_destroy(fe_physics_constraint_component_t* constraint) {
    if (constraint) {
        free(constraint);
        FE_LOG_TRACE("Kısıtlama yok edildi.");
    }
}

/**
 * Uygulama: fe_constraint_set_spring_data
 */
void fe_constraint_set_spring_data(
    fe_physics_constraint_component_t* constraint, 
    float rest_length, 
    float stiffness, 
    float damping) 
{
    if (!constraint || constraint->type != FE_CONSTRAINT_SPRING) {
        FE_LOG_WARN("Yay verisi atamasi yanlis kısıtlama tipinde deneniyor.");
        return;
    }
    
    constraint->properties.spring_data.rest_length = rest_length;
    constraint->properties.spring_data.stiffness = stiffness;
    constraint->properties.spring_data.damping = damping;
}

// ----------------------------------------------------------------------
// Kısıtlama Çözümü (Gelecek - fe_physics_manager tarafından çağrılacak)
// ----------------------------------------------------------------------

/**
 * @brief Yalnizca bir yay kısıtlamasi için gerekli kuvveti hesaplar ve uygular.
 * * Bu, kısıtlama çözücünün bir alt adimi olabilir.
 */
void fe_constraint_apply_spring_force(fe_physics_constraint_component_t* constraint) {
    if (!constraint->is_active || constraint->type != FE_CONSTRAINT_SPRING) return;
    
    fe_rigid_body_t *rb_a = constraint->body_a;
    fe_rigid_body_t *rb_b = constraint->body_b;
    
    // Basitlik için sadece cisimlerin merkezlerini kullanalım (Anchor'lar kullanılmadan)
    fe_vec3_t pos_a = rb_a->position;
    fe_vec3_t pos_b = rb_b ? rb_b->position : (fe_vec3_t){0.0f, 0.0f, 0.0f}; // Eğer B yoksa (Dünyaya sabit)

    fe_vec3_t delta = fe_vec3_subtract(pos_a, pos_b);
    float current_length = fe_vec3_length(delta);

    // Yayin uzama miktari: x = L - L0
    float stretch = current_length - constraint->properties.spring_data.rest_length;

    // Hooke Yasasi: F = -k * x
    float stiffness_force_mag = -constraint->properties.spring_data.stiffness * stretch;
    
    // Kuvvet yönü (normalize edilmiş delta)
    fe_vec3_t direction = fe_vec3_normalize(delta);

    // Toplam Yay Kuvveti (Basit kuvvet: sadece sertlik)
    fe_vec3_t spring_force = fe_vec3_scale(direction, stiffness_force_mag);

    // Kuvvetleri rigid body'lere uygula
    fe_rigid_body_apply_force(rb_a, spring_force);
    
    if (rb_b) {
        // İkinci cisme zıt kuvvet uygula
        fe_rigid_body_apply_force(rb_b, fe_vec3_scale(spring_force, -1.0f));
    }
    
    // Not: Gerçek bir kısıtlama çözücüde sürtünme (damping) ve kısıtlama çözümü (IMPULSE) kullanılır.
}