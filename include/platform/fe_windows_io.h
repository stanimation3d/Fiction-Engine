// include/platform/fe_windows_io.h

#ifndef FE_WINDOWS_IO_H
#define FE_WINDOWS_IO_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "error/fe_error.h"

// Windows API'de dosya tanıtıcısı bir HANDLE'dır.
// Unix ile uyumluluk için burada bir takma ad kullanılır.
typedef void* fe_file_handle_t; 

// Windows'ta geçersiz tanıtıcı (handle) değeri NULL'dur (veya INVALID_HANDLE_VALUE).
#define FE_INVALID_HANDLE NULL

/**
 * @brief Dosya açma modları.
 * * fe_unix_io.h'deki enum ile aynı olmalıdır.
 */
typedef enum fe_file_mode {
    FE_FILE_MODE_READ,          // Sadece okuma (Read Only)
    FE_FILE_MODE_WRITE,         // Dosya yoksa oluştur, varsa içeriğini sil ve yaz (Write Only)
    FE_FILE_MODE_READ_WRITE,    // Hem oku hem yaz
    FE_FILE_MODE_APPEND         // Dosyanın sonuna ekle
} fe_file_mode_t;


// -------------------------------------------------------------
// Fonksiyon Prototipleri (Unix versiyonu ile aynı imzalar)
// -------------------------------------------------------------

fe_file_handle_t fe_windows_open(const char* path, fe_file_mode_t mode);
int64_t fe_windows_read(fe_file_handle_t handle, void* buffer, size_t size);
int64_t fe_windows_write(fe_file_handle_t handle, const void* buffer, size_t size);
int64_t fe_windows_get_size(fe_file_handle_t handle);
fe_error_code_t fe_windows_close(fe_file_handle_t handle);

#endif // FE_WINDOWS_IO_H