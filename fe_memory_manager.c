// src/memory/fe_memory_manager.c

#include "memory/fe_memory_manager.h"
#include "utils/fe_logger.h"
#include <stdlib.h> // malloc/free için
#include <string.h> // memset için

// Tekil (Singleton) Bellek Yöneticisi değişkeninin tanımı
fe_memory_manager_t g_fe_memory_manager = {0};

/**
 * Uygulama: fe_memory_manager_init
 * Bellek yöneticisini başlatır.
 */
fe_error_code_t fe_memory_manager_init(void) {
    fe_error_code_t err = FE_OK;
    size_t total_required_size = FE_MEMORY_SIZE_GENERAL + FE_MEMORY_SIZE_GRAPHICS_DATA + FE_MEMORY_SIZE_EDITOR_DATA;
    
    FE_LOG_INFO("Bellek Yonetimi baslatiliyor. Toplam tahsis edilecek sanal bellek: %.2f MB", 
                (double)total_required_size / (1024.0 * 1024.0));
    
    // 1. Ana Bellek Bloğunu Tahsis Et (Ana "Arena")
    g_fe_memory_manager.main_memory_block = (uint8_t*)malloc(total_required_size);
    if (g_fe_memory_manager.main_memory_block == NULL) {
        FE_LOG_FATAL("Ana bellek blogu tahsis edilemedi. %zu Byte.", total_required_size);
        return FE_ERR_MEMORY_ALLOCATION;
    }
    g_fe_memory_manager.main_memory_size = total_required_size;
    memset(g_fe_memory_manager.main_memory_block, 0, total_required_size); // Belleği sıfırla

    // 2. Havuzları Başlat (Statik olarak ana bloktan parçaları kullan)
    // Her havuzun başlangıç pointer'ını ve boyutunu hesapla
    uint8_t* current_offset = g_fe_memory_manager.main_memory_block;
    
    // --- Genel Amaçlı Havuz (Örn: 64 byte'lık nesneler için) ---
    err = fe_pool_init(&g_fe_memory_manager.general_pool, 
                       FE_MEMORY_SIZE_GENERAL, 
                       64, // Örnek chunk boyutu
                       current_offset);
    if (FE_CHECK(err) != FE_OK) return err;
    current_offset += FE_MEMORY_SIZE_GENERAL;
    
    // --- Grafik Veri Havuzu (Örn: GeometryV Küme Verileri, 1024 byte) ---
    err = fe_pool_init(&g_fe_memory_manager.graphics_pool, 
                       FE_MEMORY_SIZE_GRAPHICS_DATA, 
                       1024, // Örnek chunk boyutu
                       current_offset);
    if (FE_CHECK(err) != FE_OK) return err;
    current_offset += FE_MEMORY_SIZE_GRAPHICS_DATA;
    
    // --- Editör Veri Havuzu (Örn: UI Widget'ları, 32 byte) ---
    err = fe_pool_init(&g_fe_memory_manager.editor_pool, 
                       FE_MEMORY_SIZE_EDITOR_DATA, 
                       32, // Örnek chunk boyutu
                       current_offset);
    if (FE_CHECK(err) != FE_OK) return err;
    // current_offset burada artık gerekmiyor

    FE_LOG_INFO("Bellek Yonetimi ve Tum Havuzlar Basariyla Baslatildi.");
    return FE_OK;
}

/**
 * Uygulama: fe_memory_manager_shutdown
 * Tahsis edilen ana bellek bloğunu serbest bırakır.
 */
void fe_memory_manager_shutdown(void) {
    if (g_fe_memory_manager.main_memory_block != NULL) {
        // Havuzlar statik olarak bu blok içinde tahsis edildiği için, 
        // havuzları tek tek destroy etmeye gerek yoktur (free(main_block) yeterlidir),
        // ancak referans sayımı nedeniyle bellek sızıntısı uyarısı almamak için
        // havuzları kontrol edebiliriz (fe_pool_destroy sadece dinamik olanı serbest bırakır).
        
        // Statik havuzları kontrol et.
        // fe_pool_destroy(ve her havuz)

        free(g_fe_memory_manager.main_memory_block);
        g_fe_memory_manager.main_memory_block = NULL;
        g_fe_memory_manager.main_memory_size = 0;
        
        FE_LOG_INFO("Fiction Engine ana bellek blogu temizlendi ve serbest birakildi.");
    }
}

/**
 * Uygulama: fe_mem_allocate_from_pool
 * Verilen havuzdan Ownership Pointer tahsis etmeyi kolaylaştıran genel fonksiyon.
 */
fe_owned_ptr_t fe_mem_allocate_from_pool(fe_allocator_pool_t* pool) {
    // Doğrudan fe_pool_allocate fonksiyonunu çağırır.
    return fe_pool_allocate(pool);
}