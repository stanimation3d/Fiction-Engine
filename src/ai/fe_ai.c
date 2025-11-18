// src/ai/fe_ai.c

#include "ai/fe_ai.h"
#include "utils/fe_logger.h" // Loglama
#include <stdlib.h> // malloc, free
#include <string.h> // memset
#include <math.h>

// ----------------------------------------------------------------------
// GENEL AI SİSTEM YÖNETİMİ
// ----------------------------------------------------------------------

// Basit bir global NavMesh nesnesini tutar
static fe_navmesh_t* g_global_navmesh = NULL;

/**
 * @brief AI sistemini baslatir (NavMesh vb. yükler).
 * @return Başarılıysa true, degilse false.
 */
bool fe_ai_init(void) {
    // NavMesh'in global olarak tahsis edilmesi (Simülasyon)
    g_global_navmesh = (fe_navmesh_t*)malloc(sizeof(fe_navmesh_t));
    if (!g_global_navmesh) {
        FE_LOG_ERROR("AI Sistemi baslatilamadi: Global NavMesh bellek hatasi.");
        return false;
    }
    
    // NavMesh'i başlat (fe_array_t yapılarının başlatılması gerekir)
    // g_global_navmesh->polygons = fe_array_create(sizeof(fe_navmesh_poly_t));
    
    FE_LOG_INFO("AI Sistemi baslatildi: Behavior Trees, NavMesh, Perception, EQS hazir.");
    return true;
}

/**
 * @brief AI sistemini kapatir ve kaynaklari serbest birakir.
 */
void fe_ai_shutdown(void) {
    if (g_global_navmesh) {
        // NavMesh'in serbest bırakılması
        // fe_array_destroy(g_global_navmesh->polygons);
        free(g_global_navmesh);
        g_global_navmesh = NULL;
    }
    FE_LOG_INFO("AI Sistemi kapatildi.");
}

/**
 * @brief Tüm AI bileşenlerini ve sistemlerini günceller.
 * @param dt Geçen zaman (delta time).
 */
void fe_ai_update(float dt) {
    // 1. Perception System'i güncelle: Yeni uyaranlari topla
    // 2. Behavior Trees'i güncelle: Her AI Component için BT'yi çalıştır.
    // 3. Navigasyon ve hareket sistemini güncelle.
    
    // ... Implementasyon burada yer alacak
}

// ----------------------------------------------------------------------
// NAVMESH UYGULAMALARI (Yol Bulma - Pathfinding)
// ----------------------------------------------------------------------

/**
 * Uygulama: fe_navmesh_find_path
 */
bool fe_navmesh_find_path(fe_vec3_t start_pos, fe_vec3_t end_pos, fe_array_t* path) {
    // Gerçek implementasyon A* veya HPA* algoritmasini kullanır.
    
    if (g_global_navmesh == NULL) {
        FE_LOG_WARN("NavMesh yüklenmedi. Yol bulunamadi.");
        return false;
    }

    // Basit yer tutucu (dummy) yol: Başlangıç ve bitiş noktalarını ekle
    // fe_array_push(path, &start_pos);
    // fe_array_push(path, &end_pos);
    
    FE_LOG_DEBUG("Navigasyon: Yol arama %s -> %s.", 
                 "fe_vec3_to_string(start_pos)", "fe_vec3_to_string(end_pos)");
                 
    // Varsayalım ki her zaman başarılı
    return true; 
}


// ----------------------------------------------------------------------
// EQS UYGULAMALARI (Çevresel Sorgu)
// ----------------------------------------------------------------------

/**
 * Uygulama: fe_eqs_run_query
 */
bool fe_eqs_run_query(fe_vec3_t center_pos, const char* query_name, fe_array_t* results) {
    // Gerçek implementasyon, dünya geometrisinden sorguya uyan noktalar yaratır,
    // sonra bu noktaları skorlama kriterlerine göre puanlar.

    if (strcmp(query_name, "Find_Cover") == 0) {
        // Yer tutucu sonuç 1:
        fe_eqs_result_t res1 = { 
            .location = fe_vec3_create(center_pos.x + 5.0f, center_pos.y, center_pos.z), 
            .score = 0.85f 
        };
        fe_array_push(results, &res1);
        
        FE_LOG_DEBUG("EQS: '%s' sorgusu basarili.", query_name);
        return true;
    }
    
    FE_LOG_WARN("EQS: Tanımlanmamis sorgu: %s", query_name);
    return false;
}
