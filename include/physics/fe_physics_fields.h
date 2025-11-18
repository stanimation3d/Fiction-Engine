// include/physics/fe_physics_fields.h

#ifndef FE_PHYSICS_FIELDS_H
#define FE_PHYSICS_FIELDS_H

#include <stdint.h>
#include <stdbool.h>
#include "math/fe_vector.h"
#include "physics/fe_rigid_body.h" // fe_rigid_body_t pointer'larını kullanmak için
#include "data_structures/fe_array.h" // Alan koleksiyonları için

// ----------------------------------------------------------------------
// 1. ALAN TİPLERİ
// ----------------------------------------------------------------------

/**
 * @brief Fizik alanının davranış tipini tanımlar.
 */
typedef enum fe_field_type {
    FE_FIELD_TYPE_VECTOR,      // Belirli bir yönde (örneğin rüzgar) kuvvet uygular.
    FE_FIELD_TYPE_RADIAL,      // Bir merkezden yayılan veya merkeze çeken kuvvet (örneğin patlama, kara delik).
    FE_FIELD_TYPE_VORTEX,      // Girdap benzeri dönme kuvveti.
    FE_FIELD_TYPE_COUNT
} fe_field_type_t;

/**
 * @brief Fizik alanının etkili olduğu geometrik hacim şekli.
 */
typedef enum fe_field_shape {
    FE_FIELD_SHAPE_AABB,       // Axis-Aligned Bounding Box (Küp/Dikdörtgen prizma).
    FE_FIELD_SHAPE_SPHERE,     // Küresel hacim.
    FE_FIELD_SHAPE_COUNT
} fe_field_shape_t;

// ----------------------------------------------------------------------
// 2. ALAN YAPISI
// ----------------------------------------------------------------------

/**
 * @brief Bir fiziksel kuvvet alanını temsil eder.
 */
typedef struct fe_physics_field {
    uint32_t id;
    fe_field_type_t type;
    fe_field_shape_t shape;

    // Konum ve Hacim Bilgisi
    fe_vec3_t position;         // Alanın merkez konumu (Dünya uzayı)
    fe_vec3_t size;             // Hacmin boyutları (AABB için uzunluk, Küre için yariçap vb. tutulabilir)
    float falloff;              // Kuvvetin kenarlara doğru azalma hızı (0.0: keskin, 1.0: yumusak)

    // Kuvvet Özellikleri
    union {
        // Vektör Alanı (Rüzgar)
        struct {
            fe_vec3_t direction; // Kuvvetin yönü (normalize edilmiş)
            float strength;      // Kuvvetin büyüklüğü (Newton)
        } vector_data;

        // Radyal Alan (Patlama/Çekim)
        struct {
            float strength;      // Merkezden yayılan/çeken kuvvetin büyüklüğü
            bool pulls;          // True ise çeker (Merkeze), False ise iter (Dışa)
        } radial_data;
    } properties;

    bool is_active;
} fe_physics_field_t;


// ----------------------------------------------------------------------
// 3. YÖNETİM VE UYGULAMA FONKSİYONLARI
// ----------------------------------------------------------------------

/**
 * @brief Yeni bir fizik alani yapisi olusturur ve baslatir.
 * * @param type Alan tipi.
 * @param shape Alanin hacim sekli.
 * @return Baslatilmis alan pointer'i.
 */
fe_physics_field_t* fe_field_create(fe_field_type_t type, fe_field_shape_t shape);

/**
 * @brief Fizik alanini bellekten serbest birakir.
 */
void fe_field_destroy(fe_physics_field_t* field);

/**
 * @brief Tüm etkin fizik alanlarinin, verilen katı cisimlere kuvvet uygulamasini saglar.
 * * @param fields Tum alanlarin koleksiyonu (fe_array_t*).
 * @param rb Kuvvetin uygulanacagi katı cisim.
 * @param dt Zaman adimi (Gerekirse entegrasyon için).
 */
void fe_physics_fields_apply_forces_to_rigid_body(
    fe_array_t* fields, 
    fe_rigid_body_t* rb, 
    float dt);

#endif // FE_PHYSICS_FIELDS_H