// src/physics/fe_ragdoll_physics.c

#include "physics/fe_ragdoll_physics.h"
#include "utils/fe_logger.h"
#include "data_structures/fe_array.h"
#include <stdlib.h> // malloc, free
#include <string.h> // memset

// Benzersiz kimlik sayacı
static uint32_t g_next_ragdoll_id = 1;

// ----------------------------------------------------------------------
// 1. YÖNETİM UYGULAMALARI
// ----------------------------------------------------------------------

/**
 * Uygulama: fe_ragdoll_create
 */
fe_ragdoll_t* fe_ragdoll_create(void* character_handle) {
    fe_ragdoll_t* ragdoll = (fe_ragdoll_t*)calloc(1, sizeof(fe_ragdoll_t));
    if (!ragdoll) {
        FE_LOG_FATAL("Ragdoll icin bellek ayrilamadi.");
        return NULL;
    }
    
    ragdoll->id = g_next_ragdoll_id++;
    ragdoll->character_handle = character_handle;
    ragdoll->is_active = false;
    
    // Koleksiyonları başlat
    ragdoll->rigid_bodies = fe_array_create(sizeof(fe_rigid_body_t*));
    ragdoll->constraints = fe_array_create(sizeof(fe_physics_constraint_component_t*));
    
    FE_LOG_INFO("Ragdoll %u olusturuldu.", ragdoll->id);
    return ragdoll;
}

/**
 * Uygulama: fe_ragdoll_destroy
 */
void fe_ragdoll_destroy(fe_ragdoll_t* ragdoll) {
    if (!ragdoll) return;
    
    // Önce kısıtlamaları yok et
    for (size_t i = 0; i < fe_array_count(ragdoll->constraints); ++i) {
        fe_physics_constraint_component_t** c_ptr = (fe_physics_constraint_component_t**)fe_array_get(ragdoll->constraints, i);
        fe_constraint_destroy(*c_ptr);
    }
    fe_array_destroy(ragdoll->constraints);
    
    // Sonra rigid body'leri yok et
    for (size_t i = 0; i < fe_array_count(ragdoll->rigid_bodies); ++i) {
        fe_rigid_body_t** rb_ptr = (fe_rigid_body_t**)fe_array_get(ragdoll->rigid_bodies, i);
        // NOT: Ragdoll'daki rigid body'ler fe_physics_manager'dan da kaldirilmalidir.
        fe_rigid_body_destroy(*rb_ptr);
    }
    fe_array_destroy(ragdoll->rigid_bodies);

    free(ragdoll);
    FE_LOG_INFO("Ragdoll %u yok edildi.", ragdoll->id);
}

// ----------------------------------------------------------------------
// 2. KURULUM VE BAĞLANTI (SETUP)
// ----------------------------------------------------------------------

/**
 * Uygulama: fe_ragdoll_setup_from_skeleton
 * * Gerçek iskelet verileriyle doldurma işlemi, motorun iskelet sistemine bağlıdır.
 */
void fe_ragdoll_setup_from_skeleton(fe_ragdoll_t* ragdoll /* Skeletal veri parametreleri */) {
    // Bu, tipik bir kurulum örneğidir:
    
    // 1. Kemikleri Rigid Body Olarak Yarat
    
    // fe_rigid_body_t* pelvis_rb = fe_rigid_body_create();
    // // ... kütle ve tensör ayarları
    // fe_array_push(ragdoll->rigid_bodies, &pelvis_rb);
    
    // fe_rigid_body_t* spine_rb = fe_rigid_body_create();
    // fe_array_push(ragdoll->rigid_bodies, &spine_rb);
    
    // 2. Kısıtlamaları (Eklemleri) Yarat
    
    // fe_physics_constraint_component_t* pelvis_spine_joint = fe_constraint_create(
    //     FE_CONSTRAINT_BALL_AND_SOCKET, pelvis_rb, spine_rb);
    
    // // fe_constraint_set_limits(pelvis_spine_joint, ...); // Açı limitlerini ayarla
    
    // fe_array_push(ragdoll->constraints, &pelvis_spine_joint);
    
    // 3. Rigid Body'leri fe_physics_manager'a ekle (Manager'ın bu fonksiyonu olmalı)
    // fe_physics_manager_add_rigid_body(pelvis_rb);
    // fe_physics_manager_add_rigid_body(spine_rb);
    
    FE_LOG_DEBUG("Ragdoll iskelet verileri ile kuruldu. Kemik sayisi: %zu, Eklemler: %zu", 
                 fe_array_count(ragdoll->rigid_bodies), fe_array_count(ragdoll->constraints));
}

// ----------------------------------------------------------------------
// 3. DURUM GEÇİŞLERİ (ACTIVATION/DEACTIVATION)
// ----------------------------------------------------------------------

/**
 * Uygulama: fe_ragdoll_activate
 */
void fe_ragdoll_activate(fe_ragdoll_t* ragdoll) {
    if (ragdoll->is_active) return;
    
    // 1. Tüm Rigid Body'leri Kinematik moddan Dinamik moda al
    for (size_t i = 0; i < fe_array_count(ragdoll->rigid_bodies); ++i) {
        fe_rigid_body_t** rb_ptr = (fe_rigid_body_t**)fe_array_get(ragdoll->rigid_bodies, i);
        fe_rigid_body_t* rb = *rb_ptr;

        // Statik kütlesi olmayan cisimler bileşenleri tamamen statik bırakılabilir.
        if (rb->mass > 0.0f) {
            rb->is_kinematic = false; // Fiziğin kontrolüne geç
            rb->is_awake = true;      // Simülasyonu başlat
            
            // TODO: Eğer varsa, karakterin mevcut animasyon pozisyonundan RB'lerin konumunu ve yönelimini ayarla.
            // rb->position = fe_get_bone_world_position(ragdoll->character_handle, i);
        }
    }
    
    // 2. Tüm kısıtlamaları etkinleştir (Açı ve mesafe limitlerini uygulamaya başla)
    for (size_t i = 0; i < fe_array_count(ragdoll->constraints); ++i) {
        fe_physics_constraint_component_t** c_ptr = (fe_physics_constraint_component_t**)fe_array_get(ragdoll->constraints, i);
        (*c_ptr)->is_active = true;
    }
    
    ragdoll->is_active = true;
    FE_LOG_WARNING("Ragdoll %u etkinlestirildi. Fizik kontrolü devraldi.", ragdoll->id);
    
    
}

/**
 * Uygulama: fe_ragdoll_deactivate
 */
void fe_ragdoll_deactivate(fe_ragdoll_t* ragdoll) {
    if (!ragdoll->is_active) return;

    // 1. Tüm Rigid Body'leri Kinematik moda al
    for (size_t i = 0; i < fe_array_count(ragdoll->rigid_bodies); ++i) {
        fe_rigid_body_t** rb_ptr = (fe_rigid_body_t**)fe_array_get(ragdoll->rigid_bodies, i);
        fe_rigid_body_t* rb = *rb_ptr;

        if (rb->mass > 0.0f) {
            rb->is_kinematic = true;  // Animasyon sisteminin kontrolüne geç
            rb->is_awake = false;     // Uykuya al
            // Hızları sıfırla
            rb->linear_velocity = (fe_vec3_t){0};
            rb->angular_velocity = (fe_vec3_t){0};
        }
    }
    
    // 2. Tüm kısıtlamaları devre dışı bırak
    for (size_t i = 0; i < fe_array_count(ragdoll->constraints); ++i) {
        fe_physics_constraint_component_t** c_ptr = (fe_physics_constraint_component_t**)fe_array_get(ragdoll->constraints, i);
        (*c_ptr)->is_active = false;
    }

    ragdoll->is_active = false;
    FE_LOG_WARNING("Ragdoll %u devre disi birakildi. Animasyon kontrolü devraldi.", ragdoll->id);
}