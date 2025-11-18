// include/physics/fe_world.h

#ifndef FE_WORLD_H
#define FE_WORLD_H

#include <stdint.h>
#include <stdbool.h>
#include "math/fe_vector.h"

// ----------------------------------------------------------------------
// 1. DÜNYA SINIR YAPISI
// ----------------------------------------------------------------------

/**
 * @brief Fizik dünyasının sınırlarını tanımlar (Axis-Aligned Bounding Box - AABB).
 * * Bu sinirlar disina çikan cisimler yok edilebilir veya durdurulabilir.
 */
typedef struct fe_world_bounds {
    fe_vec3_t min; // Sinirin en düsük x, y, z koordinatlari
    fe_vec3_t max; // Sinirin en yüksek x, y, z koordinatlari
} fe_world_bounds_t;

// ----------------------------------------------------------------------
// 2. DÜNYA AYARLARI YAPISI
// ----------------------------------------------------------------------

/**
 * @brief Genel fiziksel ortam ayarlarini tutar.
 */
typedef struct fe_world_settings {
    // Statik Ayarlar
    fe_world_bounds_t bounds;    // Simülasyon sinirlari
    float air_density;           // Hava yogunlugu (Sürükleme kuvveti hesaplamalari için)
    float wind_speed;            // Genel rüzgar hizi (fe_physics_fields'a baglanabilir)
    
    // Performans/Optimizasyon Ayarlari
    int max_rigid_bodies;        // Dünyada izin verilen maksimum cisim sayisi
    bool enable_sleeping;        // Uyku optimizasyonunun etkin olup olmamasi
} fe_world_settings_t;


// ----------------------------------------------------------------------
// 3. YÖNETİM VE SORGULAMA FONKSİYONLARI
// ----------------------------------------------------------------------

/**
 * @brief Varsayilan dünya ayarlari ile yeni bir dünya yapisi olusturur.
 * * @return Baslatilmis fe_world_settings_t yapisi.
 */
fe_world_settings_t fe_world_create_default_settings(void);

/**
 * @brief Bir noktanin fizik sinirlari içinde olup olmadigini kontrol eder.
 */
bool fe_world_is_point_in_bounds(const fe_world_settings_t* settings, fe_vec3_t point);


#endif // FE_WORLD_H