// src/data_structures/fe_hashmap.c

#include "data_structures/fe_hashmap.h"
#include "utils/fe_logger.h" 
#include <stdlib.h> 
#include <string.h> 
#include <math.h> // ceilf

// Varsayılan kova sayısı
#define FE_HASHMAP_DEFAULT_CAPACITY 16
// Yeniden boyutlandırma için maksimum yük faktörü (capacity/count)
#define FE_HASHMAP_MAX_LOAD_FACTOR 0.75f
// Yeniden boyutlandırma çarpanı
#define FE_HASHMAP_RESIZE_FACTOR 2

// ----------------------------------------------------------------------
// 1. TEMEL HASH VE YARDIMCI FONKSİYONLAR
// ----------------------------------------------------------------------

/**
 * @brief Basit bir FNV-1a tabanlı 64-bit hash fonksiyonu.
 * * Anahtarin hash degerini hesaplar.
 */
static uint64_t fe_hash_data(const void* data, size_t size) {
    if (!data || size == 0) return 0;
    
    uint64_t hash = 0xcbf29ce484222325ULL; // FNV_offset_basis
    const uint8_t* bytes = (const uint8_t*)data;

    for (size_t i = 0; i < size; ++i) {
        hash ^= bytes[i];
        hash *= 0x100000001b3ULL; // FNV_prime
    }
    return hash;
}

/**
 * @brief Bir düğüm (node) yapısını bellekten tahsis eder ve veriyi kopyalar.
 */
static fe_hashmap_node_t* fe_hashmap_create_node(const void* key_ptr, size_t key_size, 
                                                 const void* value_ptr, size_t value_size, 
                                                 uint64_t hash) 
{
    fe_hashmap_node_t* node = (fe_hashmap_node_t*)malloc(sizeof(fe_hashmap_node_t));
    if (!node) return NULL;
    
    node->hash = hash;
    node->next = NULL;
    
    // Anahtar ve Deger için bellek tahsisi
    node->key = malloc(key_size);
    node->value = malloc(value_size);
    
    if (!node->key || !node->value) {
        if (node->key) free(node->key);
        if (node->value) free(node->value);
        free(node);
        return NULL;
    }
    
    // Verileri kopyala
    memcpy(node->key, key_ptr, key_size);
    memcpy(node->value, value_ptr, value_size);

    return node;
}

/**
 * @brief Bir düğümü serbest bırakır.
 */
static void fe_hashmap_destroy_node(fe_hashmap_node_t* node) {
    if (node) {
        if (node->key) free(node->key);
        if (node->value) free(node->value);
        free(node);
    }
}

/**
 * @brief Hash Map'i yeniden boyutlandırır (Rehash).
 */
static bool fe_hashmap_resize(fe_hashmap_t* map, size_t new_capacity) {
    // Tüm kova içeriğini yeni kovaya taşı
    fe_hashmap_node_t** old_buckets = map->buckets;
    size_t old_capacity = map->capacity;
    
    // Yeni kovaları tahsis et ve sıfırla
    fe_hashmap_node_t** new_buckets = (fe_hashmap_node_t**)calloc(new_capacity, sizeof(fe_hashmap_node_t*));
    if (!new_buckets) {
        FE_LOG_ERROR("Hashmap yeniden boyutlandirma basarisiz.");
        return false;
    }
    
    map->buckets = new_buckets;
    map->capacity = new_capacity;
    map->count = 0; // Yeniden eklerken artırılacak

    // Eski kovadaki tüm elemanları yeni kovaya ekle (Rehash)
    for (size_t i = 0; i < old_capacity; ++i) {
        fe_hashmap_node_t* node = old_buckets[i];
        fe_hashmap_node_t* next_node;
        
        while (node != NULL) {
            next_node = node->next;
            
            // Elemanı yeniden ekle (Yeni kovadaki indeksi hesapla)
            // Bu basamakta sadece düğümün konumunu değiştiriyoruz, veriyi kopyalamıyoruz.
            
            // Yeni kova indeksi
            size_t new_bucket_idx = (size_t)(node->hash % map->capacity);
            
            // Zincirleme (yeni düğümü kovanın başına ekle)
            node->next = map->buckets[new_bucket_idx];
            map->buckets[new_bucket_idx] = node;
            map->count++; 

            node = next_node;
        }
    }

    free(old_buckets);
    return true;
}


// ----------------------------------------------------------------------
// 2. YÖNETİM VE İŞLEMLER UYGULAMALARI
// ----------------------------------------------------------------------

/**
 * Uygulama: fe_hashmap_create
 */
fe_hashmap_t* fe_hashmap_create(size_t key_size, size_t value_size) {
    if (key_size == 0 || value_size == 0) {
        FE_LOG_ERROR("Hashmap anahtar veya deger boyutu sifir olamaz.");
        return NULL;
    }

    fe_hashmap_t* map = (fe_hashmap_t*)calloc(1, sizeof(fe_hashmap_t));
    if (!map) return NULL;
    
    map->key_size = key_size;
    map->value_size = value_size;
    map->capacity = FE_HASHMAP_DEFAULT_CAPACITY;
    map->load_factor = FE_HASHMAP_MAX_LOAD_FACTOR;
    
    // Kova dizisi tahsis et ve sıfırla
    map->buckets = (fe_hashmap_node_t**)calloc(map->capacity, sizeof(fe_hashmap_node_t*));
    
    if (!map->buckets) {
        free(map);
        return NULL;
    }
    
    return map;
}

/**
 * Uygulama: fe_hashmap_destroy
 */
void fe_hashmap_destroy(fe_hashmap_t* map) {
    if (map) {
        if (map->buckets) {
            // Her kovayı ve zincirdeki tüm düğümleri serbest bırak
            for (size_t i = 0; i < map->capacity; ++i) {
                fe_hashmap_node_t* node = map->buckets[i];
                fe_hashmap_node_t* next;
                while (node != NULL) {
                    next = node->next;
                    fe_hashmap_destroy_node(node);
                    node = next;
                }
            }
            free(map->buckets);
        }
        free(map);
    }
}

/**
 * Uygulama: fe_hashmap_insert
 */
bool fe_hashmap_insert(fe_hashmap_t* map, const void* key_ptr, const void* value_ptr) {
    if (!map || !key_ptr || !value_ptr) return false;
    
    // 1. Hash ve Kova İndeksi Hesapla
    uint64_t hash = fe_hash_data(key_ptr, map->key_size);
    size_t bucket_idx = (size_t)(hash % map->capacity);

    // 2. Çakışma Kontrolü (Aynı anahtar var mı?)
    fe_hashmap_node_t* current = map->buckets[bucket_idx];
    while (current != NULL) {
        // Anahtarın hash değeri ve verisi eşleşiyor mu?
        if (current->hash == hash && memcmp(current->key, key_ptr, map->key_size) == 0) {
            // Anahtar bulundu: Değeri güncelle
            memcpy(current->value, value_ptr, map->value_size);
            return true;
        }
        current = current->next;
    }

    // 3. Yeni düğüm oluştur ve Kovanın başına ekle (Insert)
    fe_hashmap_node_t* new_node = fe_hashmap_create_node(key_ptr, map->key_size, value_ptr, map->value_size, hash);
    if (!new_node) return false;
    
    new_node->next = map->buckets[bucket_idx];
    map->buckets[bucket_idx] = new_node;
    map->count++;

    // 4. Yeniden Boyutlandırma Kontrolü
    if ((float)map->count / (float)map->capacity > map->load_factor) {
        fe_hashmap_resize(map, map->capacity * FE_HASHMAP_RESIZE_FACTOR);
    }
    
    return true;
}

/**
 * Uygulama: fe_hashmap_get
 */
void* fe_hashmap_get(fe_hashmap_t* map, const void* key_ptr) {
    if (!map || !key_ptr) return NULL;

    uint64_t hash = fe_hash_data(key_ptr, map->key_size);
    size_t bucket_idx = (size_t)(hash % map->capacity);

    fe_hashmap_node_t* current = map->buckets[bucket_idx];
    while (current != NULL) {
        // Hash ve anahtar verisi eşleşiyor mu?
        if (current->hash == hash && memcmp(current->key, key_ptr, map->key_size) == 0) {
            return current->value; // Değerin adresi bulundu
        }
        current = current->next;
    }
    
    return NULL; // Bulunamadı
}

/**
 * Uygulama: fe_hashmap_remove
 */
bool fe_hashmap_remove(fe_hashmap_t* map, const void* key_ptr) {
    if (!map || !key_ptr) return false;

    uint64_t hash = fe_hash_data(key_ptr, map->key_size);
    size_t bucket_idx = (size_t)(hash % map->capacity);

    fe_hashmap_node_t* current = map->buckets[bucket_idx];
    fe_hashmap_node_t* prev = NULL;

    while (current != NULL) {
        // Hash ve anahtar verisi eşleşiyor mu?
        if (current->hash == hash && memcmp(current->key, key_ptr, map->key_size) == 0) {
            // Eleman bulundu, zincirden çıkar
            if (prev == NULL) {
                // Kovanın başındaki eleman
                map->buckets[bucket_idx] = current->next;
            } else {
                // Zincirin ortasındaki/sonundaki eleman
                prev->next = current->next;
            }
            
            fe_hashmap_destroy_node(current);
            map->count--;
            return true;
        }
        
        prev = current;
        current = current->next;
    }
    
    return false; // Bulunamadı
}