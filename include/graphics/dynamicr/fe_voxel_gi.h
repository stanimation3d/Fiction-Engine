// include/graphics/dynamicr/fe_voxel_gi.h

#ifndef FE_VOXEL_GI_H
#define FE_VOXEL_GI_H

#include <stdint.h>
#include <stdbool.h>
#include "error/fe_error.h"
#include "graphics/fe_render_types.h"
#include "graphics/fe_material_editor.h" 
#include "math/fe_matrix.h" 
#include "math/fe_vector.h" 

// ----------------------------------------------------------------------
// 1. VOXEL IZGARA VE KONTEKS YAPISI
// ----------------------------------------------------------------------

#define VOXEL_GRID_RESOLUTION 128 // Ornek cozunurluk (128x128x128)

/**
 * @brief Voxel Izgarasi verilerini tutan yapi.
 */
typedef struct fe_voxel_grid {
    fe_texture_id_t opacity_volume_id;  // 3D Doku: Opaklik (Bir voxel'in dolu olup olmadigi)
    fe_texture_id_t radiance_volume_id; // 3D Doku: Birikmis isinimi (Dolayli Isik)
    fe_vec3_t world_min;                // Izgaranin kapsadigi alanin Dünya koordinatindaki min noktasi
    fe_vec3_t world_max;                // Izgaranin kapsadigi alanin Dünya koordinatindaki max noktasi
    uint32_t resolution;
} fe_voxel_grid_t;

/**
 * @brief Voxel GI Pass'ini yoneten baglam yapi.
 */
typedef struct fe_voxel_gi_context {
    fe_voxel_grid_t grid;
    fe_material_t* voxelization_material; // Geometriyi Voxelize eden shader (Vertex/Geometry/Fragment)
    fe_material_t* inject_material;       // Isiklandirma verisini ızgaraya enjekte eden Compute Shader
    fe_material_t* tracing_material;      // Voxel ızgarasinda isin takibi yapan Compute Shader
} fe_voxel_gi_context_t;


// ----------------------------------------------------------------------
// 2. YÖNETİM VE ÇİZİM FONKSİYONLARI
// ----------------------------------------------------------------------

/**
 * @brief Voxel GI sistemini baslatir (Izgara Volume'larini ve Shader'lari yukler).
 * * @param world_min Izgaranin kapsayacagi alanin min noktasi.
 * @param world_max Izgaranin kapsayacagi alanin max noktasi.
 * @return Baslatilan baglam (context) pointer'i. Basarisiz olursa NULL.
 */
fe_voxel_gi_context_t* fe_voxel_gi_init(fe_vec3_t world_min, fe_vec3_t world_max);

/**
 * @brief Voxel GI sistemini kapatir ve Volume kaynaklarini serbest birakir.
 */
void fe_voxel_gi_shutdown(fe_voxel_gi_context_t* context);

/**
 * @brief Sahne geometrisini Voxel ızgarasına kodlar (Voxelization Pass).
 * * @param context Voxel GI baglami.
 * @param meshes Voxelize edilecek mesh listesi.
 * @param mesh_count Mesh sayisi.
 */
void fe_voxel_gi_voxelize_scene(fe_voxel_gi_context_t* context, 
                                 const fe_mesh_t* const* meshes, 
                                 uint32_t mesh_count);

/**
 * @brief Voxel ızgarasına dinamik isiklandirma verisini enjekte eder (Injection Pass).
 * * @param context Voxel GI baglami.
 * @param camera_position Mevcut kamera pozisyonu (dinamik isiklarin güncellenmesi icin).
 */
void fe_voxel_gi_inject_radiance(fe_voxel_gi_context_t* context, fe_vec3_t camera_position);

/**
 * @brief Voxel ızgarasinda isin takibi yapar ve dolayli isigi hesaplar (Tracing Pass).
 * * @param context Voxel GI baglami.
 */
void fe_voxel_gi_trace_and_accumulate(fe_voxel_gi_context_t* context);


#endif // FE_VOXEL_GI_H