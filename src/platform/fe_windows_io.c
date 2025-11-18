// src/platform/fe_windows_io.c

// Bu dosyanın sadece Windows sistemlerde derlenmesini sağlayan makro kontrolü
#ifdef _WIN32

#include "platform/fe_windows_io.h"
#include "error/fe_error.h"
#include "utils/fe_logger.h"

// Windows API başlıkları
#include <windows.h>
#include <strsafe.h> // Hata mesajlarını almak için

// Windows'ta INVALID_HANDLE_VALUE tip olarak HANDLE (void*) olduğu için
// NULL ile uyumluluğu sağlıyoruz.
#define WIN_INVALID_HANDLE_VALUE (fe_file_handle_t)INVALID_HANDLE_VALUE


/**
 * @brief fe_file_mode_t enum değerlerini Windows API'sine özgü bayraklara dönüştürür.
 * * @param mode Motorun dosya açma modu.
 * @param dwDesiredAccess Gerekli erişim tipi (GENERIC_READ, GENERIC_WRITE vb.).
 * @param dwCreationDisposition Oluşturma/Açma tipi (CREATE_ALWAYS, OPEN_EXISTING vb.).
 */
static void fe_windows_get_flags(fe_file_mode_t mode, DWORD* dwDesiredAccess, DWORD* dwCreationDisposition) {
    switch (mode) {
        case FE_FILE_MODE_READ:
            // Sadece oku, zaten var olmalı
            *dwDesiredAccess = GENERIC_READ;
            *dwCreationDisposition = OPEN_EXISTING;
            break;
        case FE_FILE_MODE_WRITE:
            // Sadece yaz, yoksa oluştur, varsa içeriğini sil
            *dwDesiredAccess = GENERIC_WRITE;
            *dwCreationDisposition = CREATE_ALWAYS; 
            break;
        case FE_FILE_MODE_READ_WRITE:
            // Hem oku hem yaz, yoksa oluştur
            *dwDesiredAccess = GENERIC_READ | GENERIC_WRITE;
            *dwCreationDisposition = OPEN_ALWAYS;
            break;
        case FE_FILE_MODE_APPEND:
            // Sadece yaz (APPEND için CreateFile'da özel bir bayrak yok, SetFilePointer kullanılır, 
            // ancak CREATE_ALWAYS ile açıp yazma işlemini motor tarafında sona konumlandırmak daha güvenlidir.)
            *dwDesiredAccess = GENERIC_WRITE;
            *dwCreationDisposition = OPEN_ALWAYS; // Dosya varsa aç, yoksa oluştur
            break;
        default:
            // Güvenlik amaçlı varsayılanı sadece okuma yapıyoruz.
            *dwDesiredAccess = GENERIC_READ;
            *dwCreationDisposition = OPEN_EXISTING;
            break;
    }
}

/**
 * Uygulama: fe_windows_open
 */
fe_file_handle_t fe_windows_open(const char* path, fe_file_mode_t mode) {
    DWORD dwDesiredAccess;
    DWORD dwCreationDisposition;
    
    fe_windows_get_flags(mode, &dwDesiredAccess, &dwCreationDisposition);
    
    // CreateFileA, ANSI (char*) yolunu kullanır
    fe_file_handle_t handle = (fe_file_handle_t)CreateFileA(
        path,                       // Dosya yolu
        dwDesiredAccess,            // Erişim modu (oku/yaz)
        FILE_SHARE_READ,            // Paylaşım modu (diğer işlemler okuyabilir)
        NULL,                       // Güvenlik nitelikleri
        dwCreationDisposition,      // Nasıl oluşturulacağı/açılacağı
        FILE_ATTRIBUTE_NORMAL,      // Dosya nitelikleri
        NULL                        // Şablon dosyası
    );

    if (handle == WIN_INVALID_HANDLE_VALUE) {
        DWORD error = GetLastError();
        FE_LOG_ERROR("Dosya acilamadi: %s (Windows Hata Kodu: %lu)", path, error);
        return FE_INVALID_HANDLE;
    }
    
    // APPEND modu için yazma pointer'ını dosyanın sonuna taşı
    if (mode == FE_FILE_MODE_APPEND) {
        SetFilePointer((HANDLE)handle, 0, NULL, FILE_END);
    }
    
    FE_LOG_DEBUG("Dosya acildi: %s (Handle: %p)", path, handle);
    return handle;
}

/**
 * Uygulama: fe_windows_read
 */
int64_t fe_windows_read(fe_file_handle_t handle, void* buffer, size_t size) {
    if (handle == FE_INVALID_HANDLE || handle == WIN_INVALID_HANDLE_VALUE) {
        FE_LOG_ERROR("Gecersiz dosya tanıtıcısı (handle) ile okuma girisimi.");
        return -1;
    }
    
    DWORD bytes_read = 0;
    // ReadFile fonksiyonu okunan byte sayısını bytes_read değişkenine yazar
    BOOL result = ReadFile((HANDLE)handle, buffer, (DWORD)size, &bytes_read, NULL);

    if (result == FALSE) {
        DWORD error = GetLastError();
        FE_LOG_ERROR("Dosya okuma hatasi (Windows Hata Kodu: %lu)", error);
        return -1;
    }
    
    return (int64_t)bytes_read;
}

/**
 * Uygulama: fe_windows_write
 */
int64_t fe_windows_write(fe_file_handle_t handle, const void* buffer, size_t size) {
    if (handle == FE_INVALID_HANDLE || handle == WIN_INVALID_HANDLE_VALUE) {
        FE_LOG_ERROR("Gecersiz dosya tanıtıcısı (handle) ile yazma girisimi.");
        return -1;
    }

    DWORD bytes_written = 0;
    // WriteFile fonksiyonu yazılan byte sayısını bytes_written değişkenine yazar
    BOOL result = WriteFile((HANDLE)handle, buffer, (DWORD)size, &bytes_written, NULL);

    if (result == FALSE) {
        DWORD error = GetLastError();
        FE_LOG_ERROR("Dosya yazma hatasi (Windows Hata Kodu: %lu)", error);
        return -1;
    }

    return (int64_t)bytes_written;
}

/**
 * Uygulama: fe_windows_get_size
 * Dosyanın boyutunu almak için `GetFileSizeEx` kullanılır.
 */
int64_t fe_windows_get_size(fe_file_handle_t handle) {
    if (handle == FE_INVALID_HANDLE || handle == WIN_INVALID_HANDLE_VALUE) {
        FE_LOG_ERROR("Gecersiz dosya tanıtıcısı (handle) ile boyut sorgulama girisimi.");
        return -1;
    }
    
    LARGE_INTEGER file_size;
    
    // 64-bit dosya boyutunu almak için GetFileSizeEx kullanılır
    if (GetFileSizeEx((HANDLE)handle, &file_size) == FALSE) {
        DWORD error = GetLastError();
        FE_LOG_ERROR("Dosya boyutu alinamadi (Windows Hata Kodu: %lu)", error);
        return -1;
    }

    return (int64_t)file_size.QuadPart;
}

/**
 * Uygulama: fe_windows_close
 */
fe_error_code_t fe_windows_close(fe_file_handle_t handle) {
    if (handle == FE_INVALID_HANDLE || handle == WIN_INVALID_HANDLE_VALUE) {
        FE_LOG_WARN("Kapatilmaya calisilan dosya zaten gecersizdi.");
        return FE_OK;
    }
    
    // CloseHandle sistem çağrısı
    if (CloseHandle((HANDLE)handle) == FALSE) {
        DWORD error = GetLastError();
        FE_LOG_ERROR("Dosya kapatma hatasi (Windows Hata Kodu: %lu)", error);
        return FE_ERR_GENERAL_UNKNOWN; 
    }
    
    FE_LOG_DEBUG("Dosya basariyla kapatildi. (Handle: %p)", handle);
    return FE_OK;
}

#endif // _WIN32