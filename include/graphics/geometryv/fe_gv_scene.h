// include/graphics/geometryv/fe_gv_scene.h

#ifndef FE_GV_SCENE_H
#define FE_GV_SCENE_H

#include <stdint.h>
#include <stdbool.h>
#include "error/fe_error.h"
#include "graphics/fe_render_types.h"
#include "math/fe_vector.h" 
#include "math/fe_matrix.h" 

// ----------------------------------------------------------------------
// 1. GEOMETRYV VERİ YAPILARI
// ----------------------------------------------------------------------

/**
 * @brief Sahnedeki geometrinin kucuk bir bolumunu (bir grup ucgeni) temsil eden kumeler.
 */
typedef struct fe_gv_cluster {
    fe_vec3_t aabb_min;       // Kumenin Sınırlayıcı Kutusu (AABB) min noktasi
    fe_vec3_t aabb_max;       // Kumenin Sınırlayıcı Kutusu (AABB) max noktasi
    uint32_t first_triangle_idx; // Bu kumeye ait ilk ucgenin sahne ucgen listesindeki indeksi
    uint32_t triangle_count;     // Bu kumeye ait ucgen sayisi
} fe_gv_cluster_t;

/**
 * @brief Ucgen Kumelerinin Hiyerarsik yapisini tutar (Ileride eklenecek).
 * * Ornegin, AABB Hiyerarsisi (AABB Tree) veya Bounding Volume Hiyerarsisi (BVH) dugumleri.
 */
typedef struct fe_gv_hierarchy {
    // fe_gv_node_t* nodes; // Hierarsi dugumleri
    fe_buffer_id_t hierarchy_ssbo; // Dugumleri tutan GPU tamponu
    uint32_t node_count;
} fe_gv_hierarchy_t;

/**
 * @brief GeometryV Render sisteminin tüm kaynaklarini ve durumunu tutar.
 */
typedef struct fe_gv_scene {
    // Geometri Kaynaklari (Sahnenin Tum Verileri)
    fe_buffer_id_t triangle_ssbo; // Sahnedeki tüm ucgenlerin verisini tutan tampon
    fe_buffer_id_t cluster_ssbo;  // fe_gv_cluster_t yapilarini tutan tampon
    uint32_t total_triangle_count;
    uint32_t cluster_count;
    
    // Hiyerarsi Yöneticisi
    fe_gv_hierarchy_t hierarchy;
    
    // Kamera Matrisleri
    fe_mat4_t view_matrix;
    fe_mat4_t projection_matrix;

    // TODO: fe_gv_trace_context_t* tracing_ctx; // Ray Tracing modulu eklenecek
} fe_gv_scene_t;


// ----------------------------------------------------------------------
// 2. YÖNETİM FONKSİYONLARI
// ----------------------------------------------------------------------

/**
 * @brief GeometryV Scene yapisini baslatir, SSBO'lari ayirir.
 * @return Yeni olusturulan fe_gv_scene_t pointer'i. Basarisiz olursa NULL.
 */
fe_gv_scene_t* fe_gv_scene_init(void);

/**
 * @brief GeometryV Scene'i kapatir ve tüm GPU/CPU kaynaklarini serbest birakir.
 */
void fe_gv_scene_shutdown(fe_gv_scene_t* scene);

/**
 * @brief Sahne geometrisini GeometryV yapisina yukler ve kümelere ayirir (Clustering).
 * * @param meshes Sahneden alinan tüm mesh'lerin listesi.
 * @param mesh_count Mesh sayisi.
 */
void fe_gv_scene_load_geometry(fe_gv_scene_t* scene, 
                                const fe_mesh_t* const* meshes, 
                                uint32_t mesh_count);

/**
 * @brief Cluster verilerini kullanarak hiyerarsi yapisini (BVH/AABB Tree) insa eder.
 * * Bu, isin takibini hizlandirmak icin gereklidir.
 */
void fe_gv_scene_build_hierarchy(fe_gv_scene_t* scene);

/**
 * @brief Her karede kamera ve diger uniform verilerini gunceller.
 */
void fe_gv_scene_update(fe_gv_scene_t* scene, 
                        const fe_mat4_t* view, 
                        const fe_mat4_t* proj);

#endif // FE_GV_SCENE_H