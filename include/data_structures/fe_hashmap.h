// include/data_structures/fe_hashmap.h

#ifndef FE_HASHMAP_H
#define FE_HASHMAP_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "fe_array.h" // Zincirleme için dinamik dizi kullanilabilir

// ----------------------------------------------------------------------
// 1. DÜĞÜM VE ZİNCİR YAPISI
// ----------------------------------------------------------------------

/**
 * @brief Hash Map'teki tek bir Anahtar-Deger çiftini temsil eden düğüm.
 */
typedef struct fe_hashmap_node {
    uint64_t hash;          // Anahtarın önceden hesaplanmış hash değeri
    void* key;              // Anahtar verisinin adresi
    void* value;            // Deger verisinin adresi
    // Ayrı Zincirleme için:
    struct fe_hashmap_node* next; // Aynı kovadaki bir sonraki düğüm
} fe_hashmap_node_t;


// ----------------------------------------------------------------------
// 2. HASH MAP YAPISI (fe_hashmap_t)
// ----------------------------------------------------------------------

/**
 * @brief Temel Karma Tablo yapisi.
 */
typedef struct fe_hashmap {
    fe_hashmap_node_t** buckets; // Kova (Bucket) dizisi (fe_hashmap_node_t* dizisi)
    size_t key_size;            // Anahtarın boyutu (byte)
    size_t value_size;          // Degerin boyutu (byte)
    size_t capacity;            // Kova sayisi (buckets dizisinin boyutu)
    size_t count;               // Haritadaki mevcut eleman sayısı
    float load_factor;          // Yeniden boyutlandirma için yük faktörü
} fe_hashmap_t;


// ----------------------------------------------------------------------
// 3. YÖNETİM VE İŞLEMLER
// ----------------------------------------------------------------------

/**
 * @brief Yeni bir Hash Map olusturur.
 * @param key_size Anahtarın boyutu.
 * @param value_size Degerin boyutu.
 * @return Yeni olusturulmus fe_hashmap_t* veya NULL.
 */
fe_hashmap_t* fe_hashmap_create(size_t key_size, size_t value_size);

/**
 * @brief Hash Map'i ve içindeki tüm düğümleri bellekten serbest birakir.
 */
void fe_hashmap_destroy(fe_hashmap_t* map);

/**
 * @brief Hash Map'e yeni bir Anahtar-Deger çifti ekler veya var olanı günceller.
 * @param key_ptr Anahtar verisinin adresi.
 * @param value_ptr Deger verisinin adresi.
 * @return Başarılıysa true, degilse false.
 */
bool fe_hashmap_insert(fe_hashmap_t* map, const void* key_ptr, const void* value_ptr);

/**
 * @brief Anahtara karsilik gelen degeri arar.
 * @param key_ptr Aranacak anahtarın adresi.
 * @return Degerin adresi (void*) veya NULL (bulunamazsa).
 */
void* fe_hashmap_get(fe_hashmap_t* map, const void* key_ptr);

/**
 * @brief Anahtara karsilik gelen elemanı haritadan kaldirir.
 * @param key_ptr Kaldırılacak anahtarın adresi.
 * @return Başarılıysa true, degilse false.
 */
bool fe_hashmap_remove(fe_hashmap_t* map, const void* key_ptr);

/**
 * @brief Anahtar ve deger verilerine erişmek için iteratör olusturulur.
 * * (Gelişmiş motorlarda iteratörler gereklidir, burada basit bir yapısı varsayılır.)
 */
// typedef struct fe_hashmap_iterator { ... } fe_hashmap_iterator_t;

#endif // FE_HASHMAP_H