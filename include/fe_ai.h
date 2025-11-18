// include/ai/fe_ai.h

#ifndef FE_AI_H
#define FE_AI_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "data_structures/fe_array.h" // Bileşenleri ve düğümleri yönetmek için
#include "math/fe_vector.h"       // NavMesh ve Perception için

// ----------------------------------------------------------------------
// 1. GENEL AI MİMARİSİ VE KONTROL (fe_ai_component)
// ----------------------------------------------------------------------

/**
 * @brief Oyun dünyasında bir varlığı (Entity) kontrol eden ana Yapay Zeka bileşeni.
 * * Karakterin hangi BT'yi kullandığını, hedef ve durumunu tutar.
 */
typedef struct fe_ai_component {
    uint32_t entity_id;             // Kontrol edilen varlığın (örn. karakter) ID'si
    // fe_ai_behavior_tree_t* active_bt; // Aktif Davranış Ağacına işaretçi (İleride tanımlanacak)
    fe_vec3_t target_location;      // Güncel navigasyon hedefi
    float move_speed;               // Hareket hızı
    // ... Diğer durum değişkenleri (örn: is_alert, current_state)
} fe_ai_component_t;


// ----------------------------------------------------------------------
// 2. BEHAVIOR TREES (Davranış Ağaçları)
// ----------------------------------------------------------------------

// BT düğüm tipleri
typedef enum fe_bt_node_type {
    FE_BT_NODE_SEQUENCE,    // Sırayla dener, biri başarısız olursa durur.
    FE_BT_NODE_SELECTOR,    // Sırayla dener, biri başarılı olursa durur.
    FE_BT_NODE_TASK,        // Yapılacak aksiyon (örn: yürü, saldır)
    FE_BT_NODE_DECORATOR    // Koşul kontrolü (örn: hedef yakınsa)
} fe_bt_node_type_t;

/**
 * @brief Davranış Ağacının tek bir düğümünü temsil eder.
 */
typedef struct fe_bt_node {
    fe_bt_node_type_t type;
    // fe_array_t children;    // Alt düğümler (fe_bt_node_t* içerir)
    // ... İleride ek aksiyon/koşul verileri eklenecek
} fe_bt_node_t;

// ----------------------------------------------------------------------
// 3. AI NAVMESH (Navigasyon Ağı)
// ----------------------------------------------------------------------

/**
 * @brief NavMesh'i olusturan bir yüzey (polygon/mesh) parçasını temsil eder.
 * * Gerçek hayatta çok daha karmaşıktır (AABB ağaçları, bağlantı listeleri içerir).
 */
typedef struct fe_navmesh_poly {
    // fe_array_t vertices;  // Köşe noktaları (fe_vec3_t)
    // fe_array_t neighbors; // Komşu polygon ID'leri
    // ...
} fe_navmesh_poly_t;

/**
 * @brief En kısa yol bulma sisteminin temel arayüzü
 */
typedef struct fe_navmesh {
    fe_array_t polygons;    // fe_navmesh_poly_t'leri tutar
    // ... Diğer yapı verileri
} fe_navmesh_t;

/**
 * @brief Bir başlangıç noktasından hedefe giden yolu hesaplar.
 * @param start_pos Başlangıç pozisyonu.
 * @param end_pos Bitiş pozisyonu.
 * @param path Çıktı parametresi: fe_vec3_t'lerden oluşan yolu tutan fe_array_t.
 * @return Başarılıysa true, degilse false.
 */
bool fe_navmesh_find_path(fe_vec3_t start_pos, fe_vec3_t end_pos, fe_array_t* path);


// ----------------------------------------------------------------------
// 4. AI PERCEPTION SYSTEM (Algılama Sistemi)
// ----------------------------------------------------------------------

/**
 * @brief Bir AI'nin algilayabileceği uyaran (stimulus) türleri.
 */
typedef enum fe_stimulus_type {
    FE_STIMULUS_SIGHT,      // Görme
    FE_STIMULUS_HEARING,    // Duyma
    FE_STIMULUS_TOUCH,      // Temas
    // ... Diğer duyular
} fe_stimulus_type_t;

/**
 * @brief Algılama sistemine girilen tek bir uyaran.
 */
typedef struct fe_stimulus {
    fe_stimulus_type_t type;
    uint32_t source_entity_id;  // Uyaranin kaynagi
    fe_vec3_t location;         // Uyaranin pozisyonu
    float strength;             // Uyaranin siddeti (örn: ses yüksekliği)
    float expiration_time;      // Uyaranin geçerliliğini yitireceği zaman
} fe_stimulus_t;

/**
 * @brief Bir AI için algılanan tüm güncel uyaranların kaydını tutar.
 */
typedef struct fe_perception_data {
    fe_array_t sensed_stimuli; // fe_stimulus_t'leri tutar
    // ... Diğer algılanan durumlar
} fe_perception_data_t;

// ----------------------------------------------------------------------
// 5. ENVIRONMENT QUERY SYSTEM (EQS)
// ----------------------------------------------------------------------

/**
 * @brief EQS'de bir sorgu sonucunu temsil eden tek bir nokta.
 */
typedef struct fe_eqs_result {
    fe_vec3_t location;
    float score;        // Sorgu kriterlerine göre puan
} fe_eqs_result_t;

/**
 * @brief Çevresel sorgu sistemini çalıştırır.
 * * Örn: "Hedefin 10 metre yakınında düşmandan gizlenebilecek en iyi 5 yer neresidir?"
 * @param center_pos Sorgunun merkez noktası.
 * @param query_name Yürütülecek sorgunun adı (İleride dosya tabanlı yüklenecek).
 * @param results Çıktı: fe_eqs_result_t'leri tutan fe_array_t.
 * @return Başarılıysa true, degilse false.
 */
bool fe_eqs_run_query(fe_vec3_t center_pos, const char* query_name, fe_array_t* results);

#endif // FE_AI_H