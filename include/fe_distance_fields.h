// include/graphics/dynamicr/fe_distance_fields.h

#ifndef FE_DISTANCE_FIELDS_H
#define FE_DISTANCE_FIELDS_H

#include <stdint.h>
#include <stdbool.h>
#include "error/fe_error.h"
#include "graphics/fe_render_types.h"
#include "math/fe_vector.h" // fe_vec3_t için

// ----------------------------------------------------------------------
// 1. UZAKLIK ALANI YAPILARI
// ----------------------------------------------------------------------

/**
 * @brief Sahnenin Uzaklik Alanini (Distance Field) tutan yapi.
 */
typedef struct fe_distance_field {
    fe_texture_id_t sdf_volume_id; // 3D Doku Volume ID'si (GL_TEXTURE_3D)
    fe_vec3_t world_min;           // Volume'un kapsadigi alanin Dünya koordinatindaki min noktasi
    fe_vec3_t world_max;           // Volume'un kapsadigi alanin Dünya koordinatindaki max noktasi
    uint32_t resolution;           // Volume'un cozunurlugu (NxNxN)
    
    fe_material_t* creation_material; // SDF olusturma (Compute) shader'ini tutar
} fe_distance_field_t;


// ----------------------------------------------------------------------
// 2. YÖNETİM FONKSİYONLARI
// ----------------------------------------------------------------------

/**
 * @brief Uzaklik Alanı sistemini baslatir ve SDF Volume'unu olusturur.
 * * @param world_min Volume'un kapsayacagi alanin min noktasi.
 * @param world_max Volume'un kapsayacagi alanin max noktasi.
 * @param resolution Volume cozunurlugu (orn: 128 -> 128x128x128).
 * @return Yeni olusturulan fe_distance_field_t pointer'i. Basarisiz olursa NULL.
 */
fe_distance_field_t* fe_distance_field_init(fe_vec3_t world_min, fe_vec3_t world_max, uint32_t resolution);

/**
 * @brief Uzaklik Alanı sistemini kapatir ve kaynaklari serbest birakir.
 */
void fe_distance_field_shutdown(fe_distance_field_t* df);

/**
 * @brief Sahnedeki statik geometriyi kullanarak SDF Volume'unu gunceller/yeniden olusturur.
 * * Bu, Compute Shader uzerinden yapilir.
 * @param df Hedef Uzaklik Alanı.
 * @param static_meshes Sahnedeki statik mesh'ler (SDF'e dahil edilecek geometri).
 * @param mesh_count Mesh sayisi.
 */
void fe_distance_field_rebuild(fe_distance_field_t* df, const fe_mesh_t* const* static_meshes, uint32_t mesh_count);

#endif // FE_DISTANCE_FIELDS_H