// src/error/fe_error.c

#include "error/fe_error.h" // Kendi başlık dosyamızı dahil ediyoruz
#include <stdio.h>
#include <string.h>

/**
 * @brief Hata kodları ve karşılık gelen mesajları içeren statik dizi.
 * * Dizi, fe_error_code enum yapısıyla aynı sırayı takip etmelidir.
 * FE_ERR_COUNT'a kadar olan tüm kodlar için bir mesaj içermelidir.
 */
static const char* fe_error_messages[] = {
    "İşlem Başarılı", // FE_OK
    
    // Genel Sistem Hataları
    "Bilinmeyen Genel Hata", // FE_ERR_GENERAL_UNKNOWN
    "Bellek Tahsisi Başarısız Oldu (Out of Memory)", // FE_ERR_MEMORY_ALLOCATION
    "Geçersiz Fonksiyon Argümanı", // FE_ERR_INVALID_ARGUMENT
    
    // Raylib (UI/Pencere) Hataları
    "Raylib: Pencere Başlatma Başarısız", // FE_ERR_RL_WINDOW_INIT_FAILED
    "Raylib: Grafik Bağlamı (Context) Oluşturulamadı", // FE_ERR_RL_CONTEXT_CREATION
    
    // Render Motoru Hataları
    "Render API Başlatma Başarısız (Vulkan, DX, OpenGL, Metal)", // FE_ERR_RENDER_API_INIT_FAILED
    "Shader Derleme Hatası", // FE_ERR_SHADER_COMPILATION
    "Framebuffer (Çerçeve Tamponu) Oluşturma Hatası", // FE_ERR_FRAMEBUFFER_CREATION
    
    // GeometryV Hataları
    "GeometryV: Geometri Kümeleme (Clustering) Hatası", // FE_ERR_GMV_CLUSTER_FAULT
    "GeometryV: LOD/Veri Akışı (Streaming) Hatası", // FE_ERR_GMV_LOD_STREAMING
    
    // DynamicR Hataları
    "DynamicR: Aydınlatma Probu Güncelleme Hatası", // FE_ERR_DNR_PROBE_UPDATE
    "DynamicR: Hibrit GI/Yansıma Hesaplama Hatası", // FE_ERR_DNR_HYBRID_CALC
    
    // Not: FE_ERR_COUNT için bir mesaj olmasına gerek yoktur, bu yüzden boş bırakılmıştır.
};

/**
 * Uygulama: fe_error_get_message
 * Hata koduna karşılık gelen mesajı döndürür.
 */
const char* fe_error_get_message(fe_error_code_t code) {
    if (code >= 0 && code < FE_ERR_COUNT) {
        // Hata kodu geçerli sınırlar içindeyse mesajı döndür.
        return fe_error_messages[code];
    }
    // Geçersiz bir kod gelirse genel hata mesajını döndür.
    return fe_error_messages[FE_ERR_GENERAL_UNKNOWN];
}

/**
 * Uygulama: fe_error_print
 * Hata kodunu, dosya adını ve satır numarasını standart hata akışına yazdırır.
 */
void fe_error_print(fe_error_code_t code, const char* file, int line) {
    const char* message = fe_error_get_message(code);
    
    // STDERR'a (Standart Hata Akışı) okunabilir formatta hata mesajı yazdırılıyor.
    fprintf(stderr, "FICTION ENGINE HATA [%d]: %s\n", code, message);
    fprintf(stderr, "  Dosya: %s, Satır: %d\n", file, line);
}