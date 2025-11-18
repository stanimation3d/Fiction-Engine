// src/physics/fe_world.c

#include "physics/fe_world.h"
#include "utils/fe_logger.h"
#include <math.h> // fmaxf, fminf gibi fonksiyonlar için

// ----------------------------------------------------------------------
// 1. OLUŞTURMA VE BAŞLATMA
// ----------------------------------------------------------------------

/**
 * Uygulama: fe_world_create_default_settings
 */
fe_world_settings_t fe_world_create_default_settings(void) {
    fe_world_settings_t settings = {0};
    
    // Varsayilan dünya sinirlari (1000x1000x1000 metrelik bir küp)
    settings.bounds.min = (fe_vec3_t){-500.0f, -500.0f, -500.0f};
    settings.bounds.max = (fe_vec3_t){500.0f, 500.0f, 500.0f};
    
    // Çevre (Ortalama hava yoğunluğu)
    settings.air_density = 1.225f; // kg/m^3 (Deniz seviyesinde)
    settings.wind_speed = 0.0f;    // Varsayilan rüzgar yok
    
    // Optimizasyon
    settings.max_rigid_bodies = 10000;
    settings.enable_sleeping = true;
    
    FE_LOG_DEBUG("Varsayilan Fizik Dünyasi Ayarlari Olusturuldu.");
    return settings;
}

// ----------------------------------------------------------------------
// 2. SORGULAMA FONKSİYONLARI
// ----------------------------------------------------------------------

/**
 * Uygulama: fe_world_is_point_in_bounds
 * AABB testi: Nokta, x, y ve z eksenlerinde [min, max] araliginda olmalidir.
 */
bool fe_world_is_point_in_bounds(const fe_world_settings_t* settings, fe_vec3_t point) {
    const fe_vec3_t min = settings->bounds.min;
    const fe_vec3_t max = settings->bounds.max;

    // X ekseni kontrolü
    if (point.x < min.x || point.x > max.x) return false;
    
    // Y ekseni kontrolü
    if (point.y < min.y || point.y > max.y) return false;
    
    // Z ekseni kontrolü
    if (point.z < min.z || point.z > max.z) return false;

    return true;
}

// TODO: fe_world_raycast_query(ray, max_distance) gibi fonksiyonlar buraya gelecektir.