// include/physics/fe_physics_constraint_component.h

#ifndef FE_PHYSICS_CONSTRAINT_COMPONENT_H
#define FE_PHYSICS_CONSTRAINT_COMPONENT_H

#include <stdint.h>
#include <stdbool.h>
#include "math/fe_vector.h"
#include "physics/fe_rigid_body.h" // fe_rigid_body_t pointer'larını kullanmak için

// ----------------------------------------------------------------------
// 1. KISITLAMA TİPLERİ
// ----------------------------------------------------------------------

/**
 * @brief Iki katı cisim arasindaki kısıtlama (baglantı) tipini tanımlar.
 */
typedef enum fe_constraint_type {
    FE_CONSTRAINT_BALL_AND_SOCKET, // Bilye ve Yuva (Üç serbest dönme ekseni)
    FE_CONSTRAINT_HINGE,           // Menteşe (Tek dönme ekseni, kapi gibi)
    FE_CONSTRAINT_SPRING,          // Yay (Cisimleri sabit bir mesafede tutmaya çalisir)
    FE_CONSTRAINT_FIXED,           // Sabit (Cisimleri birbirine kaynak yapar)
    FE_CONSTRAINT_COUNT
} fe_constraint_type_t;

// ----------------------------------------------------------------------
// 2. KISITLAMA YAPISI
// ----------------------------------------------------------------------

/**
 * @brief İki katı cismi baglayan bir fizik kısıtlamasını temsil eder.
 */
typedef struct fe_physics_constraint_component {
    fe_constraint_type_t type;
    
    // Bağlantı Yapıları
    fe_rigid_body_t* body_a;       // Kısıtlamanın birinci cismi
    fe_rigid_body_t* body_b;       // Kısıtlamanın ikinci cismi (NULL ise, body_a dünyaya sabitlenir)
    
    // Bağlama Noktaları (Cisimlerin yerel uzayinda)
    fe_vec3_t anchor_a_local;      // Body A'nın yerel uzayindaki baglama noktasi
    fe_vec3_t anchor_b_local;      // Body B'nin yerel uzayindaki baglama noktasi

    // Kısıtlama Özellikleri (Tip'e özgü)
    union {
        // Yay kısıtlaması özellikleri
        struct {
            float rest_length;     // Yayin ideal uzunlugu
            float stiffness;       // Yayin sertligi (k)
            float damping;         // Sönümleme katsayisi
        } spring_data;

        // Bilye ve Yuva limitleri (ileride eklenebilir)
        // struct { ... } ball_socket_data;
        
        // Menteşe ekseni (ileride eklenebilir)
        // struct { fe_vec3_t axis_local; } hinge_data;
        
    } properties;

    bool is_active;                // Kısıtlama etkin mi?
    uint32_t id;                   // Benzersiz kimlik (Yönetim için)

} fe_physics_constraint_component_t;


// ----------------------------------------------------------------------
// 3. YÖNETİM VE BAŞLATMA FONKSİYONLARI
// ----------------------------------------------------------------------

/**
 * @brief Yeni bir kısıtlama yapisi olusturur ve baslatir.
 * * @param type Kısıtlama tipi.
 * @param body_a Birinci cisim.
 * @param body_b Ikinci cisim (NULL olabilir).
 * @return Baslatilmis kısıtlama pointer'i.
 */
fe_physics_constraint_component_t* fe_constraint_create(
    fe_constraint_type_t type, 
    fe_rigid_body_t* body_a, 
    fe_rigid_body_t* body_b);

/**
 * @brief Kısıtlama yapisini bellekten serbest birakir.
 */
void fe_constraint_destroy(fe_physics_constraint_component_t* constraint);

/**
 * @brief Bir yay kısıtlamasinin parametrelerini ayarlar.
 */
void fe_constraint_set_spring_data(
    fe_physics_constraint_component_t* constraint, 
    float rest_length, 
    float stiffness, 
    float damping);

// (Diğer kısıtlama tipleri için ayar fonksiyonları buraya gelecektir)

#endif // FE_PHYSICS_CONSTRAINT_COMPONENT_H