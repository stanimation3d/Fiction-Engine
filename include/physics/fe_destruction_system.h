// include/physics/fe_destruction_system.h

#ifndef FE_DESTRUCTION_SYSTEM_H
#define FE_DESTRUCTION_SYSTEM_H

#include <stdint.h>
#include <stdbool.h>
#include "math/fe_vector.h"
#include "physics/fe_rigid_body.h"
#include "data_structures/fe_array.h" // Parçalanma parçaları ve bileşenler için

// ----------------------------------------------------------------------
// 1. KIRILMA PARÇALARI YAPISI
// ----------------------------------------------------------------------

/**
 * @brief Yikim sonrasi olusacak tek bir parçayi (kırıntıyı) temsil eder.
 */
typedef struct fe_fracture_chunk {
    fe_rigid_body_t* rigid_body; // Parçanın katı cisim verisi (Kütle, şekil, pozisyon)
    uint32_t mesh_id;            // Parçaya ait görsel mesh verisi referansi (Render sistemi için)
    // Ek olarak: Yüzey alanı, iç/dış yüzey bilgisi gibi veriler eklenebilir.
} fe_fracture_chunk_t;

/**
 * @brief Önceden hesaplanmis yikim verisini tutan bir mesh kümesi.
 */
typedef struct fe_fracture_mesh {
    uint32_t id;
    fe_array_t* chunks; // fe_fracture_chunk_t turunde dizisi
    // Orijinal nesnenin hacim, kütle, vb. bilgileri burada depolanabilir.
} fe_fracture_mesh_t;

// ----------------------------------------------------------------------
// 2. YIKILABİLİR BİLEŞEN
// ----------------------------------------------------------------------

/**
 * @brief Bir fe_rigid_body'yi yikilabilir yapan bilesen.
 */
typedef struct fe_destructible_component {
    uint32_t id;
    fe_rigid_body_t* target_body;   // Yikilacak olan ana katı cisim
    
    fe_fracture_mesh_t* fracture_data; // Yikim sonrasi olusacak parçalarin verisi

    float health;                   // Mevcut dayanıklılık
    float impulse_threshold;        // Yikimi tetiklemek için gereken minimum darbe (Impulse) büyüklügü
    
    float kinetic_energy_threshold; // Yikimi tetiklemek için gereken minimum Kinetik Enerji (0.5 * m * v^2)

    bool is_pending_destruction;    // Yikim tetiklendi mi?
    bool is_destroyed;              // Zaten yikildi mi?
} fe_destructible_component_t;


// ----------------------------------------------------------------------
// 3. YÖNETİM VE SİSTEM FONKSİYONLARI
// ----------------------------------------------------------------------

/**
 * @brief Yeni bir yikilabilir bilesen olusturur.
 */
fe_destructible_component_t* fe_destruction_create_component(
    fe_rigid_body_t* target_body, 
    fe_fracture_mesh_t* fracture_data, 
    float health, 
    float threshold);

/**
 * @brief Bileseni bellekten serbest birakir.
 */
void fe_destruction_destroy_component(fe_destructible_component_t* dest_comp);

/**
 * @brief Çarpisma aninda yikilabilir bilesene darbe (impulse) uygular.
 * * Bu, çarpisma çözücüsü tarafindan çagrilir.
 * @param dest_comp Yikilabilir bilesen.
 * @param impulse_magnitude Uygulanan darbenin büyüklügü.
 */
void fe_destruction_apply_impulse(fe_destructible_component_t* dest_comp, float impulse_magnitude);

/**
 * @brief Yikilabilir bilesenleri kontrol eder ve gerekli durumlarda yikimi tetikler.
 * * Bu, fe_physics_manager_step içinde çagrilir.
 * @param dest_comp Kontrol edilecek bilesen.
 */
void fe_destruction_check_and_process(fe_destructible_component_t* dest_comp);

/**
 * @brief Yikim islemini gerçekleştirir:
 * * 1. Ana cismi siler.
 * * 2. Parça rigid body'leri fizik motoruna enjekte eder.
 * * 3. Parçalara patlama/yayılma dürtüsü verir.
 */
void fe_destruction_perform(fe_destructible_component_t* dest_comp);

#endif // FE_DESTRUCTION_SYSTEM_H