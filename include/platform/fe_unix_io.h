// include/platform/fe_unix_io.h

#ifndef FE_UNIX_IO_H
#define FE_UNIX_IO_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "error/fe_error.h"

// Fiction Engine Dosya Tanımlayıcısı (File Descriptor) için bir takma ad
// Unix'te bu genellikle bir int'tir.
typedef int fe_file_handle_t;

// Dosya işlemi sırasında başarısızlık durumunu belirtmek için kullanılan sabit
#define FE_INVALID_HANDLE (-1)

/**
 * @brief Dosya açma modları.
 * * Motorun ihtiyaç duyduğu temel modları içerir.
 */
typedef enum fe_file_mode {
    FE_FILE_MODE_READ,          // Sadece okuma (Read Only)
    FE_FILE_MODE_WRITE,         // Dosya yoksa oluştur, varsa içeriğini sil ve yaz (Write Only)
    FE_FILE_MODE_READ_WRITE,    // Hem oku hem yaz
    FE_FILE_MODE_APPEND         // Dosyanın sonuna ekle
} fe_file_mode_t;


/**
 * @brief Belirtilen yoldaki dosyayı açar.
 * * @param path Açılacak dosyanın yolu.
 * @param mode Dosya açma modu (FE_FILE_MODE_READ, vb.).
 * @return Başarılıysa geçerli bir dosya tanıtıcısı (handle), aksi takdirde FE_INVALID_HANDLE (-1).
 */
fe_file_handle_t fe_unix_open(const char* path, fe_file_mode_t mode);

/**
 * @brief Açık bir dosyadan veri okur.
 * * @param handle Dosya tanıtıcısı.
 * @param buffer Okunan verinin yazılacağı tampon (buffer).
 * @param size Okunacak maksimum byte sayısı.
 * @return Başarılıysa okunan gerçek byte sayısı, aksi takdirde -1.
 */
int64_t fe_unix_read(fe_file_handle_t handle, void* buffer, size_t size);

/**
 * @brief Açık bir dosyaya veri yazar.
 * * @param handle Dosya tanıtıcısı.
 * @param buffer Yazılacak veriyi içeren tampon.
 * @param size Yazılacak byte sayısı.
 * @return Başarılıysa yazılan gerçek byte sayısı, aksi takdirde -1.
 */
int64_t fe_unix_write(fe_file_handle_t handle, const void* buffer, size_t size);

/**
 * @brief Açık bir dosyanın boyutunu (byte olarak) döndürür.
 * * @param handle Dosya tanıtıcısı.
 * @return Dosyanın boyutu, hata durumunda 0 veya -1.
 */
int64_t fe_unix_get_size(fe_file_handle_t handle);

/**
 * @brief Açık bir dosyayı kapatır.
 * * @param handle Kapatılacak dosya tanıtıcısı.
 * @return Başarılıysa FE_OK, aksi takdirde hata kodu.
 */
fe_error_code_t fe_unix_close(fe_file_handle_t handle);

#endif // FE_UNIX_IO_H