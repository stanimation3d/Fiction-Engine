// include/physics/fe_rigid_body.h

#ifndef FE_RIGID_BODY_H
#define FE_RIGID_BODY_H

#include <stdint.h>
#include <stdbool.h>
#include "math/fe_vector.h"       // fe_vec3_t, fe_vec4_t
#include "math/fe_matrix.h"       // fe_mat4_t
#include "physics/fe_physical_materials.h" // fe_physical_material_t

// ----------------------------------------------------------------------
// 1. KATILAR CİSİM YAPISI
// ----------------------------------------------------------------------

/**
 * @brief Fizik simülasyonunda bir katı cismi (rigid body) temsil eder.
 */
typedef struct fe_rigid_body {
    // ------------------------------------
    // A. Kütle ve Çarpışma Özellikleri (Sabit)
    // ------------------------------------
    float mass;                 // Kütle (kg). Mass = 0.0f ise cisim statiktir (hareketsiz).
    float inverse_mass;         // 1 / mass (Çarpışma hesaplamalarında performans için kullanılır)
    fe_mat4_t inertia_tensor;   // Cismin yerel uzaydaki eylemsizlik tensörü (Dönme direnci)
    fe_mat4_t inverse_inertia_tensor; // Ters eylemsizlik tensörü (Performans için)

    const fe_physical_material_t* material; // Malzeme (Sürtünme, Sekme)

    // TODO: fe_collider_t* collider; // Çarpışma geometrisi (Kapsül, Küp, vb.) buraya eklenecektir.

    // ------------------------------------
    // B. Konum ve Yönelim (Durum Vektörleri)
    // ------------------------------------
    fe_vec3_t position;         // Dünya uzayindaki konumu (x, y, z)
    fe_vec4_t orientation;      // Dünya uzayindaki yönelimi (Quaternion)
    
    // ------------------------------------
    // C. Hız (Dinamik)
    // ------------------------------------
    fe_vec3_t linear_velocity;  // Doğrusal hız (m/s)
    fe_vec3_t angular_velocity; // Açısal hız (radyan/s)

    // ------------------------------------
    // D. Etkileşen Kuvvetler (Her karede sıfırlanır)
    // ------------------------------------
    fe_vec3_t total_force;      // Cisme etki eden toplam doğrusal kuvvet (Newton)
    fe_vec3_t total_torque;     // Cisme etki eden toplam tork/dönme kuvveti (Newton * metre)

    // ------------------------------------
    // E. Türetilmiş Değerler
    // ------------------------------------
    fe_mat4_t rotation_matrix;  // Yönelimden türetilen dönüş matrisi (Render için)

    bool is_awake;              // Cisim hareket ediyor/etkilasiyor mu? (Uyku/Dinlenme optimizasyonu)
    bool is_kinematic;          // Cismi hareket ettiren fizik degil, kullanici/animasyon mu? (Kilitli cisimler)
} fe_rigid_body_t;


// ----------------------------------------------------------------------
// 2. YÖNETİM VE HESAPLAMA FONKSİYONLARI
// ----------------------------------------------------------------------

/**
 * @brief Yeni bir katı cisim (rigid body) yapisi olusturur ve baslatir.
 * @return Baslatilmis katı cisim pointer'i.
 */
fe_rigid_body_t* fe_rigid_body_create(void);

/**
 * @brief Katı cisim yapisini bellekten serbest birakir.
 */
void fe_rigid_body_destroy(fe_rigid_body_t* rb);

/**
 * @brief Cismin kütle, eylemsizlik tensörü ve ters matrislerini ayarlar.
 * * Bir cismin sekli ve yogunlugu bilindiginde kütle/tensör hesaplanmalidir.
 * @param rb Cisim.
 * @param mass Cismin kütlesi.
 * @param local_inertia_tensor Cismin yerel eylemsizlik tensörü.
 */
void fe_rigid_body_set_mass_properties(fe_rigid_body_t* rb, float mass, fe_mat4_t local_inertia_tensor);

/**
 * @brief Cisme bir kuvvet uygular. 
 * * @param rb Cisim.
 * @param force Uygulanacak kuvvet (Doğrusal).
 */
void fe_rigid_body_apply_force(fe_rigid_body_t* rb, fe_vec3_t force);

/**
 * @brief Cisme belirli bir dünya noktasindan bir kuvvet uygular (Tork olusur).
 * * @param rb Cisim.
 * @param force Uygulanacak kuvvet.
 * @param world_point Kuvvetin uygulandigi dünya koordinati.
 */
void fe_rigid_body_apply_force_at_point(fe_rigid_body_t* rb, fe_vec3_t force, fe_vec3_t world_point);

/**
 * @brief Cisme tork (dönme kuvveti) uygular.
 */
void fe_rigid_body_apply_torque(fe_rigid_body_t* rb, fe_vec3_t torque);

/**
 * @brief Cismin toplam kuvvet/tork birikimini sifirlar.
 * * Genellikle her simülasyon adımının başında çağrılır.
 */
void fe_rigid_body_clear_forces(fe_rigid_body_t* rb);

/**
 * @brief Cismin doğrusal ve açısal hızını, toplam kuvvet/tork ve delta zamana göre günceller (Integrasyon).
 * * Bu, fizik simülasyonunun kalbidir.
 * @param rb Cisim.
 * @param dt Gecen simülasyon süresi (saniye).
 */
void fe_rigid_body_integrate(fe_rigid_body_t* rb, float dt);

#endif // FE_RIGID_BODY_H