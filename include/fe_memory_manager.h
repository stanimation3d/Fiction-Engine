// include/memory/fe_memory_manager.h

#ifndef FE_MEMORY_MANAGER_H
#define FE_MEMORY_MANAGER_H

#include <stddef.h>
#include "error/fe_error.h"
#include "memory/fe_allocator_pool.h" // Havuz yapısını kullanacak

// Fiction Engine (FE) için gereken tahmini Toplam Bellek Miktarları (Byte)
// Bu değerler, geliştirme sırasında motorun ihtiyacına göre ayarlanabilir.
#define FE_MEMORY_SIZE_GENERAL          (128 * 1024 * 1024) // 128 MB Genel Kullanım
#define FE_MEMORY_SIZE_GRAPHICS_DATA    (512 * 1024 * 1024) // 512 MB GPU'ya Gönderilecek Veri
#define FE_MEMORY_SIZE_EDITOR_DATA      (64 * 1024 * 1024)  // 64 MB Editör UI Verisi

/**
 * @brief Fiction Engine Merkezi Bellek Yöneticisi Yapısı.
 * Motorun tüm bellek tahsis stratejilerini ve havuzlarını içerir.
 */
typedef struct fe_memory_manager {
    // Ana bellek bloğuna işaretçi (Motorun genel "Heap" alanı)
    uint8_t* main_memory_block;
    size_t main_memory_size;
    
    // Motorun temel bileşenleri için özelleşmiş havuz ayırıcılar
    fe_allocator_pool_t general_pool;       // Genel/Kısa Ömürlü Veriler için Havuz (Örn: Geçici Diziler)
    fe_allocator_pool_t graphics_pool;      // GeometryV, DynamicR Verileri için Havuz (Büyük Chunks)
    fe_allocator_pool_t editor_pool;        // Editör Arayüz Nesneleri için Havuz (Küçük Chunks)
    
    // İleride daha karmaşık tahsis ediciler (Stack/Linear Allocator vb.) buraya eklenebilir.
} fe_memory_manager_t;

// Tekil (Singleton) Bellek Yöneticisinin dışarıdan erişilebilmesi için
extern fe_memory_manager_t g_fe_memory_manager;

/**
 * @brief Bellek Yöneticisini ve tüm alt havuzları başlatır.
 * * Motorun başlangıcında bir kez çağrılmalıdır.
 * @return Başarı durumunda FE_OK, aksi takdirde hata kodu.
 */
fe_error_code_t fe_memory_manager_init(void);

/**
 * @brief Bellek Yöneticisi tarafından tahsis edilen tüm belleği (özellikle main_memory_block) temizler.
 */
void fe_memory_manager_shutdown(void);

/**
 * @brief Verilen havuzdan Ownership Pointer tahsis etmeyi kolaylaştıran genel fonksiyon.
 * * @param pool Havuz pointer'ı.
 * @return Sahip olunan pointer.
 */
fe_owned_ptr_t fe_mem_allocate_from_pool(fe_allocator_pool_t* pool);

#endif // FE_MEMORY_MANAGER_H