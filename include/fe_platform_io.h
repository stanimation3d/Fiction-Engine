// include/platform/fe_platform_io.h

#ifndef FE_PLATFORM_IO_H
#define FE_PLATFORM_IO_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "error/fe_error.h" // Hata kodları için

// ----------------------------------------------------------------------
// PLATFORM SEÇİMİ VE UYGUN BAŞLIK DOSYASINI DAHİL ETME
// ----------------------------------------------------------------------

#if defined(__unix__) || defined(__APPLE__)
    // Unix benzeri sistemler (Linux, macOS) için
    #include "platform/fe_unix_io.h"
    
    // Motorun genel kullanacağı isimleri platforma özgü isimlere yönlendir
    #define fe_file_handle_t        fe_file_handle_t        // Tip adını koru
    #define FE_INVALID_HANDLE       FE_INVALID_HANDLE       // Sabit adını koru
    #define fe_file_mode_t          fe_file_mode_t          // Enum adını koru

    // Motorun Kullanacağı Fonksiyon İsimleri
    #define fe_open         fe_unix_open
    #define fe_read         fe_unix_read
    #define fe_write        fe_unix_write
    #define fe_get_size     fe_unix_get_size
    #define fe_close        fe_unix_close
    
#elif defined(_WIN32)
    // Windows için
    #include "platform/fe_windows_io.h"

    // Motorun genel kullanacağı isimleri platforma özgü isimlere yönlendir
    #define fe_file_handle_t        fe_file_handle_t        // Tip adını koru
    #define FE_INVALID_HANDLE       FE_INVALID_HANDLE       // Sabit adını koru
    #define fe_file_mode_t          fe_file_mode_t          // Enum adını koru

    // Motorun Kullanacağı Fonksiyon İsimleri
    #define fe_open         fe_windows_open
    #define fe_read         fe_windows_read
    #define fe_write        fe_windows_write
    #define fe_get_size     fe_windows_get_size
    #define fe_close        fe_windows_close
    
#else
    #error "Desteklenmeyen Isletim Sistemi: Fiction Engine sadece Unix ve Windows platformlarini destekler."
#endif

// ----------------------------------------------------------------------
// GENEL ARABİRİM PROTOTİPLERİ
// ----------------------------------------------------------------------

/**
 * @brief Belirtilen yoldaki dosyayı açar.
 * * Motorun geri kalanı tarafından kullanılan genel I/O arayüzüdür.
 * @param path Açılacak dosyanın yolu.
 * @param mode Dosya açma modu.
 * @return Başarılıysa geçerli bir dosya tanıtıcısı, aksi takdirde FE_INVALID_HANDLE.
 */
fe_file_handle_t fe_open(const char* path, fe_file_mode_t mode);

/**
 * @brief Açık bir dosyadan veri okur.
 */
int64_t fe_read(fe_file_handle_t handle, void* buffer, size_t size);

/**
 * @brief Açık bir dosyaya veri yazar.
 */
int64_t fe_write(fe_file_handle_t handle, const void* buffer, size_t size);

/**
 * @brief Açık bir dosyanın boyutunu döndürür.
 */
int64_t fe_get_size(fe_file_handle_t handle);

/**
 * @brief Açık bir dosyayı kapatır.
 */
fe_error_code_t fe_close(fe_file_handle_t handle);

#endif // FE_PLATFORM_IO_H