// src/memory/fe_allocator_pool.c

#include "memory/fe_allocator_pool.h"
#include "utils/fe_logger.h"
#include <stdlib.h> // malloc/free için
#include <string.h> // memset için

// Meta veri yapısının boyutu
#define METADATA_SIZE sizeof(fe_block_metadata_t)
// Serbest düğüm yapısının boyutu
#define FREE_NODE_SIZE sizeof(fe_free_node_t)

/**
 * @brief Kullanıcının talep ettiği chunk boyutu ile meta veri boyutu arasında
 * tahsis edilmesi gereken gerçek boyutu hesaplar.
 */
static size_t fe_pool_get_actual_chunk_size(size_t chunk_size) {
    // Tahsis edilen her blok, kullanıcı verisi + meta veriyi içermelidir.
    // Ayrıca bellek adresi hizalaması için en büyük yapının boyutuna (FREE_NODE_SIZE veya METADATA_SIZE)
    // göre hizalama da yapılabilir, ancak şimdilik sadece toplam boyut tutulur.
    size_t actual_size = chunk_size + METADATA_SIZE;
    
    // Tahsis edilmemiş durumdayken FREE_NODE_T'yi tutabilmesi için en az Free Node boyutu kadar olmalıdır.
    if (actual_size < FREE_NODE_SIZE) {
        actual_size = FREE_NODE_SIZE;
    }
    
    return actual_size;
}

/**
 * Uygulama: fe_pool_init
 * Havuz Ayırıcıyı başlatır.
 */
fe_error_code_t fe_pool_init(fe_allocator_pool_t* pool, size_t buffer_size, size_t chunk_size, void* memory_buffer) {
    if (!pool || chunk_size == 0 || buffer_size == 0 || buffer_size < fe_pool_get_actual_chunk_size(chunk_size)) {
        FE_LOG_ERROR("Gecersiz havuz baslatma parametreleri.");
        return FE_ERR_INVALID_ARGUMENT;
    }

    size_t actual_chunk_size = fe_pool_get_actual_chunk_size(chunk_size);
    pool->chunk_size = chunk_size;
    pool->total_size = buffer_size;
    pool->block_count = buffer_size / actual_chunk_size;
    
    if (pool->block_count == 0) {
        FE_LOG_ERROR("Havuz boyutu, tek bir nesneyi bile tutmaya yetmiyor.");
        return FE_ERR_INVALID_ARGUMENT;
    }

    // Bellek tahsisi (Dinamik veya Statik)
    if (memory_buffer == NULL) {
        // Dinamik Tahsis
        pool->pool_start = (uint8_t*)malloc(pool->block_count * actual_chunk_size);
        if (pool->pool_start == NULL) {
            return FE_ERR_MEMORY_ALLOCATION;
        }
        pool->is_dynamic = true;
        FE_LOG_INFO("Dinamik Havuz baslatildi. Blok Sayisi: %zu", pool->block_count);
    } else {
        // Statik Kullanım
        pool->pool_start = (uint8_t*)memory_buffer;
        pool->is_dynamic = false;
        FE_LOG_INFO("Statik Havuz baslatildi. Blok Sayisi: %zu", pool->block_count);
    }

    // 1. Serbest Listeyi (Free List) Oluşturma (Tüm blokları birbirine bağlama)
    pool->free_list = (fe_free_node_t*)pool->pool_start;
    pool->allocated_count = 0;
    
    uint8_t* current_block = pool->pool_start;
    for (size_t i = 0; i < pool->block_count - 1; ++i) {
        fe_free_node_t* current_node = (fe_free_node_t*)current_block;
        uint8_t* next_block = current_block + actual_chunk_size;
        
        current_node->next = (fe_free_node_t*)next_block;
        current_block = next_block;
    }
    // Son bloğun işaretçisi NULL olmalı
    ((fe_free_node_t*)current_block)->next = NULL;

    return FE_OK;
}

/**
 * Uygulama: fe_pool_allocate
 * Havuzdan yeni bir blok tahsis eder ve Owner Pointer döndürür.
 */
fe_owned_ptr_t fe_pool_allocate(fe_allocator_pool_t* pool) {
    fe_owned_ptr_t owned_ptr = { .data = NULL, .metadata = NULL };

    if (pool->free_list == NULL) {
        FE_LOG_WARN("Havuz dolu! Tahsis yapilamiyor.");
        return owned_ptr;
    }

    // 1. Serbest Listeden Bloğu Al
    uint8_t* raw_block = (uint8_t*)pool->free_list;
    pool->free_list = pool->free_list->next;
    pool->allocated_count++;

    // 2. Meta Veriyi Ayarla (Bloğun en başı)
    fe_block_metadata_t* metadata = (fe_block_metadata_t*)raw_block;
    metadata->ref_count = 1; // Sahiplik Başladı
    metadata->owner_pool = pool;

    // 3. Kullanıcı Veri Alanını Belirle (Meta veriden hemen sonra)
    uint8_t* user_data = raw_block + METADATA_SIZE;

    // 4. Owned Pointer yapısını doldur
    owned_ptr.data = user_data;
    owned_ptr.metadata = metadata;
    
    FE_LOG_TRACE("Tahsis edildi. Havuzdaki Tahsis Edilmis Blok: %zu", pool->allocated_count);

    return owned_ptr;
}

/**
 * Uygulama: fe_owned_ptr_clone
 * Referans sayacını artırır (Ödünç Alma veya Sahiplik Kopyalama).
 */
fe_owned_ptr_t fe_owned_ptr_clone(const fe_owned_ptr_t* owned_ptr) {
    if (!owned_ptr || !owned_ptr->data) {
        fe_owned_ptr_t null_ptr = { .data = NULL, .metadata = NULL };
        return null_ptr;
    }
    
    // Kritik İşlem: Referans Sayacını Artır
    owned_ptr->metadata->ref_count++;
    
    FE_LOG_TRACE("Ref Sayaci artirildi. Yeni Sayi: %u", owned_ptr->metadata->ref_count);

    // Yeni Sahip olunan pointer, aynı meta veriye işaret eder.
    return *owned_ptr; 
}

/**
 * Uygulama: fe_owned_ptr_release
 * Referans sayacını azaltır. Sıfır ise serbest bırakma yapılır (Sahipliği Bırakma).
 */
void fe_owned_ptr_release(fe_owned_ptr_t owned_ptr) {
    if (!owned_ptr.data || !owned_ptr.metadata) {
        return; 
    }

    // Kritik İşlem: Referans Sayacını Azalt
    owned_ptr.metadata->ref_count--;
    
    FE_LOG_TRACE("Ref Sayaci azaltildi. Yeni Sayi: %u", owned_ptr.metadata->ref_count);

    if (owned_ptr.metadata->ref_count == 0) {
        // Sahiplik sona erdi, belleği havuza geri ver.
        fe_allocator_pool_t* pool = owned_ptr.metadata->owner_pool;
        if (pool == NULL) {
            FE_LOG_FATAL("Bellek bloğunun sahibi havuzu tanimlanmamis! Kritik Hata.");
            return;
        }

        // 1. Kullanıcı veri alanından (data) geriye, raw bellek bloğuna git
        uint8_t* raw_block = (uint8_t*)owned_ptr.data - METADATA_SIZE;
        
        // 2. Bloğu serbest listenin başına ekle
        fe_free_node_t* released_node = (fe_free_node_t*)raw_block;
        released_node->next = pool->free_list;
        pool->free_list = released_node;
        
        pool->allocated_count--;
        FE_LOG_DEBUG("Bellek bloğu serbest birakildi ve havuza geri eklendi.");
    }
    // Aksi takdirde, hala ödünç alanlar (ref_count > 0) olduğundan serbest bırakılmaz.
}


/**
 * Uygulama: fe_pool_destroy
 * Dinamik olarak tahsis edilmiş havuz belleğini serbest bırakır.
 */
void fe_pool_destroy(fe_allocator_pool_t* pool) {
    if (!pool) return;
    
    if (pool->is_dynamic && pool->pool_start != NULL) {
        if (pool->allocated_count > 0) {
            FE_LOG_WARN("Dinamik Havuz yok ediliyor, ancak hala %zu adet tahsis edilmis blok var. Bellek sizintisi olabilir!", pool->allocated_count);
        }
        free(pool->pool_start);
        pool->pool_start = NULL;
        pool->free_list = NULL;
        pool->allocated_count = 0;
        FE_LOG_INFO("Dinamik Havuz basariyla yok edildi.");
    } else if (!pool->is_dynamic) {
        FE_LOG_INFO("Statik Havuz: Yok etme islemi gerekli degil.");
        // Statik hafıza (memory_buffer) dışarıdan yönetildiğinden burada free çağrılmaz.
    }
}
