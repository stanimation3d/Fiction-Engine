// include/animation/fe_animation.h

#ifndef FE_ANIMATION_H
#define FE_ANIMATION_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "data_structures/fe_array.h" // Kemikleri ve Keyframe'leri tutmak için
#include "math/fe_vector.h"       // Pozisyon/Ölçek
#include "math/fe_quaternion.h"   // Döndürme

// ----------------------------------------------------------------------
// 1. TEMEL YAPILAR (KEMİK VE DÖNÜŞÜM)
// ----------------------------------------------------------------------

/**
 * @brief Tek bir kemiğin yerel dönüsümünü (Local Transform) temsil eder.
 * * Animasyon verisi olarak kullanılır.
 */
typedef struct fe_anim_transform {
    fe_vec3_t position;
    fe_quat_t rotation;
    fe_vec3_t scale;
} fe_anim_transform_t;

/**
 * @brief Bir İskeletteki (Skeleton) tek bir Kemiği temsil eder.
 */
typedef struct fe_bone {
    uint32_t id;                // Kemiğin benzersiz ID'si
    int32_t parent_id;          // Ebeveyn kemiğin ID'si (-1 ise kök kemiktir)
    fe_anim_transform_t offset; // Kemiğin varsayılan pozdan Matris ofseti (Inverse Bind Pose)
} fe_bone_t;

/**
 * @brief Karakterin İskelet Yapısını (Skeleton) temsil eder.
 */
typedef struct fe_skeleton {
    fe_array_t bones;           // fe_bone_t dizisi
    size_t bone_count;
} fe_skeleton_t;


// ----------------------------------------------------------------------
// 2. ANİMASYON KLİBİ (VERİ)
// ----------------------------------------------------------------------

/**
 * @brief Bir animasyon klibindeki tek bir kemiğin zaman içindeki hareketini (Keyframe'lerini) tutar.
 */
typedef struct fe_bone_channel {
    uint32_t bone_id;           // Hedeflenen kemik ID'si
    fe_array_t position_keys;   // fe_vec3_t + float time çiftleri
    fe_array_t rotation_keys;   // fe_quat_t + float time çiftleri
    fe_array_t scale_keys;      // fe_vec3_t + float time çiftleri
} fe_bone_channel_t;

/**
 * @brief Tek bir animasyon klibini (örn: "Run", "Jump") temsil eder.
 */
typedef struct fe_anim_clip {
    float duration;             // Animasyonun toplam süresi (saniye)
    float ticks_per_second;     // Keyframe hızı (FPS)
    fe_array_t channels;        // fe_bone_channel_t dizisi
} fe_anim_clip_t;


// ----------------------------------------------------------------------
// 3. ÇALIŞMA ZAMANI YÖNETİMİ
// ----------------------------------------------------------------------

/**
 * @brief Oyun dünyasında çalan bir animasyonun örneği (instance).
 */
typedef struct fe_anim_instance {
    fe_skeleton_t* skeleton;        // Kullanılan iskelet
    fe_anim_clip_t* active_clip;    // Şu an çalan klip
    float current_time;             // Klipteki mevcut zaman (saniye)
    float blend_weight;             // Karıştırma ağırlığı (1.0 = tam aktif)
    // fe_anim_clip_t* next_clip;    // Karıştırılacak bir sonraki klip (Blending için)
    fe_array_t final_transforms;    // Çıktı: Nihai dünya dönüşüm matrisleri (fe_mat4_t)
} fe_anim_instance_t;


/**
 * @brief Animasyon sistemini baslatir.
 */
bool fe_animation_init(void);

/**
 * @brief Animasyon sistemini kapatir.
 */
void fe_animation_shutdown(void);

/**
 * @brief Animasyon örneğini zamana göre günceller ve final dönüşümlerini hesaplar.
 * @param instance Güncellenecek animasyon örneği.
 * @param dt Geçen zaman (delta time).
 */
void fe_anim_instance_update(fe_anim_instance_t* instance, float dt);

#endif // FE_ANIMATION_H
