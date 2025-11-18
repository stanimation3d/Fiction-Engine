// src/physics/fe_physical_materials.c

#include "physics/fe_physical_materials.h"
#include <math.h> // fminf, fmaxf için

// ----------------------------------------------------------------------
// 1. SABİT MALZEMELERİN TANIMLARI
// ----------------------------------------------------------------------

/**
 * Uygulama: Varsayilan Malzeme (Ortalama Toprak/Taş)
 */
const fe_physical_material_t FE_MAT_DEFAULT = {
    .name = "Default",
    .density = 1000.0f,      // 1000 kg/m^3 (Suyun yoğunluğu, iyi bir başlangıç)
    .static_friction = 0.6f,
    .dynamic_friction = 0.5f,
    .restitution = 0.3f      // Biraz seker
};

/**
 * Uygulama: Lastik
 */
const fe_physical_material_t FE_MAT_RUBBER = {
    .name = "Rubber",
    .density = 1500.0f,      // Daha yoğun
    .static_friction = 0.9f, // Yüksek sürtünme
    .dynamic_friction = 0.7f,
    .restitution = 0.8f      // Çok seker
};

/**
 * Uygulama: Buz
 */
const fe_physical_material_t FE_MAT_ICE = {
    .name = "Ice",
    .density = 917.0f,       // Daha az yoğun
    .static_friction = 0.05f, // Çok düşük sürtünme
    .dynamic_friction = 0.02f,
    .restitution = 0.1f       // Az seker
};

/**
 * Uygulama: Metal
 */
const fe_physical_material_t FE_MAT_METAL = {
    .name = "Metal",
    .density = 7850.0f,      // Çok yoğun
    .static_friction = 0.4f,
    .dynamic_friction = 0.3f,
    .restitution = 0.2f       // Neredeyse hiç sekmez (eğilme simülasyonu dahil edilmediği için)
};


// ----------------------------------------------------------------------
// 2. KOMBİNASYON KURALLARI
// ----------------------------------------------------------------------

/**
 * Uygulama: fe_material_combine_static_friction
 * Kural: Statik ve Dinamik Sürtünme, genellikle iki katsayının geometrik ortalaması (sqrt(a * b)) 
 * veya en küçük olanı (fminf) alınarak hesaplanır. En küçük olanı almak, çarpışma yüzeylerinin 
 * en zayıf noktasını temsil ettiği için daha güvenli bir yaklaşımdır.
 */
float fe_material_combine_static_friction(const fe_physical_material_t* mat_a, const fe_physical_material_t* mat_b) {
    // Geometrik Ortalama: sqrt(a * b)
    return sqrtf(mat_a->static_friction * mat_b->static_friction);
    
    // Minimum: (En güvenli yaklaşım)
    return fminf(mat_a->static_friction, mat_b->static_friction);
}

/**
 * Uygulama: fe_material_combine_restitution
 * Kural: Esneklik (Sekme), genellikle iki katsayının minimumu alınır. 
 * (Bir cam ile bir lastik çarpışırsa, daha az esnek olan camın davranışı baskın olur).
 */
float fe_material_combine_restitution(const fe_physical_material_t* mat_a, const fe_physical_material_t* mat_b) {
    // Minimum:
    return fminf(mat_a->restitution, mat_b->restitution);
}

// Dinamik sürtünme için de benzer bir fonksiyon eklenebilir.
float fe_material_combine_dynamic_friction(const fe_physical_material_t* mat_a, const fe_physical_material_t* mat_b) {
   return fminf(mat_a->dynamic_friction, mat_b->dynamic_friction);
 }