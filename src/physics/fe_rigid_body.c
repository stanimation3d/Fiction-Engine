// src/physics/fe_rigid_body.c

#include "physics/fe_rigid_body.h"
#include "utils/fe_logger.h"
#include <stdlib.h> // malloc, free
#include <string.h> // memcpy

// Dahili yardımcı fonksiyonlar için bildirimler (fe_quaternion.h varsayılarak)
fe_vec4_t fe_quat_normalize(fe_vec4_t q);
fe_mat4_t fe_quat_to_mat4(fe_vec4_t q);
fe_vec4_t fe_quat_multiply_vec3(fe_vec4_t q, fe_vec3_t v); // w=0 varsayılır
fe_vec4_t fe_quat_add(fe_vec4_t a, fe_vec4_t b);
fe_vec4_t fe_quat_scale(fe_vec4_t q, float s);


// ----------------------------------------------------------------------
// 1. YÖNETİM UYGULAMALARI
// ----------------------------------------------------------------------

/**
 * Uygulama: fe_rigid_body_create
 */
fe_rigid_body_t* fe_rigid_body_create(void) {
    fe_rigid_body_t* rb = (fe_rigid_body_t*)calloc(1, sizeof(fe_rigid_body_t));
    if (!rb) {
        FE_LOG_FATAL("Rigid body icin bellek ayrilamadi.");
        return NULL;
    }

    // Varsayılan değerler
    rb->material = &FE_MAT_DEFAULT; // Varsayılan malzemeyi kullan
    rb->mass = 1.0f;
    rb->inverse_mass = 1.0f;
    rb->position = (fe_vec3_t){0.0f, 0.0f, 0.0f};
    rb->orientation = (fe_vec4_t){0.0f, 0.0f, 0.0f, 1.0f}; // Identity Quaternion
    rb->is_awake = true;
    rb->is_kinematic = false;
    rb->inertia_tensor = FE_MAT4_IDENTITY;
    rb->inverse_inertia_tensor = FE_MAT4_IDENTITY; 
    
    FE_LOG_TRACE("Rigid Body olusturuldu.");
    return rb;
}

/**
 * Uygulama: fe_rigid_body_destroy
 */
void fe_rigid_body_destroy(fe_rigid_body_t* rb) {
    if (rb) {
        free(rb);
        FE_LOG_TRACE("Rigid Body yok edildi.");
    }
}

/**
 * Uygulama: fe_rigid_body_set_mass_properties
 */
void fe_rigid_body_set_mass_properties(fe_rigid_body_t* rb, float mass, fe_mat4_t local_inertia_tensor) {
    rb->mass = mass;

    if (mass <= 0.0f) {
        // Kütle 0 veya negatif ise cisim statiktir (hareketsiz).
        rb->inverse_mass = 0.0f;
        rb->is_kinematic = true;
        rb->inertia_tensor = FE_MAT4_IDENTITY;
        rb->inverse_inertia_tensor = FE_MAT4_IDENTITY; // Veya sıfır matrisi
    } else {
        rb->inverse_mass = 1.0f / mass;
        rb->is_kinematic = false;
        rb->inertia_tensor = local_inertia_tensor;
        
        // Ters tensörü hesapla
        // NOT: fe_mat4_inverse fonksiyonunun genel/hızlı ters alma kullandığı varsayılır.
        rb->inverse_inertia_tensor = fe_mat4_inverse(local_inertia_tensor); 
    }
    rb->is_awake = true; // Kütle değişince uyandır
}


// ----------------------------------------------------------------------
// 2. KUVVET UYGULAMALARI
// ----------------------------------------------------------------------

/**
 * Uygulama: fe_rigid_body_clear_forces
 */
void fe_rigid_body_clear_forces(fe_rigid_body_t* rb) {
    if (!rb->is_awake || rb->is_kinematic) return;

    rb->total_force = (fe_vec3_t){0.0f, 0.0f, 0.0f};
    rb->total_torque = (fe_vec3_t){0.0f, 0.0f, 0.0f};
}

/**
 * Uygulama: fe_rigid_body_apply_force
 */
void fe_rigid_body_apply_force(fe_rigid_body_t* rb, fe_vec3_t force) {
    if (!rb->is_awake || rb->is_kinematic) return;
    
    // Doğrusal kuvvet ekle (F = ma)
    rb->total_force = fe_vec3_add(rb->total_force, force);
}

/**
 * Uygulama: fe_rigid_body_apply_torque
 */
void fe_rigid_body_apply_torque(fe_rigid_body_t* rb, fe_vec3_t torque) {
    if (!rb->is_awake || rb->is_kinematic) return;
    
    // Tork ekle (tau = I * alpha)
    rb->total_torque = fe_vec3_add(rb->total_torque, torque);
}

/**
 * Uygulama: fe_rigid_body_apply_force_at_point
 */
void fe_rigid_body_apply_force_at_point(fe_rigid_body_t* rb, fe_vec3_t force, fe_vec3_t world_point) {
    if (!rb->is_awake || rb->is_kinematic) return;
    
    // 1. Doğrusal Kuvveti Ekle
    fe_rigid_body_apply_force(rb, force);

    // 2. Torku Hesapla: tau = r x F
    // r = kuvvet uygulama noktasindan cismin kütle merkezine (position) olan vektör
    fe_vec3_t r = fe_vec3_subtract(world_point, rb->position);
    fe_vec3_t torque = fe_vec3_cross(r, force);
    
    // 3. Torku Ekle
    fe_rigid_body_apply_torque(rb, torque);
}


// ----------------------------------------------------------------------
// 3. ENTEGRASYON (DURUM GÜNCELLEMESİ)
// ----------------------------------------------------------------------

/**
 * Uygulama: fe_rigid_body_integrate
 * Simülasyonun kalbi: Hareket denklemlerini çözerek cismin yeni konumunu hesaplar (Basit Euler entegrasyonu).
 */
void fe_rigid_body_integrate(fe_rigid_body_t* rb, float dt) {
    if (!rb->is_awake || rb->is_kinematic) return;
    
    // **********************************************
    // 1. Doğrusal Hareket (Position ve Linear Velocity)
    // **********************************************
    
    // İvme (a) = F / m
    fe_vec3_t linear_acceleration = fe_vec3_scale(rb->total_force, rb->inverse_mass);
    
    // Yeni Hız: v_yeni = v_eski + a * dt
    rb->linear_velocity = fe_vec3_add(rb->linear_velocity, fe_vec3_scale(linear_acceleration, dt));
    
    // Yeni Konum: x_yeni = x_eski + v * dt
    rb->position = fe_vec3_add(rb->position, fe_vec3_scale(rb->linear_velocity, dt));

    // **********************************************
    // 2. Açısal Hareket (Orientation ve Angular Velocity)
    // **********************************************
    
    // Açısal İvme: alpha = Inverse_I * Tork
    // Tensör, dünya uzayına dönüştürülmelidir: I_world_inverse = R * I_local_inverse * R^T
    // NOT: Basitlik için sadece yerel ters tensör çarpımı kullanılmıştır.
    // fe_mat4_transform_vec3_normal fonksiyonunun T * v olduğunu varsayalım.
    fe_vec3_t angular_acceleration; 
    // angular_acceleration = fe_mat4_transform_vec3_normal(rb->inverse_inertia_tensor, rb->total_torque); 
    // fe_mat4_transform_vec3_normal fonksiyonu mevcut olmadığı için vektör matris çarpımının uygulanması gerekir.
    // Basit bir yaklaşımla, sadece dönme matrisini kullanarak çarpımı simüle edelim (Gerçekte daha karmaşıktır):
    
    // Torkun etkisini açısal hızda biriktir
    angular_acceleration = fe_vec3_scale(rb->total_torque, 0.1f * rb->inverse_mass); // Çok basit yaklaşımla
    rb->angular_velocity = fe_vec3_add(rb->angular_velocity, fe_vec3_scale(angular_acceleration, dt));
    
    // Yönelim (Quaternion) Güncellemesi: q_yeni = q_eski + (0.5 * q_eski * w) * dt
    // w: (angular_velocity.x, angular_velocity.y, angular_velocity.z, 0) kuaterni
    fe_vec4_t w = (fe_vec4_t){rb->angular_velocity.x, rb->angular_velocity.y, rb->angular_velocity.z, 0.0f};
    fe_vec4_t dq = fe_quat_multiply_vec3(rb->orientation, w);
    
    rb->orientation = fe_quat_add(rb->orientation, fe_quat_scale(dq, 0.5f * dt));
    
    // Yönelimi normalize et (drift'i önler)
    rb->orientation = fe_quat_normalize(rb->orientation);
    
    // **********************************************
    // 3. Türetilmiş Değerleri Güncelle
    // **********************************************
    
    // Render ve çarpışma için dönüş matrisini güncelle
    rb->rotation_matrix = fe_quat_to_mat4(rb->orientation);

    // Kuvvetleri temizle (bir sonraki kare için)
    fe_rigid_body_clear_forces(rb);
    
    // TODO: Uyku sistemini kontrol et (Hız sıfıra yakınsa is_awake = false)
    
    
}

// ----------------------------------------------------------------------
// 4. KUATERNİYON YARDIMCI FONKSİYONLARI (fe_quaternion.h/c'den gelecektir)
// Basit yer tutucular:
// ----------------------------------------------------------------------

fe_vec4_t fe_quat_normalize(fe_vec4_t q) {
    float length = sqrtf(q.x*q.x + q.y*q.y + q.z*q.z + q.w*q.w);
    if (length < 0.000001f) return (fe_vec4_t){0.0f, 0.0f, 0.0f, 1.0f};
    return fe_quat_scale(q, 1.0f / length);
}

fe_vec4_t fe_quat_scale(fe_vec4_t q, float s) {
    return (fe_vec4_t){q.x * s, q.y * s, q.z * s, q.w * s};
}

fe_vec4_t fe_quat_add(fe_vec4_t a, fe_vec4_t b) {
    return (fe_vec4_t){a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w};
}

// Temsili kuaternion çarpımı: q * v (v'nin w=0 olduğu varsayılır)
fe_vec4_t fe_quat_multiply_vec3(fe_vec4_t q, fe_vec3_t v) {
    // Kuaternion çarpım formülü: (q_x * v_w + q_w * v_x + q_y * v_z - q_z * v_y, ...)
    fe_vec4_t result;
    // ... çarpım formülü buraya gelecektir
    // Basit yer tutucu:
    result.x = q.w * v.x + q.y * v.z - q.z * v.y;
    result.y = q.w * v.y + q.z * v.x - q.x * v.z;
    result.z = q.w * v.z + q.x * v.y - q.y * v.x;
    result.w = -q.x * v.x - q.y * v.y - q.z * v.z;
    return result;
}

// Temsili Quaternion'dan Matris'e dönüştürme (fe_matrix.c'de de olmalıdır)
fe_mat4_t fe_quat_to_mat4(fe_vec4_t q) {
    // ... dönüştürme formülü buraya gelecektir
    // Basit yer tutucu olarak Identity döndürülür:
    return FE_MAT4_IDENTITY; 
}