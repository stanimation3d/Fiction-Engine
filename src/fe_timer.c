// src/utils/fe_timer.c

#include "utils/fe_timer.h"
#include "error/fe_error.h" // Hata yönetimi için dahil edildi
#include <stdio.h>

// Windows'da frekans sadece bir kez sorgulanmalı, global değişken olarak tutulur.
#ifdef _WIN32
    // Statik olarak tanımlanır, böylece sadece bu dosyadan erişilebilir.
    static LARGE_INTEGER fe_timer_global_frequency = {0};
#endif


/**
 * Uygulama: fe_timer_system_init
 * Zamanlayıcı sistemi için platforma özgü hazırlıkları yapar.
 */
fe_error_code_t fe_timer_system_init(void) {
#ifdef _WIN32
    // Windows'da Yüksek Çözünürlüklü Performans Sayacının frekansını sorgula.
    if (!QueryPerformanceFrequency(&fe_timer_global_frequency)) {
        // Eğer donanım sayacı desteklenmiyorsa (çok nadir), hata döndür.
        return FE_ERR_GENERAL_UNKNOWN; 
    }
#endif
    // Unix/Linux'ta ek bir başlatma gerekmez (clock_gettime her zaman mevcuttur).
    
    return FE_OK; // Başarılı
}


/**
 * Uygulama: fe_timer_start
 * Belirtilen zamanlayıcıyı mevcut zamana ayarlar.
 */
void fe_timer_start(fe_timer_t* timer) {
    if (!timer) return;

#ifdef _WIN32
    // Windows: Sayacın frekansını başlangıç yapısına kopyala
    timer->frequency = fe_timer_global_frequency;
    // Sayacın mevcut değerini al
    QueryPerformanceCounter(&(timer->start_time));
#else
    // Unix/Linux: CLOCK_MONOTONIC'i kullanmak, sistem saati değişikliklerinden etkilenmemeyi sağlar.
    clock_gettime(CLOCK_MONOTONIC, &(timer->start_time));
#endif

    timer->is_running = true;
}

/**
 * Uygulama: fe_timer_get_elapsed_s
 * Başlangıçtan bu yana geçen süreyi saniye cinsinden döndürür.
 */
double fe_timer_get_elapsed_s(const fe_timer_t* timer) {
    if (!timer || !timer->is_running) {
        // Zamanlayıcı başlatılmamışsa, geçerli olmayan bir değer döndürülür.
        return 0.0;
    }

#ifdef _WIN32
    LARGE_INTEGER end_time;
    // Sayacın bitiş değerini al
    QueryPerformanceCounter(&end_time);
    
    // Geçen sayım farkını hesapla
    LONGLONG elapsed_counts = end_time.QuadPart - timer->start_time.QuadPart;
    
    // Saniye cinsinden süreyi hesapla: (Fark / Frekans)
    // Frekansın sıfır olmadığı varsayılır (fe_timer_system_init kontrolü sayesinde).
    return (double)elapsed_counts / (double)timer->frequency.QuadPart;
#else
    struct timespec end_time;
    // Bitiş zamanını al (Aynı clock'u kullanmak önemlidir)
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    
    // Saniye cinsinden süreyi hesapla: (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1,000,000,000.0
    double elapsed_sec = (double)(end_time.tv_sec - timer->start_time.tv_sec);
    double elapsed_nsec = (double)(end_time.tv_nsec - timer->start_time.tv_nsec) / 1000000000.0;
    
    return elapsed_sec + elapsed_nsec;
#endif
}