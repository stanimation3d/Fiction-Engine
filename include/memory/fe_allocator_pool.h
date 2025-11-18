// include/memory/fe_allocator_pool.h

#ifndef FE_ALLOCATOR_POOL_H
#define FE_ALLOCATOR_POOL_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "error/fe_error.h" // Hata kodları için

// --------------------------------------------------
// SAHİPLİK (OWNERSHIP) VE ÖMÜR (LIFETIME) DESTEĞİ
// --------------------------------------------------

/**
 * @brief Bellek bloğu meta verileri.
 * * Tahsis edilmiş her nesnenin hemen önüne yerleştirilir.
 * Bu meta veriler, Rust'taki "Owner" rolünü üstlenir.
 */
typedef struct fe_block_metadata {
    // Nesneye kaç pointer'ın sahip olduğunu (veya ödünç aldığını) takip eder.
    // 1'den fazla olursa, o anda birden fazla yerde kullanılıyor demektir (Ödünç Alma).
    uint32_t ref_count;         
    struct fe_allocator_pool* owner_pool; // Hangi havuzun bu bloğa sahip olduğunu belirtir
    // İleride daha karmaşık GC veya ömür takibi için buraya zaman damgası eklenebilir.
} fe_block_metadata_t;


/**
 * @brief Sahip Olunan Pointer (Owned Pointer) Yapısı.
 * * Bu, bellek bloğunun sahipliğini temsil eden tek yapıdır.
 * * Referans sayacını yönetmek için motorun her yerinde bu yapı kullanılmalıdır.
 */
typedef struct fe_owned_ptr {
    void* data;             // Gerçek veri (kullanıcı tarafından görülen)
    fe_block_metadata_t* metadata; // Meta veriye işaretçi
} fe_owned_ptr_t;

// --------------------------------------------------
// HAVUZ (POOL) YAPISI
// --------------------------------------------------

/**
 * @brief Tek bir serbest bellek bloğuna işaret eden yapı.
 * Serbest listeyi oluşturmak için kullanılır.
 */
typedef struct fe_free_node {
    struct fe_free_node* next;
} fe_free_node_t;

/**
 * @brief Bellek Havuzu Ayırıcısı (Pool Allocator).
 * * Hem Dinamik hem de Statik havuzları yönetir.
 */
typedef struct fe_allocator_pool {
    size_t chunk_size;         // Tahsis edilecek her bir nesnenin boyutu (Byte).
    size_t block_count;        // Havuzdaki toplam blok sayısı.
    size_t total_size;         // Havuzun toplam boyutu (Byte).
    
    uint8_t* pool_start;       // Havuz belleğinin başlangıç adresi.
    bool is_dynamic;           // true ise, havuz kendisi heap'ten tahsis edilmiştir (Dinamik).
    
    fe_free_node_t* free_list; // Kullanılmaya hazır serbest blokların listesinin başı.
    
    size_t allocated_count;    // Şu anda tahsis edilmiş blok sayısı (İzleme için).
} fe_allocator_pool_t;


/**
 * @brief Bellek havuzunu başlatır (Statik veya Dinamik).
 * * Eğer `memory_buffer` NULL ise, havuz dinami k olarak ana heap'ten tahsis edilir.
 * * Eğer `memory_buffer` NULL değilse, havuz belirtilen statik arabelleği kullanır.
 * @param pool Başlatılacak havuz pointer'ı.
 * @param buffer_size Kullanılacak arabelleğin toplam boyutu.
 * @param chunk_size Tek bir nesnenin boyutu.
 * @param memory_buffer Statik arabellek (NULL ise dinamik olur).
 * @return Başarı durumunda FE_OK.
 */
fe_error_code_t fe_pool_init(fe_allocator_pool_t* pool, size_t buffer_size, size_t chunk_size, void* memory_buffer);

/**
 * @brief Havuzdan bir bellek bloğu tahsis eder (Owned Pointer döndürür).
 * * Referans sayacı 1 olarak başlatılır.
 * @param pool Havuz pointer'ı.
 * @return Sahip olunan pointer (Owned Pointer) veya hata durumunda NULL data içeren yapı.
 */
fe_owned_ptr_t fe_pool_allocate(fe_allocator_pool_t* pool);

/**
 * @brief Sahip olunan bir pointer'ın referans sayacını artırır (Yeni bir ödünç alma veya sahiplik kopyalama).
 * @param owned_ptr Sahip olunan pointer pointer'ı.
 * @return Yeni sahibini temsil eden fe_owned_ptr_t.
 */
fe_owned_ptr_t fe_owned_ptr_clone(const fe_owned_ptr_t* owned_ptr);

/**
 * @brief Sahip olunan bir pointer'ın referans sayacını azaltır.
 * * Referans sayacı sıfıra düşerse, bellek havuzda serbest bırakılır.
 * @param owned_ptr Serbest bırakılacak (kullanımdan kaldırılacak) fe_owned_ptr_t.
 */
void fe_owned_ptr_release(fe_owned_ptr_t owned_ptr);

/**
 * @brief Dinamik olarak tahsis edilmiş havuzu temizler ve serbest bırakır.
 * * Eğer havuz statik ise bu fonksiyon hiçbir şey yapmaz.
 * @param pool Havuz pointer'ı.
 */
void fe_pool_destroy(fe_allocator_pool_t* pool);

#endif // FE_ALLOCATOR_POOL_H
