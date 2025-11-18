// include/platform/fe_thread.h

#ifndef FE_THREAD_H
#define FE_THREAD_H

#include <stddef.h>
#include <stdint.h>
#include "error/fe_error.h"

// Platforma özgü başlıkları ve tipleri dahil et
#ifdef _WIN32
    #include <windows.h>
    typedef HANDLE fe_thread_handle_t;
    typedef HANDLE fe_mutex_t;
    typedef CONDITION_VARIABLE fe_cond_t;
#else // Unix / POSIX
    #include <pthread.h>
    typedef pthread_t fe_thread_handle_t;
    typedef pthread_mutex_t fe_mutex_t;
    typedef pthread_cond_t fe_cond_t;
#endif

// İş parçacığı argümanlarını ve geri dönüş değerini standartlaştırmak için
typedef void* (*fe_thread_func_t)(void* arg);


// ----------------------------------------------------------------------
// 1. İŞ PARÇACIĞI YÖNETİMİ (THREAD MANAGEMENT)
// ----------------------------------------------------------------------

/**
 * @brief Bir iş parçacığını temsil eden yapı.
 */
typedef struct fe_thread {
    fe_thread_handle_t handle;
    fe_thread_func_t start_routine;
    void* arg;
    bool is_running;
} fe_thread_t;

/**
 * @brief Yeni bir iş parçacığı oluşturur ve başlatır.
 * * @param thread Başlatılacak fe_thread_t yapısının pointer'ı.
 * @param start_routine İş parçacığının çalıştıracağı fonksiyon.
 * @param arg Fonksiyona iletilecek argüman.
 * @return Başarı durumunda FE_OK.
 */
fe_error_code_t fe_thread_create(fe_thread_t* thread, fe_thread_func_t start_routine, void* arg);

/**
 * @brief İş parçacığının tamamlanmasını bekler ve kaynaklarını serbest bırakır (join).
 * * @param thread Beklenecek fe_thread_t yapısının pointer'ı.
 * @return Başarı durumunda FE_OK.
 */
fe_error_code_t fe_thread_join(fe_thread_t* thread);


// ----------------------------------------------------------------------
// 2. KİLİT (MUTEX) YÖNETİMİ
// ----------------------------------------------------------------------

/**
 * @brief Bir kilit (mutex) ilkelini başlatır.
 */
fe_error_code_t fe_mutex_init(fe_mutex_t* mutex);

/**
 * @brief Mutex'i kilitler (bloke edici çağrı).
 * * Bir iş parçacığı kilidi tutuyorsa, diğer iş parçacıkları bekler.
 */
fe_error_code_t fe_mutex_lock(fe_mutex_t* mutex);

/**
 * @brief Mutex'in kilidini açar.
 */
fe_error_code_t fe_mutex_unlock(fe_mutex_t* mutex);

/**
 * @brief Mutex kaynağını serbest bırakır.
 */
fe_error_code_t fe_mutex_destroy(fe_mutex_t* mutex);


// ----------------------------------------------------------------------
// 3. DURUM DEĞİŞKENİ (CONDITION VARIABLE) YÖNETİMİ
// ----------------------------------------------------------------------

/**
 * @brief Bir durum değişkenini başlatır.
 */
fe_error_code_t fe_cond_init(fe_cond_t* cond);

/**
 * @brief Durum değişkeni üzerinde beklemeye başlar.
 * * Atomik olarak mutex'in kilidini açar ve iş parçacığını bloke eder.
 * @param cond Beklenecek durum değişkeni.
 * @param mutex Beklerken serbest bırakılacak/kilitlenecek mutex.
 */
fe_error_code_t fe_cond_wait(fe_cond_t* cond, fe_mutex_t* mutex);

/**
 * @brief Bekleyen iş parçacıklarından birini uyandırır.
 */
fe_error_code_t fe_cond_signal(fe_cond_t* cond);

/**
 * @brief Bekleyen tüm iş parçacıklarını uyandırır.
 */
fe_error_code_t fe_cond_broadcast(fe_cond_t* cond);

/**
 * @brief Durum değişkeni kaynağını serbest bırakır.
 */
fe_error_code_t fe_cond_destroy(fe_cond_t* cond);


#endif // FE_THREAD_H