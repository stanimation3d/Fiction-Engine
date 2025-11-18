// src/platform/fe_unix_io.c

// Bu dosyanın sadece Unix sistemlerde derlenmesini sağlayan makro kontrolü
#if defined(__unix__) || defined(__APPLE__)

#include "platform/fe_unix_io.h"
#include "error/fe_error.h"
#include "utils/fe_logger.h"

// Unix/Linux/macOS sistem çağrıları için gerekli başlıklar
#include <sys/stat.h> // Dosya durum bilgisi (stat) için
#include <fcntl.h>    // Dosya kontrolü (open modları) için
#include <unistd.h>   // Genel Unix standart fonksiyonları (read, write, close) için
#include <errno.h>    // Hata numaralarını almak için

/**
 * @brief fe_file_mode_t enum değerlerini Unix'e özgü `open` bayraklarına dönüştürür.
 * * @param mode Motorun dosya açma modu.
 * @return Unix `open` sistem çağrısı için kullanılan bayraklar (O_RDONLY, O_WRONLY vb.).
 */
static int fe_unix_get_flags(fe_file_mode_t mode) {
    int flags = 0;
    
    switch (mode) {
        case FE_FILE_MODE_READ:
            flags = O_RDONLY;
            break;
        case FE_FILE_MODE_WRITE:
            // O_WRONLY: Sadece yazma
            // O_CREAT: Dosya yoksa oluştur
            // O_TRUNC: Dosya varsa içeriğini sil
            flags = O_WRONLY | O_CREAT | O_TRUNC;
            break;
        case FE_FILE_MODE_READ_WRITE:
            // O_RDWR: Hem oku hem yaz
            flags = O_RDWR;
            break;
        case FE_FILE_MODE_APPEND:
            // O_APPEND: Yazmayı dosyanın sonundan başlat
            flags = O_WRONLY | O_CREAT | O_APPEND;
            break;
        default:
            flags = 0;
            break;
    }
    return flags;
}

/**
 * Uygulama: fe_unix_open
 */
fe_file_handle_t fe_unix_open(const char* path, fe_file_mode_t mode) {
    int flags = fe_unix_get_flags(mode);
    // Yeni dosya oluşturuluyorsa (O_CREAT kullanıldıysa), izinler (mod) de sağlanmalıdır.
    // 0666: Sahip, Grup ve Diğerleri için Okuma/Yazma izni.
    mode_t permissions = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
    
    // open sistem çağrısı
    fe_file_handle_t handle = open(path, flags, permissions);

    if (handle == FE_INVALID_HANDLE) {
        FE_LOG_ERROR("Dosya acilamadi: %s (Hata Kodu: %d)", path, errno);
    } else {
        FE_LOG_DEBUG("Dosya acildi: %s (Handle: %d)", path, handle);
    }
    
    return handle;
}

/**
 * Uygulama: fe_unix_read
 */
int64_t fe_unix_read(fe_file_handle_t handle, void* buffer, size_t size) {
    if (handle == FE_INVALID_HANDLE) {
        FE_LOG_ERROR("Gecersiz dosya tanıtıcısı (handle) ile okuma girisimi.");
        return -1;
    }
    
    // read sistem çağrısı
    ssize_t bytes_read = read(handle, buffer, size);

    if (bytes_read == -1) {
        FE_LOG_ERROR("Dosya okuma hatasi (Hata Kodu: %d)", errno);
    }
    
    return (int64_t)bytes_read;
}

/**
 * Uygulama: fe_unix_write
 */
int64_t fe_unix_write(fe_file_handle_t handle, const void* buffer, size_t size) {
    if (handle == FE_INVALID_HANDLE) {
        FE_LOG_ERROR("Gecersiz dosya tanıtıcısı (handle) ile yazma girisimi.");
        return -1;
    }
    
    // write sistem çağrısı
    ssize_t bytes_written = write(handle, buffer, size);

    if (bytes_written == -1) {
        FE_LOG_ERROR("Dosya yazma hatasi (Hata Kodu: %d)", errno);
    }

    return (int64_t)bytes_written;
}

/**
 * Uygulama: fe_unix_get_size
 * Dosyanın boyutunu öğrenmek için `fstat` kullanılır.
 */
int64_t fe_unix_get_size(fe_file_handle_t handle) {
    if (handle == FE_INVALID_HANDLE) {
        FE_LOG_ERROR("Gecersiz dosya tanıtıcısı (handle) ile boyut sorgulama girisimi.");
        return -1;
    }
    
    struct stat st;
    // fstat, dosya tanıtıcısına göre dosya bilgilerini (durumunu) alır.
    if (fstat(handle, &st) == -1) {
        FE_LOG_ERROR("Dosya boyutu alinamadi (fstat hatasi, Kod: %d)", errno);
        return -1;
    }

    return (int64_t)st.st_size;
}

/**
 * Uygulama: fe_unix_close
 */
fe_error_code_t fe_unix_close(fe_file_handle_t handle) {
    if (handle == FE_INVALID_HANDLE) {
        FE_LOG_WARN("Kapatilmaya calisilan dosya zaten gecersizdi.");
        return FE_OK;
    }
    
    // close sistem çağrısı
    if (close(handle) == -1) {
        FE_LOG_ERROR("Dosya kapatma hatasi (Hata Kodu: %d)", errno);
        return FE_ERR_GENERAL_UNKNOWN; 
    }
    
    FE_LOG_DEBUG("Dosya basariyla kapatildi. (Handle: %d)", handle);
    return FE_OK;
}

#endif // __unix__ || __APPLE__