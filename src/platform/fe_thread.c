// src/platform/fe_thread.c

#include "platform/fe_thread.h"
#include "utils/fe_logger.h"

// ----------------------------------------------------------------------
// PLATFORM UYGULAMALARI (WINDOWS / UNIX)
// ----------------------------------------------------------------------

#ifdef _WIN32 // ======================= WINDOWS UYGULAMASI =======================

// Windows'ta iş parçacığı fonksiyonlarının imzası farklıdır.
// POSIX imzasına uyması için bir ara (shim) fonksiyon kullanılır.
static DWORD WINAPI fe_windows_thread_wrapper(LPVOID lpParam) {
    fe_thread_t* thread = (fe_thread_t*)lpParam;
    
    FE_LOG_DEBUG("Windows Iş Parcacigi Baslatildi.");
    // Gerçek kullanıcı fonksiyonunu çağır
    thread->start_routine(thread->arg);
    
    thread->is_running = false;
    FE_LOG_DEBUG("Windows Iş Parcacigi Sonlandi.");
    return 0;
}

// --- THREADING ---

fe_error_code_t fe_thread_create(fe_thread_t* thread, fe_thread_func_t start_routine, void* arg) {
    thread->start_routine = start_routine;
    thread->arg = arg;
    
    // Windows'ta CreateThread çağrılır
    thread->handle = CreateThread(
        NULL,                   // Güvenlik nitelikleri
        0,                      // Yığın boyutu (varsayılan)
        fe_windows_thread_wrapper, // Başlatma fonksiyonu (wrapper)
        thread,                 // Wrapper'a geçirilecek argüman (thread yapısı)
        0,                      // Yürütme bayrakları (0 = hemen çalıştır)
        NULL                    // İş parçacığı kimliği (ID)
    );

    if (thread->handle == NULL) {
        FE_LOG_ERROR("Iş parcacigi olusturulamadi (Windows Hata Kodu: %lu)", GetLastError());
        return FE_ERR_GENERAL_UNKNOWN; 
    }
    thread->is_running = true;
    return FE_OK;
}

fe_error_code_t fe_thread_join(fe_thread_t* thread) {
    // İş parçacığının bitmesini bekler
    DWORD result = WaitForSingleObject(thread->handle, INFINITE);

    if (result == WAIT_FAILED) {
        FE_LOG_ERROR("Iş parcacigi join islemi basarisiz (Windows Hata Kodu: %lu)", GetLastError());
        CloseHandle(thread->handle);
        return FE_ERR_GENERAL_UNKNOWN;
    }

    // İş parçacığı tanıtıcısını (handle) kapatır
    if (CloseHandle(thread->handle) == 0) {
        FE_LOG_WARN("Iş parcacigi handle kapatilirken hata olustu.");
    }

    return FE_OK;
}


// --- MUTEX ---

fe_error_code_t fe_mutex_init(fe_mutex_t* mutex) {
    // Windows'ta Mutex için CRITICAL_SECTION kullanılır (daha hızlıdır)
    // Mutex = CRITICAL_SECTION tipine eşittir (bu durumda struct tanımlaması için HANDLE/CRITICAL_SECTION'ın kendisi tanımlı tip olmalı)
    // Ancak CONDITION_VARIABLE kullanıldığı için modern senkronizasyon araçlarını kullanırız:
    *mutex = CreateMutex(NULL, FALSE, NULL);
    if (*mutex == NULL) {
        FE_LOG_ERROR("Mutex olusturulamadi.");
        return FE_ERR_GENERAL_UNKNOWN;
    }
    return FE_OK;
}

fe_error_code_t fe_mutex_lock(fe_mutex_t* mutex) {
    if (WaitForSingleObject(*mutex, INFINITE) == WAIT_FAILED) {
        return FE_ERR_GENERAL_UNKNOWN;
    }
    return FE_OK;
}

fe_error_code_t fe_mutex_unlock(fe_mutex_t* mutex) {
    if (ReleaseMutex(*mutex) == 0) {
        return FE_ERR_GENERAL_UNKNOWN;
    }
    return FE_OK;
}

fe_error_code_t fe_mutex_destroy(fe_mutex_t* mutex) {
    if (CloseHandle(*mutex) == 0) {
         return FE_ERR_GENERAL_UNKNOWN;
    }
    return FE_OK;
}


// --- COND VAR ---

fe_error_code_t fe_cond_init(fe_cond_t* cond) {
    InitializeConditionVariable(cond);
    // Windows condition variable initialization cannot fail in modern usage.
    return FE_OK;
}

fe_error_code_t fe_cond_wait(fe_cond_t* cond, fe_mutex_t* mutex) {
    // Windows condition variable, Mutex yerine CRITICAL_SECTION veya SLIM_READ_WRITE (SRWLock) gerektirir.
    // Başlık dosyasında fe_mutex_t'yi HANDLE (Mutex) olarak tanımladık, bu yüzden bu işlevsellikte kısıtlı kalırız
    // ve daha karmaşık bir yapıya geçmemiz gerekir. Basitleştirme için:
    // Pthread semantiklerini korumak için, Mutex'in bir CRITICAL_SECTION veya SRWLock olması gerekirdi.
    
    // Basitleştirilmiş (ve muhtemelen hatalı) Pthread benzetimi:
    // Eğer fe_mutex_t'yi Windows'ta CRITICAL_SECTION yapsaydık, burası şöyle olurdu:
    // SleepConditionVariableCS(cond, (PCRITICAL_SECTION)mutex, INFINITE);
    
    // Şimdilik Mutex tipini HANDLE (CreateMutex) olarak bıraktığımız için (fe_mutex_t tanımına bakınız), 
    // Mutex yerine Lock'un kendisini kullanırız. Ancak bu Win32'de tam olarak Pthread Cond Var semantiği değildir.
    // Bunu Basit bir "Not Supported" hatası olarak ele alalım veya daha karmaşık bir yapıya (SRWLock) geçelim.
    FE_LOG_FATAL("Windows'ta fe_cond_wait, Mutex tanimlamasindan dolayi su an desteklenmiyor.");
    return FE_ERR_GENERAL_UNKNOWN;
}

fe_error_code_t fe_cond_signal(fe_cond_t* cond) {
    WakeConditionVariable(cond);
    return FE_OK;
}

fe_error_code_t fe_cond_broadcast(fe_cond_t* cond) {
    WakeAllConditionVariable(cond);
    return FE_OK;
}

fe_error_code_t fe_cond_destroy(fe_cond_t* cond) {
    // Windows'ta condition variable'lar destroy gerektirmez.
    return FE_OK;
}


#else // ======================= UNIX / POSIX UYGULAMASI =======================

// --- THREADING ---

fe_error_code_t fe_thread_create(fe_thread_t* thread, fe_thread_func_t start_routine, void* arg) {
    thread->start_routine = start_routine;
    thread->arg = arg;
    
    // Pthreads'te pthread_create çağrılır
    if (pthread_create(&(thread->handle), NULL, start_routine, arg) != 0) {
        FE_LOG_ERROR("Iş parcacigi olusturulamadi (pthread_create hatasi).");
        return FE_ERR_GENERAL_UNKNOWN;
    }
    thread->is_running = true;
    return FE_OK;
}

fe_error_code_t fe_thread_join(fe_thread_t* thread) {
    if (pthread_join(thread->handle, NULL) != 0) {
        FE_LOG_ERROR("Iş parcacigi join islemi basarisiz.");
        return FE_ERR_GENERAL_UNKNOWN;
    }
    return FE_OK;
}


// --- MUTEX ---

fe_error_code_t fe_mutex_init(fe_mutex_t* mutex) {
    // Pthreads'te normal bir Mutex başlatılır (recursif veya hata kontrolü yapılmaz)
    if (pthread_mutex_init(mutex, NULL) != 0) {
        return FE_ERR_GENERAL_UNKNOWN;
    }
    return FE_OK;
}

fe_error_code_t fe_mutex_lock(fe_mutex_t* mutex) {
    if (pthread_mutex_lock(mutex) != 0) {
        return FE_ERR_GENERAL_UNKNOWN;
    }
    return FE_OK;
}

fe_error_code_t fe_mutex_unlock(fe_mutex_t* mutex) {
    if (pthread_mutex_unlock(mutex) != 0) {
        return FE_ERR_GENERAL_UNKNOWN;
    }
    return FE_OK;
}

fe_error_code_t fe_mutex_destroy(fe_mutex_t* mutex) {
    if (pthread_mutex_destroy(mutex) != 0) {
        return FE_ERR_GENERAL_UNKNOWN;
    }
    return FE_OK;
}


// --- COND VAR ---

fe_error_code_t fe_cond_init(fe_cond_t* cond) {
    if (pthread_cond_init(cond, NULL) != 0) {
        return FE_ERR_GENERAL_UNKNOWN;
    }
    return FE_OK;
}

fe_error_code_t fe_cond_wait(fe_cond_t* cond, fe_mutex_t* mutex) {
    if (pthread_cond_wait(cond, mutex) != 0) {
        return FE_ERR_GENERAL_UNKNOWN;
    }
    return FE_OK;
}

fe_error_code_t fe_cond_signal(fe_cond_t* cond) {
    if (pthread_cond_signal(cond) != 0) {
        return FE_ERR_GENERAL_UNKNOWN;
    }
    return FE_OK;
}

fe_error_code_t fe_cond_broadcast(fe_cond_t* cond) {
    if (pthread_cond_broadcast(cond) != 0) {
        return FE_ERR_GENERAL_UNKNOWN;
    }
    return FE_OK;
}

fe_error_code_t fe_cond_destroy(fe_cond_t* cond) {
    if (pthread_cond_destroy(cond) != 0) {
        return FE_ERR_GENERAL_UNKNOWN;
    }
    return FE_OK;
}

#endif // _WIN32 / Unix