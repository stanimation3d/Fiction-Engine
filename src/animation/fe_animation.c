// src/animation/fe_animation.c

#include "animation/fe_animation.h"
#include "utils/fe_logger.h"
#include "math/fe_matrix.h" // fe_mat4_t dönüşümü için
#include <stdlib.h> // malloc, free
#include <string.h> // memset
#include <math.h>

// ----------------------------------------------------------------------
// STATİK YARDIMCI FONKSİYONLAR
// ----------------------------------------------------------------------

/**
 * @brief İki dönüşüm (Transform) arasında enterpolasyon yapar (Blending).
 */
static fe_anim_transform_t fe_interpolate_transforms(fe_anim_transform_t t1, fe_anim_transform_t t2, float factor) {
    fe_anim_transform_t result;
    
    // Konum (Position) - Doğrusal Enterpolasyon (Lerp)
    result.position = fe_vec3_lerp(t1.position, t2.position, factor);
    
    // Döndürme (Rotation) - Küresel Doğrusal Enterpolasyon (Slerp)
    // NOT: fe_quat_slerp, quaternion.c'de tanımlanmıştır.
    result.rotation = fe_quat_slerp(t1.rotation, t2.rotation, factor);
    
    // Ölçek (Scale) - Doğrusal Enterpolasyon (Lerp)
    result.scale = fe_vec3_lerp(t1.scale, t2.scale, factor);

    return result;
}

/**
 * @brief Bir kemik kanalındaki belirli bir zamanda o kemiğin dönüsümünü hesaplar.
 * * Gerçek implementasyonda Keyframe arama ve enterpolasyon yapılır.
 */
static fe_anim_transform_t fe_get_bone_transform_at_time(fe_bone_channel_t* channel, float time) {
    // Basit bir yer tutucu (dummy): Sadece kimlik dönüşümünü döndür.
    // Gerçekte: zamanı kullanarak ilgili keyframe çiftlerini bulur ve aralarında enterpolasyon yapar.
    
    fe_anim_transform_t identity = {
        .position = FE_VEC3_ZERO,
        .rotation = FE_QUAT_IDENTITY,
        .scale = FE_VEC3_ONE
    };
    
    // ******* Gerçek keyframe mantığı burada yer alacaktır *******
    // if (channel->position_keys.count < 2) return identity;
    // ... Keyframe enterpolasyonu
    
    return identity;
}


// ----------------------------------------------------------------------
// 3. SİSTEM YÖNETİMİ VE GÜNCELLEME UYGULAMALARI
// ----------------------------------------------------------------------

/**
 * Uygulama: fe_animation_init
 */
bool fe_animation_init(void) {
    FE_LOG_INFO("Animasyon Sistemi baslatildi.");
    return true;
}

/**
 * Uygulama: fe_animation_shutdown
 */
void fe_animation_shutdown(void) {
    FE_LOG_INFO("Animasyon Sistemi kapatildi.");
}

/**
 * Uygulama: fe_anim_instance_update
 */
void fe_anim_instance_update(fe_anim_instance_t* instance, float dt) {
    if (!instance || !instance->active_clip || !instance->skeleton) return;
    
    // 1. Zamanı güncelle
    instance->current_time += dt * instance->active_clip->ticks_per_second;
    
    // Klibin sonuna ulaşıldıysa döngüye al
    if (instance->current_time >= instance->active_clip->duration) {
        instance->current_time = fmodf(instance->current_time, instance->active_clip->duration);
    }
    
    // 2. Nihai dönüşümlerin çıktısı için fe_array_t'yi hazırla
    if (fe_array_count(&instance->final_transforms) != instance->skeleton->bone_count) {
        // Diziyi yeniden boyutlandır (fe_array_t kullanıldığı varsayılır)
        // fe_array_resize(&instance->final_transforms, instance->skeleton->bone_count);
    }

    // Kök düğümden başlayarak kemik hiyerarşisini gez
    // Bu, genellikle özyinelemeli (recursive) bir fonksiyonda yapılır.
    
    // fe_calculate_bone_transforms(instance, instance->current_time, root_bone_id, FE_MAT4_IDENTITY);
    
    FE_LOG_DEBUG("Animasyon Instance güncellendi (Time: %.2f)", instance->current_time);
}


 // * @brief Özyinelemeli olarak tüm kemiklerin dünya dönüşümlerini hesaplar (Sadece Konsept)
//  * * Bu, gerçek iskelet animasyonunun kalbidir.
 

static void fe_calculate_bone_transforms(fe_anim_instance_t* instance, float time, uint32_t bone_id, fe_mat4_t parent_world_transform) {
    fe_bone_t* bone = fe_array_get(&instance->skeleton->bones, bone_id);
    fe_bone_channel_t* channel = fe_find_channel_for_bone(instance->active_clip, bone_id);
    
    // 1. Yerel Animasyon Dönüşümünü hesapla
    fe_anim_transform_t local_anim_transform = fe_get_bone_transform_at_time(channel, time);
    fe_mat4_t local_anim_matrix = fe_mat4_from_transform(local_anim_transform);
    
    // 2. Dünya Dönüşümünü hesapla
    fe_mat4_t global_transform = fe_mat4_multiply(parent_world_transform, local_anim_matrix);
    
    // 3. Final Dönüşümünü (Vertex Shader'a gönderilecek) hesapla
    // Final = Global_Transform * Inverse_Bind_Pose
    fe_mat4_t final_transform = fe_mat4_multiply(global_transform, fe_mat4_from_transform(bone->offset));
    
    fe_array_set(&instance->final_transforms, bone_id, &final_transform);
    
    // 4. Alt kemikleri gez
    for (child_bone_id in bone->children) {
    fe_calculate_bone_transforms(instance, time, child_bone_id, global_transform);
    }
}
