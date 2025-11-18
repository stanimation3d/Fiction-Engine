// include/math/fe_vertex.h

#ifndef FE_VERTEX_H
#define FE_VERTEX_H

#include <stdint.h>
#include "fe_vector.h" // fe_vec3_t, fe_vec2_t gibi vektör türlerini kullanmak için
#include "fe_color.h"  // fe_color_t türünü kullanmak için

// ----------------------------------------------------------------------
// 1. KÖŞE NOKTASI (VERTEX) YAPISI
// ----------------------------------------------------------------------

/**
 * @brief Bir 3D mesh'teki tek bir köşe noktasını (vertex) temsil eder.
 * * Bu yapi, genellikle GPU'ya iletilen verinin formatini tanimlar.
 */
typedef struct fe_vertex {
    // 1. Konum (Pozisyon)
    fe_vec3_t position;        // Köşe noktasinin dünya/yerel uzaydaki konumu (XYZ)

    // 2. Normal
    fe_vec3_t normal;          // Yüzeyin bu noktadaki normal vektörü (Aydınlatma için)

    // 3. Doku Koordinatları (UV)
    fe_vec2_t uv;              // Dokuyu haritalamak için 2D koordinatlar (UV)

    // 4. Renk (Opsiyonel)
    fe_color_t color;          // Köşe noktasinin rengi (RGBA - fe_color_t yapisinda oldugu varsayilir)
    
    // 5. Teğet ve İkincil Normal (Tangent and Bitangent/Binormal)
    // Detayli aydinlatma/Normal Haritalama (Normal Mapping) için gereklidir.
    fe_vec3_t tangent;         // Teğet vektörü (T)
    fe_vec3_t bitangent;       // İkincil normal vektörü (B)

    // 6. Kemik Etkileşimi (Opsiyonel - İskelet Animasyonu için)
     uint32_t bone_indices[4]; // Etkileyen kemiklerin indeksleri
     float bone_weights[4];    // Her kemiğin ağırlığı

} fe_vertex_t;


// ----------------------------------------------------------------------
// 2. YÖNETİM VE YARDIMCI FONKSİYONLAR
// ----------------------------------------------------------------------

/**
 * @brief Yeni bir fe_vertex_t olusturur ve temel degerleri ayarlar.
 */
fe_vertex_t fe_vertex_create(fe_vec3_t pos, fe_vec3_t norm, fe_vec2_t uv);

/**
 * @brief Bir köşe noktasini digerine kopyalar.
 */
void fe_vertex_copy(fe_vertex_t* dest, const fe_vertex_t* src);

/**
 * @brief Köşe noktasina Teğet ve İkincil Normal (TBN) degerlerini atar.
 */
void fe_vertex_set_tbn(fe_vertex_t* vertex, fe_vec3_t tangent, fe_vec3_t bitangent);


#endif // FE_VERTEX_H