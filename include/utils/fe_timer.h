// include/utils/fe_timer.h

#ifndef FE_TIMER_H
#define FE_TIMER_H

#include <stdint.h>
#include <stdbool.h>

#ifdef _WIN32
    // Windows API'sini dahil et
    #include <windows.h>
#else
    // Unix/Linux için zamanlama kütüphanesi
    #include <time.h>
#endif

/**
 * @brief Fiction Engine Yüksek Hassasiyetli Zamanlayıcı (Timer) Yapısı.
 * Platformdan bağımsız çalışmak için farklı üyeler içerir.
 */
typedef struct fe_timer {
#ifdef _WIN32
    // Windows: Performans Sayacı başlangıç değeri
    LARGE_INTEGER start_time;
    // Windows: Performans Sayacının saniyedeki frekansı (sabit)
    LARGE_INTEGER frequency;
#else
    // Unix/Linux: Başlangıç zamanını tutmak için timespec yapısı
    struct timespec start_time;
#endif
    // Zamanlayıcının şu anda çalışıp çalışmadığını belirtir
    bool is_running;
} fe_timer_t;

/**
 * @brief Fiction Engine zamanlayıcı sistemini başlatır ve frekansını ayarlar (Gerekirse).
 * * Motorun başlangıcında bir kez çağrılmalıdır.
 * @return Başlatma başarılıysa FE_OK, aksi takdirde hata kodu.
 */
fe_error_code_t fe_timer_system_init(void);

/**
 * @brief Bir zamanlayıcıyı başlatır veya sıfırlar.
 * * @param timer Başlatılacak fe_timer_t yapısının pointer'ı.
 */
void fe_timer_start(fe_timer_t* timer);

/**
 * @brief Zamanlayıcı durdurulmuşsa, geçen süreyi saniye cinsinden döndürür.
 * * @param timer Durdurulmuş fe_timer_t yapısının pointer'ı.
 * @return Geçen süre (saniye).
 */
double fe_timer_get_elapsed_s(const fe_timer_t* timer);

#endif // FE_TIMER_H
