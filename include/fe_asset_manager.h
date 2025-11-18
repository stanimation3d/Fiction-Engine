// include/assets/fe_asset_manager.h

#ifndef FE_ASSET_MANAGER_H
#define FE_ASSET_MANAGER_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// fe_hashmap, fe_array gibi temel yapılar dahil edilir
#include "data_structures/fe_hashmap.h" 
#include "data_structures/fe_array.h"

// ----------------------------------------------------------------------
// 1. TEMEL TİPLER
// ----------------------------------------------------------------------

/**
 * @brief Benzersiz Kaynak Kimliği (Asset ID)
 */
typedef uint64_t fe_asset_id_t;
#define FE_ASSET_INVALID_ID 0ULL

/**
 * @brief Motor tarafından desteklenen kaynak türleri.
 * * Daha önce tanımlanan diğer modüllere karşılık gelir.
 */
typedef enum fe_asset_type {
    FE_ASSET_TYPE_UNKNOWN = 0,
    FE_ASSET_TYPE_MESH,         // Grafik/Renderer
    FE_ASSET_TYPE_TEXTURE,      // Grafik/Renderer
    FE_ASSET_TYPE_SOUND,        // Audio/fe_sound_t
    FE_ASSET_TYPE_ANIMATION,    // Animation/fe_anim_clip_t
    FE_ASSET_TYPE_BEHAVIOR_TREE,// AI/fe_behavior_tree_t
    FE_ASSET_TYPE_NODE_GRAPH,   // Editor/fe_node_graph_t (Blueprint benzeri)
    FE_ASSET_TYPE_COUNT
} fe_asset_type_t;

/**
 * @brief Yüklenen tek bir varlık (Asset) örneği.
 */
typedef struct fe_asset {
    fe_asset_id_t id;           // Kaynak Kimliği
    fe_asset_type_t type;       // Kaynak Tipi
    uint32_t reference_count;   // Kaynağı kullanan aktif nesne sayısı
    void* data;                 // Kaynağın gerçek verisine işaretçi (Örn: fe_sound_t*, fe_anim_clip_t*)
    char file_path[256];        // Kaynağın yüklendiği dosya yolu
} fe_asset_t;


// ----------------------------------------------------------------------
// 2. YÖNETİM FONKSİYONLARI
// ----------------------------------------------------------------------

/**
 * @brief Kaynak Yöneticisini baslatir ve kaynak önbelleğini hazırlar.
 * @return Başarılıysa true, degilse false.
 */
bool fe_asset_manager_init(void);

/**
 * @brief Kaynak Yöneticisini kapatir ve yüklü tüm kaynaklari serbest birakir.
 */
void fe_asset_manager_shutdown(void);

/**
 * @brief Kaynak Yöneticisini günceller (Örn: Referans sayısı sıfır olanları bosaltir).
 */
void fe_asset_manager_update(float dt);

// ----------------------------------------------------------------------
// 3. KAYNAK YÜKLEME VE ERİŞİM
// ----------------------------------------------------------------------

/**
 * @brief Dosya yolundan bir kaynak yükler veya önbellekte varsa referans sayısını artırır.
 * * Kaynak yükleme, hash map'e erişim (O(1)) ve dosya yükleme (O(N)) içerir.
 * @param file_path Yüklenecek dosyanın yolu.
 * @param type Yüklenmesi beklenen kaynak türü.
 * @return fe_asset_id_t Başarılıysa geçerli bir ID, aksi takdirde FE_ASSET_INVALID_ID.
 */
fe_asset_id_t fe_asset_load(const char* file_path, fe_asset_type_t type);

/**
 * @brief Kaynak kimliği ile önbelleğe alınmış kaynağa erişir.
 * * Bu, referans sayısını ARTIRMAZ. fe_asset_release ile eşleştirilmez.
 * @param id Aranacak kaynağın kimliği.
 * @return Kaynağın ham verisine işaretçi (void*) veya NULL.
 */
void* fe_asset_get(fe_asset_id_t id);

/**
 * @brief Kaynak kimliği ile kaynağa erişir ve referans sayısını bir artırır.
 * * fe_asset_release ile eşleştirilmelidir.
 * @param id Aranacak kaynağın kimliği.
 * @return Kaynağın ham verisine işaretçi (void*) veya NULL.
 */
void* fe_asset_aquire(fe_asset_id_t id);

/**
 * @brief Kaynağın referans sayısını bir azaltır.
 * * Referans sayısı 0'a ulaşırsa, kaynak otomatik olarak bellekten bosaltılabilir.
 * @param id Serbest bırakılacak kaynağın kimliği.
 */
void fe_asset_release(fe_asset_id_t id);

#endif // FE_ASSET_MANAGER_H