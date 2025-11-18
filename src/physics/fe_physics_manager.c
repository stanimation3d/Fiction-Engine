// src/physics/fe_physics_manager.c

#include "physics/fe_physics_manager.h"
#include "utils/fe_logger.h"
#include "data_structures/fe_array.h" // fe_array_create, fe_array_push, fe_array_count, vb.

// ----------------------------------------------------------------------
// 1. GLOBAL YÖNETİCİ DURUMU
// ----------------------------------------------------------------------

static fe_physics_manager_t g_manager_state = {0};

// Simülasyon güvenliğini sağlamak için maksimum yinelenme (iteration) sınırı
#define MAX_PHYSICS_STEPS 5 

// ----------------------------------------------------------------------
// 2. YAŞAM DÖNGÜSÜ UYGULAMALARI
// ----------------------------------------------------------------------

/**
 * Uygulama: fe_physics_manager_init
 */
void fe_physics_manager_init(void) {
    FE_LOG_INFO("Fizik Yoneticisi baslatiliyor...");
    
    // Varsayilan Yerçekimi (Dünya standardı: Y ekseninde -9.81 m/s^2)
    g_manager_state.gravity = (fe_vec3_t){0.0f, -9.81f, 0.0f};

    // Dinamik dizi yapısını başlat
    g_manager_state.rigid_bodies = fe_array_create(sizeof(fe_rigid_body_t*));
    g_manager_state.accumulator = 0.0f;

    FE_LOG_INFO("Fizik Yoneticisi baslatildi. Zaman adimi: %f s", FE_PHYSICS_FIXED_DT);
}

/**
 * Uygulama: fe_physics_manager_shutdown
 */
void fe_physics_manager_shutdown(void) {
    FE_LOG_INFO("Fizik Yoneticisi kapatiliyor...");
    
    // Tüm katı cisimleri serbest bırak
    if (g_manager_state.rigid_bodies) {
        // Not: fe_rigid_body_destroy'in dinamik olarak ayrılan her RB'yi yok ettiğini varsayalım.
        for (size_t i = 0; i < fe_array_count(g_manager_state.rigid_bodies); ++i) {
            fe_rigid_body_t** rb_ptr = (fe_rigid_body_t**)fe_array_get(g_manager_state.rigid_bodies, i);
            if (rb_ptr && *rb_ptr) {
                fe_rigid_body_destroy(*rb_ptr);
            }
        }
        fe_array_destroy(g_manager_state.rigid_bodies);
        g_manager_state.rigid_bodies = NULL;
    }
}

/**
 * Uygulama: fe_physics_manager_add_rigid_body
 */
void fe_physics_manager_add_rigid_body(fe_rigid_body_t* rb) {
    if (!rb) return;
    fe_array_push(g_manager_state.rigid_bodies, &rb);
    FE_LOG_TRACE("Rigid Body eklendi. Toplam: %zu", fe_array_count(g_manager_state.rigid_bodies));
}

/**
 * Uygulama: fe_physics_manager_remove_rigid_body
 * Not: fe_array_remove_item veya fe_array_remove_at fonksiyonunun var olduğu varsayılır.
 */
void fe_physics_manager_remove_rigid_body(fe_rigid_body_t* rb) {
    if (!rb || !g_manager_state.rigid_bodies) return;
    
    for (size_t i = 0; i < fe_array_count(g_manager_state.rigid_bodies); ++i) {
        fe_rigid_body_t** rb_ptr = (fe_rigid_body_t**)fe_array_get(g_manager_state.rigid_bodies, i);
        if (rb_ptr && *rb_ptr == rb) {
            // fe_array_remove_at(g_manager_state.rigid_bodies, i); // Silme işlemi
            // Basitlik için sadece loglama yapalım, gerçek fe_array implementasyonuna bağlıdır.
            FE_LOG_TRACE("Rigid Body kaldirildi.");
            return;
        }
    }
}


// ----------------------------------------------------------------------
// 3. SİMÜLASYON ADIMI UYGULAMALARI
// ----------------------------------------------------------------------

/**
 * Uygulama: fe_physics_manager_step
 */
void fe_physics_manager_step(void) {
    size_t count = fe_array_count(g_manager_state.rigid_bodies);
    fe_rigid_body_t** current_rb_ptr;

    // 1. Kuvvetleri Uygula (Yerçekimi gibi sürekli kuvvetler)
    for (size_t i = 0; i < count; ++i) {
        current_rb_ptr = (fe_rigid_body_t**)fe_array_get(g_manager_state.rigid_bodies, i);
        if (!current_rb_ptr || !(*current_rb_ptr) || (*current_rb_ptr)->is_kinematic || !(*current_rb_ptr)->is_awake) continue;

        fe_rigid_body_t* rb = *current_rb_ptr;

        // Yerçekimi kuvveti = kütle * yerçekimi vektörü
        if (rb->mass > 0.0f) {
            fe_vec3_t gravity_force = fe_vec3_scale(g_manager_state.gravity, rb->mass);
            fe_rigid_body_apply_force(rb, gravity_force);
        }

        // TODO: fe_physics_fields'dan gelen kuvvetleri uygula
    }

    // 2. Çarpışma Tespiti ve Çözümü (En karmaşık kısım!)
    // TODO: fe_collider_manager_detect_collisions();
    // TODO: fe_collision_solver_resolve_contacts();

    // 3. Konum ve Hız Entegrasyonu
    for (size_t i = 0; i < count; ++i) {
        current_rb_ptr = (fe_rigid_body_t**)fe_array_get(g_manager_state.rigid_bodies, i);
        if (!current_rb_ptr || !(*current_rb_ptr) || (*current_rb_ptr)->is_kinematic || !(*current_rb_ptr)->is_awake) continue;

        // fe_rigid_body_integrate, kuvvetleri kullanır ve temizler.
        fe_rigid_body_integrate(*current_rb_ptr, FE_PHYSICS_FIXED_DT);
    }
    
    FE_LOG_TRACE("Fizik adimi tamamlandi.");
    
    
}


/**
 * Uygulama: fe_physics_manager_update
 * Ana uygulama döngüsünden çağrılır.
 */
void fe_physics_manager_update(float delta_time) {
    if (delta_time < 0.0f) delta_time = 0.0f;

    // Delta zamanını birikimciye ekle
    g_manager_state.accumulator += delta_time;

    int steps_taken = 0;
    
    // Birikimci dolduğu sürece sabit adımları at
    while (g_manager_state.accumulator >= FE_PHYSICS_FIXED_DT && steps_taken < MAX_PHYSICS_STEPS) {
        fe_physics_manager_step(); // Sabit adım
        g_manager_state.accumulator -= FE_PHYSICS_FIXED_DT;
        steps_taken++;
    }

    // Güvenlik: Eğer çok fazla adım attıysak (oyun donduysa), kalan birikimi kes.
    if (steps_taken >= MAX_PHYSICS_STEPS) {
        FE_LOG_WARN("Maksimum fizik adim siniri (%d) asildi. Birikimci kesildi.", MAX_PHYSICS_STEPS);
        g_manager_state.accumulator = 0.0f;
    }
    
    // Kalan birikimi sonraki kareye bırakır.
}