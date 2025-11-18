// src/physics/fe_physics_thruster_component.c

#include "physics/fe_physics_thruster_component.h"
#include "utils/fe_logger.h"
#include <stdlib.h> // malloc, free
#include <math.h>   // fmaxf, fminf

// Dahili yardımcı fonksiyonlar için bildirim (fe_vector, fe_matrix kütüphanelerinde olmalıdır)
fe_vec3_t fe_mat4_transform_vec3(fe_mat4_t mat, fe_vec3_t vec);

// Benzersiz kimlik sayacı
static uint32_t g_next_thruster_id = 1;

// ----------------------------------------------------------------------
// 1. OLUŞTURMA VE YÖNETİM
// ----------------------------------------------------------------------

/**
 * Uygulama: fe_thruster_create
 */
fe_physics_thruster_component_t* fe_thruster_create(
    fe_rigid_body_t* target_body, 
    float max_thrust, 
    fe_vec3_t local_pos,
    fe_vec3_t local_dir) 
{
    if (!target_body) {
        FE_LOG_ERROR("İtici olusturmak icin hedef Rigid Body gereklidir.");
        return NULL;
    }
    
    fe_physics_thruster_component_t* thruster = 
        (fe_physics_thruster_component_t*)calloc(1, sizeof(fe_physics_thruster_component_t));
        
    if (!thruster) {
        FE_LOG_FATAL("İtici bileseni icin bellek ayrilamadi.");
        return NULL;
    }
    
    thruster->id = g_next_thruster_id++;
    thruster->target_body = target_body;
    thruster->max_thrust = fmaxf(0.0f, max_thrust);
    thruster->local_position = local_pos;
    
    // Yön vektörünün normalize edildiğinden emin ol
    thruster->local_direction = fe_vec3_normalize(local_dir); 
    
    thruster->throttle_factor = 0.0f; // Başlangıçta kapalı
    thruster->is_active = true;

    FE_LOG_TRACE("İtici Bileseni %u olusturuldu.", thruster->id);
    return thruster;
}

/**
 * Uygulama: fe_thruster_destroy
 */
void fe_thruster_destroy(fe_physics_thruster_component_t* thruster) {
    if (thruster) {
        free(thruster);
        FE_LOG_TRACE("İtici Bileseni yok edildi.");
    }
}

/**
 * Uygulama: fe_thruster_set_throttle
 */
void fe_thruster_set_throttle(fe_physics_thruster_component_t* thruster, float factor) {
    // Faktörü [0.0, 1.0] arasına kısıtla
    thruster->throttle_factor = fminf(1.0f, fmaxf(0.0f, factor));
    // Gaz verilince cismi uyandır
    if (thruster->throttle_factor > 0.0f && thruster->target_body) {
        thruster->target_body->is_awake = true;
    }
}


// ----------------------------------------------------------------------
// 2. KUVVET UYGULAMASI
// ----------------------------------------------------------------------

/**
 * Uygulama: fe_thruster_apply_force
 */
void fe_thruster_apply_force(fe_physics_thruster_component_t* thruster) {
    fe_rigid_body_t* rb = thruster->target_body;

    if (!thruster->is_active || rb->is_kinematic || !rb->is_awake || thruster->throttle_factor <= 0.0f) {
        return;
    }

    // 1. Dünya Uzayına Dönüştür
    
    // Cismin yerel dönüş matrisi (orientation) zaten rb->rotation_matrix'te tutuluyor.
    // Bu matris, cisim yerel uzayından dünya uzayına dönüştürme matrisidir.

    // A. Kuvvet Yönü: Yerel yönü Dünya uzayına dönüştür.
    fe_vec3_t world_direction = fe_mat4_transform_vec3(rb->rotation_matrix, thruster->local_direction);

    // B. Uygulama Noktası: Yerel pozisyonu Dünya uzayına dönüştür.
    fe_vec3_t world_local_point = fe_mat4_transform_vec3(rb->rotation_matrix, thruster->local_position);
    fe_vec3_t world_point = fe_vec3_add(rb->position, world_local_point);

    // 2. Nihai Kuvveti Hesapla
    float magnitude = thruster->max_thrust * thruster->throttle_factor;
    fe_vec3_t final_force = fe_vec3_scale(world_direction, magnitude);

    // 3. Kuvveti Uygula (Doğrusal hareket ve Tork üretimi)
    // Bu fonksiyon, kuvveti cismin pozisyonu yerine world_point'e uygulayarak torku otomatik olarak hesaplayacaktır.
    fe_rigid_body_apply_force_at_point(rb, final_force, world_point);
}

// ----------------------------------------------------------------------
// 3. YARDIMCI FONKSİYON YER TUTUCU
// ----------------------------------------------------------------------

// fe_mat4_transform_vec3'ün fe_matrix.c'de var olduğu varsayılıyor.
// Sadece rotasyon matrisini kullanarak vektör dönüşümü (pozisyon değil, yön için):
// matrisin 3x3 kısmını kullanarak dönüşüm gerçekleştirilir.
fe_vec3_t fe_mat4_transform_vec3(fe_mat4_t mat, fe_vec3_t vec) {
    // Bu fonksiyonun, fe_matrix.h/c'de implemente edilmesi beklenir. 
    // Basit bir yaklaşımla (sadece rotasyonu kullanarak):
    float x = mat.m[0][0] * vec.x + mat.m[0][1] * vec.y + mat.m[0][2] * vec.z;
    float y = mat.m[1][0] * vec.x + mat.m[1][1] * vec.y + mat.m[1][2] * vec.z;
    float z = mat.m[2][0] * vec.x + mat.m[2][1] * vec.y + mat.m[2][2] * vec.z;

    // Eğer bu bir pozisyon noktasıysa, matrisin 4. sütunu (çeviri) da eklenmeli.
    // Thruster'da hem local_position hem de local_direction kullanıldığı için bu yaklaşımlar farklı olabilir.
    // Şimdilik sadece 3x3 rotasyonu varsayalım:
    return (fe_vec3_t){x, y, z};
}