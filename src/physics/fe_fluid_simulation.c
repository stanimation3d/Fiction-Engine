// src/physics/fe_fluid_simulation.c

#include "physics/fe_fluid_simulation.h"
#include "utils/fe_logger.h"
#include <stdlib.h> // malloc, free, rand
#include <math.h>   // powf, sqrtf, fabsf, M_PI

// Sabitler
#define SPH_MASS (0.001f) // Her parçacığın varsayılan kütlesi
#define SPH_DENSITY_REST (1000.0f) // Suyun yoğunluğu (kg/m^3)
#define SPH_VISCOSITY (0.02f)
#define SPH_SMOOTHING_RADIUS (0.1f) // 10 cm

// Dahili Çekirdek Fonksiyonları (W) ve Türevleri (Grad W, Lap W)
// Kübik B-spline çekirdeği varsayılır.

/**
 * @brief SPH Yoğunluk Çekirdek Fonksiyonu W(r, h).
 * * r: parçacıklar arasındaki mesafe, h: smoothing_radius.
 */
static float W_density(float r, float h) {
    if (r < 0.0f || r > h) return 0.0f;
    float q = r / h;
    float h2 = h * h;
    float h3 = h * h2;
    // Kübik spline çekirdek sabiti (2D: 10/7pi*h2, 3D: 15/pi*h3)
    const float alpha = 15.0f / (M_PI * h3); 
    return alpha * (1.0f - q * q) * (1.0f - q * q) * (1.0f - q * q);
}

/**
 * @brief SPH Basınç Kuvveti Çekirdek Fonksiyonunun Gradyenti Grad W_pressure(r, h).
 * * Genellikle W'nun bir türevi kullanılır, burada türevlenmiş spline.
 */
static fe_vec3_t GradW_pressure(fe_vec3_t delta, float r, float h) {
    if (r < 0.0001f || r > h) return (fe_vec3_t){0};
    
    float q = r / h;
    float h5 = h*h*h*h*h;
    // Gradyent çekirdek sabiti (3D: 45/pi*h5)
    const float alpha_grad = -45.0f / (M_PI * h5); 
    
    float factor = alpha_grad * (h - r) * (h - r);
    return fe_vec3_scale(delta, factor / r); // delta/r = normalize(delta)
}

/**
 * @brief SPH Viskozite Kuvveti Çekirdek Fonksiyonunun Laplacien'i Lap W_viscosity(r, h).
 * * Genellikle W'nun ikinci türevi kullanılır.
 */
static float LapW_viscosity(float r, float h) {
    if (r > h) return 0.0f;
    float q = r / h;
    float h5 = h*h*h*h*h;
    // Laplacian çekirdek sabiti (3D: 45/pi*h5)
    const float alpha_lap = 45.0f / (M_PI * h5); 
    return alpha_lap * (h - r);
}


// ----------------------------------------------------------------------
// 3. YÖNETİM VE YAŞAM DÖNGÜSÜ
// ----------------------------------------------------------------------

static uint32_t g_next_fluid_id = 1;

/**
 * Uygulama: fe_fluid_create_volume
 */
fe_fluid_volume_t* fe_fluid_create_volume(uint32_t particle_count, fe_vec3_t volume_size) {
    fe_fluid_volume_t* volume = (fe_fluid_volume_t*)calloc(1, sizeof(fe_fluid_volume_t));
    if (!volume) return NULL;
    
    volume->id = g_next_fluid_id++;
    volume->particles = fe_array_create(sizeof(fe_fluid_particle_t));
    
    // Varsayılan Ayarlar
    volume->smoothing_radius_h = SPH_SMOOTHING_RADIUS;
    volume->rest_density = SPH_DENSITY_REST;
    volume->viscosity_mu = SPH_VISCOSITY;
    volume->stiffness_k = 100.0f; // Sıkıştırılamazlık için yüksek değer
    volume->volume_max = volume_size;

    // Parçacıkları rastgele/düzenli yerleştirme (Şimdilik rastgele)
    for (uint32_t i = 0; i < particle_count; ++i) {
        fe_fluid_particle_t p = {0};
        p.mass = SPH_MASS;
        p.position.x = (float)rand() / (float)RAND_MAX * volume_size.x;
        p.position.y = (float)rand() / (float)RAND_MAX * volume_size.y;
        p.position.z = (float)rand() / (float)RAND_MAX * volume_size.z;
        
        fe_array_push(volume->particles, &p);
    }
    
    // TODO: Uzamsal Karma Tablosunu Başlat
    
    FE_LOG_INFO("Akışkan Hacmi %u olusturuldu. Parçacik: %zu", volume->id, fe_array_count(volume->particles));
    return volume;
}

/**
 * Uygulama: fe_fluid_destroy_volume
 */
void fe_fluid_destroy_volume(fe_fluid_volume_t* volume) {
    if (volume) {
        if (volume->particles) fe_array_destroy(volume->particles);
        // TODO: Uzamsal Karma Tablosunu Serbest Bırak
        free(volume);
    }
}

// ----------------------------------------------------------------------
// 4. SİMÜLASYON ADIMLARI
// ----------------------------------------------------------------------

/**
 * @brief SPH Adim 1: Yoğunluk ve Basıncı Hesapla.
 */
static void fe_fluid_calculate_density_and_pressure(fe_fluid_volume_t* volume) {
    size_t count = fe_array_count(volume->particles);
    float h = volume->smoothing_radius_h;
    
    // TODO: Komşu Arama (Uzamsal Karma Tablo kullanımı) burada olmalıdır.
    
    for (size_t i = 0; i < count; ++i) {
        fe_fluid_particle_t* pi = (fe_fluid_particle_t*)fe_array_get(volume->particles, i);
        pi->density = 0.0f;
        
        // Basitlik için tüm parçacıklar kontrol edilir (YAVAŞ!)
        for (size_t j = 0; j < count; ++j) {
            fe_fluid_particle_t* pj = (fe_fluid_particle_t*)fe_array_get(volume->particles, j);

            fe_vec3_t delta = fe_vec3_subtract(pi->position, pj->position);
            float r = fe_vec3_length(delta);
            
            // Yoğunluk katkısı: rho = Sum(m * W(r, h))
            pi->density += pj->mass * W_density(r, h);
        }
        
        // Basıncı hesapla (İdeal gaz denklemi)
        // P = k * (rho - rho0)
        pi->pressure = volume->stiffness_k * fmaxf(0.0f, pi->density - volume->rest_density);
    }
}

/**
 * @brief SPH Adim 2: Basınç ve Viskozite Kuvvetlerini Hesapla ve Uygula.
 */
static void fe_fluid_calculate_and_apply_forces(fe_fluid_volume_t* volume, fe_vec3_t gravity) {
    size_t count = fe_array_count(volume->particles);
    float h = volume->smoothing_radius_h;
    
    for (size_t i = 0; i < count; ++i) {
        fe_fluid_particle_t* pi = (fe_fluid_particle_t*)fe_array_get(volume->particles, i);
        
        pi->force_accumulator = fe_vec3_scale(gravity, pi->mass); // Yerçekimi başlangıç

        fe_vec3_t f_pressure = {0};
        fe_vec3_t f_viscosity = {0};

        // Basitlik için tüm parçacıklar kontrol edilir
        for (size_t j = 0; j < count; ++j) {
            if (i == j) continue; // Kendini atla

            fe_fluid_particle_t* pj = (fe_fluid_particle_t*)fe_array_get(volume->particles, j);

            fe_vec3_t delta = fe_vec3_subtract(pi->position, pj->position);
            float r = fe_vec3_length(delta);

            if (r > h) continue; // Etkileşim yarıçapı dışında

            // A. Basınç Kuvveti (P_i + P_j) / (2 * rho_i * rho_j) * grad W * m_i * m_j
            float avg_pressure = (pi->pressure + pj->pressure) * 0.5f;
            fe_vec3_t grad_w = GradW_pressure(delta, r, h);
            
            fe_vec3_t pressure_term = fe_vec3_scale(grad_w, 
                -(pj->mass / pj->density) * avg_pressure);
            f_pressure = fe_vec3_add(f_pressure, pressure_term);

            // B. Viskozite Kuvveti (Viskozite * (v_j - v_i) / rho_j * Lap W * m_i * m_j)
            fe_vec3_t velocity_diff = fe_vec3_subtract(pj->velocity, pi->velocity);
            float lap_w = LapW_viscosity(r, h);

            fe_vec3_t viscosity_term = fe_vec3_scale(velocity_diff, 
                (volume->viscosity_mu * pj->mass / pj->density) * lap_w);
            f_viscosity = fe_vec3_add(f_viscosity, viscosity_term);
        }

        // Toplam SPH Kuvveti
        pi->force_accumulator = fe_vec3_add(pi->force_accumulator, f_pressure);
        pi->force_accumulator = fe_vec3_add(pi->force_accumulator, f_viscosity);
    }
}

/**
 * @brief SPH Adim 3: Hız ve Konumu Güncelle (Entegrasyon)
 */
static void fe_fluid_integrate_and_handle_boundaries(fe_fluid_volume_t* volume, float dt) {
    size_t count = fe_array_count(volume->particles);

    for (size_t i = 0; i < count; ++i) {
        fe_fluid_particle_t* p = (fe_fluid_particle_t*)fe_array_get(volume->particles, i);

        // a. Hız Güncelleme (Euler)
        fe_vec3_t acceleration = fe_vec3_scale(p->force_accumulator, 1.0f / p->mass);
        p->velocity = fe_vec3_add(p->velocity, fe_vec3_scale(acceleration, dt));

        // b. Konum Güncelleme
        p->position = fe_vec3_add(p->position, fe_vec3_scale(p->velocity, dt));

        // c. Basit Sınır Çarpışması Çözümü (Kutu sınırları)
        float restitution = 0.5f; // Geri sekme katsayısı
        
        if (p->position.x < volume->volume_min.x) {
            p->position.x = volume->volume_min.x;
            p->velocity.x *= -restitution;
        } else if (p->position.x > volume->volume_max.x) {
            p->position.x = volume->volume_max.x;
            p->velocity.x *= -restitution;
        }
        // ... Y ve Z için de benzer sınır kontrolleri uygulanır ...
    }
}


/**
 * Uygulama: fe_fluid_simulate_step
 */
void fe_fluid_simulate_step(fe_fluid_volume_t* volume, fe_vec3_t gravity, float dt) {
    if (!volume) return;

    // 1. Komşuları Güncelle (TODO: Gerekli ise spatial hash)
    
    // 2. Yoğunluk ve Basıncı Hesapla
    fe_fluid_calculate_density_and_pressure(volume);

    // 3. Basınç ve Viskozite Kuvvetlerini Hesapla ve Uygula
    fe_fluid_calculate_and_apply_forces(volume, gravity);

    // 4. Konum ve Hızı Güncelle + Sınırları Çöz
    fe_fluid_integrate_and_handle_boundaries(volume, dt);
    
    
}