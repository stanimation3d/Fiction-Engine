// src/physics/fe_physics_fields.c

#include "physics/fe_physics_fields.h"
#include "utils/fe_logger.h"
#include <stdlib.h> // malloc, free
#include <math.h>   // fmaxf, fminf, sqrtf

// Benzersiz kimlik sayacı
static uint32_t g_next_field_id = 1;

// ----------------------------------------------------------------------
// 1. OLUŞTURMA VE YOK ETME
// ----------------------------------------------------------------------

/**
 * Uygulama: fe_field_create
 */
fe_physics_field_t* fe_field_create(fe_field_type_t type, fe_field_shape_t shape) {
    fe_physics_field_t* field = (fe_physics_field_t*)calloc(1, sizeof(fe_physics_field_t));
    
    if (!field) {
        FE_LOG_FATAL("Fizik alani icin bellek ayrilamadi.");
        return NULL;
    }
    
    field->id = g_next_field_id++;
    field->type = type;
    field->shape = shape;
    field->is_active = true;
    field->falloff = 0.5f; // Varsayilan yumusak azalma
    
    // Varsayilan hacim boyutu
    field->size = (fe_vec3_t){10.0f, 10.0f, 10.0f};

    FE_LOG_TRACE("Fizik alani %u olusturuldu (Tip: %d).", field->id, type);
    return field;
}

/**
 * Uygulama: fe_field_destroy
 */
void fe_field_destroy(fe_physics_field_t* field) {
    if (field) {
        free(field);
        FE_LOG_TRACE("Fizik alani yok edildi.");
    }
}

// ----------------------------------------------------------------------
// 2. YARDIMCI FONKSİYONLAR
// ----------------------------------------------------------------------

/**
 * @brief Bir noktanin (rigid body) alan hacmi içinde olup olmadigini kontrol eder.
 * * Basitlik için sadece AABB (Küp) kontrolü uygulanmıştır.
 */
static bool fe_field_is_point_in_volume(const fe_physics_field_t* field, fe_vec3_t point) {
    // Küre ve diğer hacim tipleri burada genişletilecektir.
    if (field->shape != FE_FIELD_SHAPE_AABB) {
        return true; // Henüz uygulanmadıysa tüm dünyaya etki etsin (Basitlik için)
    }

    // AABB (Küp) Kontrolü
    fe_vec3_t half_size = fe_vec3_scale(field->size, 0.5f);
    fe_vec3_t min = fe_vec3_subtract(field->position, half_size);
    fe_vec3_t max = fe_vec3_add(field->position, half_size);
    
    if (point.x >= min.x && point.x <= max.x &&
        point.y >= min.y && point.y <= max.y &&
        point.z >= min.z && point.z <= max.z) 
    {
        return true;
    }
    return false;
}

/**
 * @brief Rigid Body'nin konumuna göre kuvvetteki azalma çarpanini hesaplar.
 * * Basit azalma (Falloff) modeli kullanılır.
 */
static float fe_field_calculate_falloff_factor(const fe_physics_field_t* field, fe_vec3_t point) {
    // Şimdilik sadece alanın içinde olup olmadığını kontrol edelim.
    // İleride, merkeze olan uzaklığa göre 0.0f'dan 1.0f'a kadar bir çarpan hesaplanabilir.
    return 1.0f; 
}


// ----------------------------------------------------------------------
// 3. KUVVET UYGULAMASI
// ----------------------------------------------------------------------

/**
 * @brief Belirli bir alan tipine göre kuvvei hesaplar.
 */
static fe_vec3_t fe_field_calculate_force(const fe_physics_field_t* field, fe_vec3_t rb_position) {
    switch (field->type) {
        case FE_FIELD_TYPE_VECTOR: {
            // Vektör Alanı (Rüzgar)
            fe_vec3_t force = fe_vec3_scale(field->properties.vector_data.direction, field->properties.vector_data.strength);
            return force;
        }
        case FE_FIELD_TYPE_RADIAL: {
            // Radyal Alan (Patlama/Çekim)
            fe_vec3_t direction = fe_vec3_subtract(rb_position, field->position);
            float distance_sq = fe_vec3_dot(direction, direction);
            
            // Eğer cisim tam merkezdeyse (0 mesafe), kuvveti sıfırla
            if (distance_sq < 0.000001f) return (fe_vec3_t){0.0f, 0.0f, 0.0f};

            // F = G * m1 * m2 / r^2 benzeri karesel azalma
            float strength_at_dist = field->properties.radial_data.strength / (distance_sq);
            
            // Yönü normalize et
            fe_vec3_t normalized_direction = fe_vec3_normalize(direction);

            // Çekme veya İtme
            if (field->properties.radial_data.pulls) {
                // Çekme (Merkeze doğru)
                return fe_vec3_scale(normalized_direction, -strength_at_dist);
            } else {
                // İtme (Merkezden dışa doğru)
                return fe_vec3_scale(normalized_direction, strength_at_dist);
            }
        }
        // TODO: FE_FIELD_TYPE_VORTEX
        default:
            return (fe_vec3_t){0.0f, 0.0f, 0.0f};
    }
}


/**
 * Uygulama: fe_physics_fields_apply_forces_to_rigid_body
 */
void fe_physics_fields_apply_forces_to_rigid_body(
    fe_array_t* fields, 
    fe_rigid_body_t* rb, 
    float dt) 
{
    if (!rb || rb->is_kinematic || !rb->is_awake || rb->mass <= 0.0f) return;
    
    // Yalnızca kütlesi olan cisimlere kuvvet uygula (F = ma)

    size_t count = fe_array_count(fields);
    for (size_t i = 0; i < count; ++i) {
        fe_physics_field_t** field_ptr = (fe_physics_field_t**)fe_array_get(fields, i);
        if (!field_ptr || !(*field_ptr) || !(*field_ptr)->is_active) continue;

        fe_physics_field_t* field = *field_ptr;

        // 1. Cismin alan hacmi içinde olup olmadığını kontrol et
        if (!fe_field_is_point_in_volume(field, rb->position)) continue;

        // 2. Kuvveti hesapla
        fe_vec3_t base_force = fe_field_calculate_force(field, rb->position);

        // 3. Azalma (Falloff) faktörünü uygula
        float falloff_factor = fe_field_calculate_falloff_factor(field, rb->position);
        fe_vec3_t final_force = fe_vec3_scale(base_force, falloff_factor);

        // 4. Kuvveti Katı Cisme uygula
        fe_rigid_body_apply_force(rb, final_force);
        
        // Not: Alanların tork uygulaması gerekiyorsa (örneğin rüzgarın yelkenliye tork uygulaması),
        // fe_rigid_body_apply_force_at_point kullanılabilir.
    }
}