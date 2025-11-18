// src/assets/fe_asset_manager.c

#include "assets/fe_asset_manager.h"
#include "utils/fe_logger.h"
#include "math/fe_hash.h" // fe_hash_data fonksiyonu (fe_hashmap.c'den alinabilir)
#include <stdlib.h> // malloc, free
#include <string.h> // strcmp, strcpy

// ----------------------------------------------------------------------
// STATİK YÖNETİCİ VERİLERİ
// ----------------------------------------------------------------------

// Path'ten ID'ye eşleme (Anahtar: char*, Deger: fe_asset_id_t)
// Bu karma tablonun fe_hashmap.h/c'de string anahtar desteği olduğu varsayılır.
static fe_hashmap_t* g_asset_path_to_id = NULL;

// ID'den Kaynak yapısına eşleme (Anahtar: fe_asset_id_t, Deger: fe_asset_t*)
static fe_hashmap_t* g_asset_cache = NULL;

// Sonraki benzersiz ID'yi takip etmek için sayaç
static uint64_t g_next_asset_id = 1;

// ----------------------------------------------------------------------
// YARDIMCI FONKSİYONLAR
// ----------------------------------------------------------------------

/**
 * @brief Dosya türüne göre gerçek yüklemeyi gerçekleştiren yer tutucu fonksiyon.
 * @param path Dosya yolu.
 * @param type Beklenen tip.
 * @param size_out Yüklenen verinin boyutu (byte).
 * @return Ham veri (void*) veya NULL.
 */
static void* fe_load_asset_data_from_disk(const char* path, fe_asset_type_t type, size_t* size_out) {
    // Gerçek implementasyonda:
    // 1. switch (type) ile dosya türüne göre farklı yükleyiciler çağrılır.
    // 2. fe_audio_sound_load(), fe_renderer_mesh_load() vb. kullanılır.
    
    // Yer Tutucu (Dummy) Yükleyici:
    FE_LOG_INFO("Diskten Kaynak Yükleniyor: %s (Type: %d)", path, type);
    
    // Basit bir 100 byte'lık veri tamponu simülasyonu
    void* dummy_data = malloc(100);
    if (dummy_data) {
        memset(dummy_data, 0, 100);
        *size_out = 100;
        return dummy_data;
    }
    return NULL;
}

/**
 * @brief Kaynak verisini (data) tipine göre serbest bırakır.
 */
static void fe_unload_asset_data(fe_asset_t* asset) {
    if (asset->data == NULL) return;
    
    // Gerçek implementasyonda:
     switch (asset->type) {
    /   case FE_ASSET_TYPE_SOUND: fe_sound_destroy_internal(asset->data); break;
    //    ...
        default: free(asset->data); break;
     }
    
    // Yer Tutucu: Sadece free
    free(asset->data);
    asset->data = NULL;
    
    FE_LOG_INFO("Kaynak Serbest Birakildi: ID %llu, Path: %s", (unsigned long long)asset->id, asset->file_path);
}

// ----------------------------------------------------------------------
// 2. YÖNETİM UYGULAMALARI
// ----------------------------------------------------------------------

/**
 * Uygulama: fe_asset_manager_init
 */
bool fe_asset_manager_init(void) {
    // String anahtar, fe_asset_id_t değer
     fe_hashmap_create(key_size: sizeof(char*), value_size: sizeof(fe_asset_id_t))
     g_asset_path_to_id = fe_hashmap_create(sizeof(char*), sizeof(fe_asset_id_t)); 
    
     fe_asset_id_t anahtar, fe_asset_t* değer
     g_asset_cache = fe_hashmap_create(sizeof(fe_asset_id_t), sizeof(fe_asset_t*));
    
    FE_LOG_INFO("Kaynak Yöneticisi baslatildi.");
    return true;
}

/**
 * Uygulama: fe_asset_manager_shutdown
 */
void fe_asset_manager_shutdown(void) {
    // Tüm yüklü kaynakları serbest bırak (g_asset_cache üzerindeki döngü)
     for (asset_t* asset : g_asset_cache) { fe_unload_asset_data(asset); free(asset); }
    
     fe_hashmap_destroy(g_asset_path_to_id);
     fe_hashmap_destroy(g_asset_cache);
    
    FE_LOG_INFO("Kaynak Yöneticisi kapatildi. Tüm kaynaklar bellekten bosaltildi.");
}

/**
 * Uygulama: fe_asset_manager_update
 */
void fe_asset_manager_update(float dt) {
    // Referans sayaci 0 olan kaynaklari bosaltma (Garbage Collection)
    // Gerçek motorlarda, her döngüde değil, belirli aralıklarla yapılır.
    
    // Iterate over g_asset_cache
     if (asset->reference_count == 0 && asset->id != FE_ASSET_INVALID_ID) {
         fe_unload_asset_data(asset);
         // Hashmap'ten ve Path-to-ID'den kaldır.
     }

}


// ----------------------------------------------------------------------
// 3. KAYNAK YÜKLEME VE ERİŞİM UYGULAMALARI
// ----------------------------------------------------------------------

/**
 * Uygulama: fe_asset_load
 */
fe_asset_id_t fe_asset_load(const char* file_path, fe_asset_type_t type) {
    if (file_path == NULL || type == FE_ASSET_TYPE_UNKNOWN) return FE_ASSET_INVALID_ID;

    // 1. Önbellekte var mı kontrol et (Path -> ID)
     fe_asset_id_t* existing_id_ptr = fe_hashmap_get(g_asset_path_to_id, &file_path);
     if (existing_id_ptr != NULL) {
    //     // Kaynak önbellekte var. Referans sayısını artır.
         fe_asset_id_t existing_id = *existing_id_ptr;
         fe_asset_aquire(existing_id); 
         return existing_id;
     }

    // 2. Kaynak önbellekte yok, diskten yükle
    size_t data_size = 0;
    void* asset_data = fe_load_asset_data_from_disk(file_path, type, &data_size);

    if (asset_data == NULL) {
        FE_LOG_ERROR("Kaynak yüklenemedi: %s", file_path);
        return FE_ASSET_INVALID_ID;
    }

    // 3. Yeni Kaynak yapısını oluştur
    fe_asset_t* new_asset = (fe_asset_t*)calloc(1, sizeof(fe_asset_t));
    if (!new_asset) {
        free(asset_data);
        return FE_ASSET_INVALID_ID;
    }
    
    // 4. Benzersiz ID ata ve Kaynak yapısını doldur
    new_asset->id = g_next_asset_id++;
    new_asset->type = type;
    new_asset->reference_count = 1; // Yükleyen ilk referans
    new_asset->data = asset_data;
    strcpy(new_asset->file_path, file_path);

    // 5. Kaynakları önbelleklere ekle
     fe_hashmap_insert(g_asset_path_to_id, &file_path, &new_asset->id);
     fe_hashmap_insert(g_asset_cache, &new_asset->id, &new_asset);
    
    FE_LOG_INFO("Yeni kaynak yüklendi: ID %llu, Path: %s", (unsigned long long)new_asset->id, file_path);
    return new_asset->id;
}

/**
 * Uygulama: fe_asset_get
 */
void* fe_asset_get(fe_asset_id_t id) {
    if (id == FE_ASSET_INVALID_ID) return NULL;
    
     fe_asset_t** asset_ptr_ptr = fe_hashmap_get(g_asset_cache, &id);
     if (asset_ptr_ptr != NULL) {
         return (*asset_ptr_ptr)->data;
     }
    
    return NULL;
}

/**
 * Uygulama: fe_asset_aquire
 */
void* fe_asset_aquire(fe_asset_id_t id) {
     fe_asset_t** asset_ptr_ptr = fe_hashmap_get(g_asset_cache, &id);
     if (asset_ptr_ptr != NULL) {
         (*asset_ptr_ptr)->reference_count++;
         return (*asset_ptr_ptr)->data;
     }
    return NULL;
}

/**
 * Uygulama: fe_asset_release
 */
void fe_asset_release(fe_asset_id_t id) {
    if (id == FE_ASSET_INVALID_ID) return;
    
     fe_asset_t** asset_ptr_ptr = fe_hashmap_get(g_asset_cache, &id);
     if (asset_ptr_ptr != NULL) {
         (*asset_ptr_ptr)->reference_count--;
        
         if ((*asset_ptr_ptr)->reference_count == 0) {
             FE_LOG_DEBUG("Kaynak ID %llu referans sayisi 0. Bir sonraki update'te bosaltilacak.", (unsigned long long)id);
             // Kaynak Yöneticisi Update döngüsünde boşaltılır.
         }
     }
}
