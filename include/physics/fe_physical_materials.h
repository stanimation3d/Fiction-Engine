// include/physics/fe_physical_materials.h

#ifndef FE_PHYSICAL_MATERIALS_H
#define FE_PHYSICAL_MATERIALS_H

#include <stdint.h>
#include <stdbool.h>

// ----------------------------------------------------------------------
// 1. MALZEME YAPISI
// ----------------------------------------------------------------------

/**
 * @brief Bir cismin temel fiziksel özelliklerini tanımlar.
 */
typedef struct fe_physical_material {
    const char* name;      // Malzemenin okunabilir adi (örn: "Demir", "Lastik")
    float density;         // Yoğunluk (kg/m^3). Kütle = Hacim * Yoğunluk
    float static_friction; // Statik sürtünme katsayisi (Cisim dururken kaymaya baslama direnci)
    float dynamic_friction;  // Dinamik (kinetik) sürtünme katsayisi (Cisim hareket ederken kayma direnci)
    float restitution;     // Esneklik katsayisi (Sekme orani, 0.0: hiç sekmez, 1.0: mükemmel esnek)
} fe_physical_material_t;

// ----------------------------------------------------------------------
// 2. TEMEL SABİT MALZEMELER (Önceden Tanımlı)
// ----------------------------------------------------------------------

// Harici erisim için bildirimler
extern const fe_physical_material_t FE_MAT_DEFAULT;
extern const fe_physical_material_t FE_MAT_RUBBER;
extern const fe_physical_material_t FE_MAT_ICE;
extern const fe_physical_material_t FE_MAT_METAL;

// ----------------------------------------------------------------------
// 3. YÖNETİM VE KOMBİNASYON FONKSİYONLARI
// ----------------------------------------------------------------------

/**
 * @brief Iki malzeme arasindaki efektif sürtünme katsayisini hesaplar.
 * * Genellikle ortalama veya minimum alinir.
 * @param mat_a Birinci malzeme.
 * @param mat_b Ikinci malzeme.
 * @return Hesaplanan statik sürtünme katsayisi.
 */
float fe_material_combine_static_friction(const fe_physical_material_t* mat_a, const fe_physical_material_t* mat_b);

/**
 * @brief Iki malzeme arasindaki efektif esneklik katsayisini hesaplar (Sekme).
 * * Genellikle minimum alinir.
 * @param mat_a Birinci malzeme.
 * @param mat_b Ikinci malzeme.
 * @return Hesaplanan esneklik katsayisi.
 */
float fe_material_combine_restitution(const fe_physical_material_t* mat_a, const fe_physical_material_t* mat_b);

#endif // FE_PHYSICAL_MATERIALS_H