// src/graphics/geometryv/fe_gv_scene.c

#include "graphics/geometryv/fe_gv_scene.h"
#include "graphics/opengl/fe_gl_device.h" // Buffer yönetimi için
#include "graphics/opengl/fe_gl_commands.h"
#include "utils/fe_logger.h"
#include <stdlib.h> // calloc, free için
#include <string.h> // memcpy için
#include <math.h> // Matematiksel fonksiyonlar için


// ----------------------------------------------------------------------
// 1. DAHİLİ YARDIMCI FONKSİYONLAR
// ----------------------------------------------------------------------

// Tahmini maksimum kaynak boyutları (Gerçek uygulamada dinamik olmalıdır)
#define MAX_TRIANGLES 2000000 
#define MAX_CLUSTERS 20000

// Basit bir Üçgen Yapısı (GPU'ya gönderilecek)
typedef struct fe_gpu_triangle {
    fe_vec3_t p1, p2, p3; // 3 Köşe Pozisyonu
    // fe_vec3_t normal; // Normal verisi
    uint32_t material_id;
} fe_gpu_triangle_t;

/**
 * @brief fe_mesh_t'deki verileri fe_gpu_triangle_t yapısına dönüştürür.
 * * NOTE: Bu, mesh'lerin CPU'ya geri okunmasını gerektirir, bu da pahalıdır.
 * * Gerçek motorlar doğrudan GPU'da dönüştürme yapar.
 */
static void fe_gv_scene_process_mesh(const fe_mesh_t* mesh, fe_gpu_triangle_t* triangle_list, uint32_t* current_triangle_count) {
    // Bu sadece bir simülasyondur. fe_mesh_t'nin nasıl göründüğüne bağlıdır.
    // Varsayalım ki her mesh'in kendi vertex/index verisi var.
    
    // Basitlik için rastgele 100 üçgen eklendiği varsayılır:
    uint32_t start_idx = *current_triangle_count;
    uint32_t num_tris = 100; // Mesh'teki gercek ucgen sayisi olmalidir

    if (start_idx + num_tris > MAX_TRIANGLES) {
        num_tris = MAX_TRIANGLES - start_idx;
        FE_LOG_WARN("MAX_TRIANGLES sinirina ulasildi. Mesh'in bir kismi atlandi.");
    }

    for (uint32_t i = 0; i < num_tris; ++i) {
        // Rastgele pozisyon atama (Sadece test için)
        triangle_list[start_idx + i].p1 = (fe_vec3_t){ (float)i, 0, 0 };
        triangle_list[start_idx + i].p2 = (fe_vec3_t){ (float)i + 1, 0, 0 };
        triangle_list[start_idx + i].p3 = (fe_vec3_t){ (float)i, 1, 0 };
        triangle_list[start_idx + i].material_id = 1;
    }
    
    *current_triangle_count += num_tris;
}


// ----------------------------------------------------------------------
// 2. ARABİRİM UYGULAMALARI
// ----------------------------------------------------------------------

/**
 * Uygulama: fe_gv_scene_init
 */
fe_gv_scene_t* fe_gv_scene_init(void) {
    FE_LOG_INFO("GeometryV Scene baslatiliyor...");
    
    fe_gv_scene_t* scene = (fe_gv_scene_t*)calloc(1, sizeof(fe_gv_scene_t));
    if (!scene) return NULL;

    // 1. GPU tamponlari icin bellek ayir
    // Tüm Üçgenlerin listesi
    scene->triangle_ssbo = fe_gl_device_create_buffer(
        sizeof(fe_gpu_triangle_t) * MAX_TRIANGLES, NULL, FE_BUFFER_USAGE_STATIC);
    
    // Küme verileri (Cluster Listesi)
    scene->cluster_ssbo = fe_gl_device_create_buffer(
        sizeof(fe_gv_cluster_t) * MAX_CLUSTERS, NULL, FE_BUFFER_USAGE_STATIC);
        
    // Hiyerarsi yapisi (Başlangıçta boş)
    scene->hierarchy.hierarchy_ssbo = fe_gl_device_create_buffer(
        sizeof(uint32_t) * MAX_CLUSTERS * 4, NULL, FE_BUFFER_USAGE_STATIC);

    if (scene->triangle_ssbo == 0 || scene->cluster_ssbo == 0 || scene->hierarchy.hierarchy_ssbo == 0) {
        FE_LOG_FATAL("GeometryV SSBO'lari olusturulamadi.");
        fe_gv_scene_shutdown(scene);
        return NULL;
    }
    
    FE_LOG_INFO("GeometryV GPU kaynaklari hazir.");
    return scene;
}

/**
 * Uygulama: fe_gv_scene_shutdown
 */
void fe_gv_scene_shutdown(fe_gv_scene_t* scene) {
    if (!scene) return;

    FE_LOG_INFO("GeometryV Scene kapatiliyor.");
    
    // GPU tamponlarini sil
    fe_gl_device_destroy_buffer(scene->triangle_ssbo);
    fe_gl_device_destroy_buffer(scene->cluster_ssbo);
    fe_gl_device_destroy_buffer(scene->hierarchy.hierarchy_ssbo);

    free(scene);
    FE_LOG_DEBUG("GeometryV Scene kapatildi.");
}

/**
 * Uygulama: fe_gv_scene_load_geometry
 */
void fe_gv_scene_load_geometry(fe_gv_scene_t* scene, 
                                const fe_mesh_t* const* meshes, 
                                uint32_t mesh_count) {
    if (!scene) return;

    FE_LOG_INFO("GeometryV geometri yukleniyor ve kumeleniyor (%u mesh)...", mesh_count);

    // 1. Tüm sahne üçgenlerini CPU'da bir araya topla
    fe_gpu_triangle_t* all_triangles = (fe_gpu_triangle_t*)calloc(MAX_TRIANGLES, sizeof(fe_gpu_triangle_t));
    if (!all_triangles) {
        FE_LOG_FATAL("Ucgen verileri icin bellek yetersiz.");
        return;
    }
    
    uint32_t current_tri_count = 0;
    for (uint32_t i = 0; i < mesh_count; ++i) {
        // Bu fonksiyon, mesh'ten üçgen verilerini çıkarır
        fe_gv_scene_process_mesh(meshes[i], all_triangles, &current_tri_count);
    }
    scene->total_triangle_count = current_tri_count;
    
    // 2. Üçgen verilerini GPU'ya yükle (Triangle SSBO'yu güncelle)
    fe_gl_device_update_buffer(scene->triangle_ssbo, all_triangles, 
                               sizeof(fe_gpu_triangle_t) * scene->total_triangle_count);

    // 3. Kümeleme (Clustering)
    // Gerçek uygulamada, bu aşama GPU'da Compute Shader ile yapılır.
    // Basitlik için, burada her 100 üçgende bir küme oluşturulduğu varsayılır.
    
    fe_gv_cluster_t* cluster_list = (fe_gv_cluster_t*)calloc(MAX_CLUSTERS, sizeof(fe_gv_cluster_t));
    uint32_t current_cluster_count = 0;
    
    uint32_t cluster_size = 100; 
    for (uint32_t i = 0; i < current_tri_count; i += cluster_size) {
        if (current_cluster_count >= MAX_CLUSTERS) break;

        cluster_list[current_cluster_count].first_triangle_idx = i;
        cluster_list[current_cluster_count].triangle_count = (i + cluster_size > current_tri_count) ? 
            (current_tri_count - i) : cluster_size;
        
        // AABB hesaplama simülasyonu
        cluster_list[current_cluster_count].aabb_min = (fe_vec3_t){-10.0f, -10.0f, -10.0f};
        cluster_list[current_cluster_count].aabb_max = (fe_vec3_t){ 10.0f,  10.0f,  10.0f};
        
        current_cluster_count++;
    }
    scene->cluster_count = current_cluster_count;

    // 4. Küme verilerini GPU'ya yükle (Cluster SSBO'yu güncelle)
    fe_gl_device_update_buffer(scene->cluster_ssbo, cluster_list, 
                               sizeof(fe_gv_cluster_t) * scene->cluster_count);

    FE_LOG_INFO("Geometri kumelendi. Toplam Ucgen: %u, Kume Sayisi: %u", 
                scene->total_triangle_count, scene->cluster_count);
    
    free(all_triangles);
    free(cluster_list);
}

/**
 * Uygulama: fe_gv_scene_build_hierarchy
 */
void fe_gv_scene_build_hierarchy(fe_gv_scene_t* scene) {
    if (!scene) return;
    
    FE_LOG_INFO("Kume Hiyerarsisi insa ediliyor...");
    
    if (scene->cluster_count == 0) {
        FE_LOG_WARN("Kumeler mevcut degil, hiyerarsi insasi atlandi.");
        return;
    }
    
    // Gerçek uygulamada: Bu, Küme SSBO'sunu girdi olarak alan bir Compute Shader tarafından yürütülür
    // ve Hiyerarşi (BVH) düğüm yapısını fe_gv_hierarchy_t->hierarchy_ssbo'ya yazar.
    
    fe_gl_cmd_bind_ssbo(scene->cluster_ssbo, 0);
    fe_gl_cmd_bind_ssbo(scene->hierarchy.hierarchy_ssbo, 1);
    fe_shader_run_compute("bvh_builder.comp", dispatch_x, dispatch_y, 1);
    
    // Simülasyon: 
    scene->hierarchy.node_count = scene->cluster_count * 2 - 1; 
    FE_LOG_DEBUG("Hiyerarsi insasi tamamlandi. Toplam Dugum Sayisi: %u", scene->hierarchy.node_count);
}

/**
 * Uygulama: fe_gv_scene_update
 */
void fe_gv_scene_update(fe_gv_scene_t* scene, 
                        const fe_mat4_t* view, 
                        const fe_mat4_t* proj) {
    if (!scene) return;

    // Kamera matrislerini guncelle
    memcpy(&scene->view_matrix, view, sizeof(fe_mat4_t));
    memcpy(&scene->projection_matrix, proj, sizeof(fe_mat4_t));

    // TODO: Matrisleri global Uniform Buffer'a (UBO) yükle

    FE_LOG_TRACE("GeometryV Scene matrisleri guncellendi.");
}