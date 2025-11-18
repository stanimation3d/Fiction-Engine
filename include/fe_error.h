// include/error/fe_error.h

#ifndef FE_ERROR_H
#define FE_ERROR_H

#include <stdio.h>

/**
 * @brief Fiction Engine (FE) Hata Kodları.
 * * Tüm hata kodları bu numaralandırma (enum) yapısında tanımlanır.
 * Kodlar genellikle FE_COMPONENET_HATA_ADI formatını takip eder.
 */
typedef enum fe_error_code {
    FE_OK = 0,                        // Başarılı işlem
    
    // Genel Sistem Hataları (0 - 99)
    FE_ERR_GENERAL_UNKNOWN = 1,       // Genel/Bilinmeyen Hata
    FE_ERR_MEMORY_ALLOCATION = 2,     // Bellek tahsisi hatası (malloc/realloc başarısız)
    FE_ERR_INVALID_ARGUMENT = 3,      // Fonksiyona geçersiz parametre (NULL veya yanlış değer)
    
    // Raylib (UI/Pencere) Hataları (100 - 199)
    FE_ERR_RL_WINDOW_INIT_FAILED = 100, // Raylib pencere başlatma hatası
    FE_ERR_RL_CONTEXT_CREATION = 101,    // Raylib grafik bağlamı (context) oluşturma hatası
    
    // Render Motoru (Fiction Renderer) Hataları (200 - 299)
    FE_ERR_RENDER_API_INIT_FAILED = 200, // Seçilen Grafik API'sinin (Vulkan/OpenGL/DX/Metal) başlatılamaması
    FE_ERR_SHADER_COMPILATION = 201,    // Shader derleme (compilation) hatası
    FE_ERR_FRAMEBUFFER_CREATION = 202,  // Framebuffer oluşturma hatası
    
    // GeometryV (Sanal Geometri) Hataları (300 - 399)
    FE_ERR_GMV_CLUSTER_FAULT = 300,    // Geometri kümeleme (clustering) hatası
    FE_ERR_GMV_LOD_STREAMING = 301,     // LOD veya geometri verisi akışı (streaming) hatası
    
    // DynamicR (GI/Yansıma) Hataları (400 - 499)
    FE_ERR_DNR_PROBE_UPDATE = 400,      // Aydınlatma probu (light probe) güncelleme hatası
    FE_ERR_DNR_HYBRID_CALC = 401,       // Hibrit GI/Yansıma hesaplama hatası
    
    FE_ERR_COUNT                        // Toplam hata kodu sayısı (Daima son eleman olmalı)
} fe_error_code_t;

/**
 * @brief Bir Fiction Engine hata koduna karşılık gelen okunabilir mesajı döndürür.
 * * @param code Sorgulanacak hata kodu.
 * @return Hata koduna karşılık gelen statik dize (string).
 */
const char* fe_error_get_message(fe_error_code_t code);

/**
 * @brief Hata kodunu konsola yazdırır.
 * * @param code Yazdırılacak hata kodu.
 * @param file Hatanın oluştuğu dosya adı (Genellikle __FILE__ kullanılır).
 * @param line Hatanın oluştuğu satır numarası (Genellikle __LINE__ kullanılır).
 */
void fe_error_print(fe_error_code_t code, const char* file, int line);


// Makro: Hata Kontrolü ve Raporlama için kolay bir yol sunar
#define FE_CHECK(code) do { \
    if ((code) != FE_OK) { \
        fe_error_print((code), __FILE__, __LINE__); \
        /* İstenirse buraya uygulamanın durdurulması eklenebilir. */ \
    } \
} while (0)

#endif // FE_ERROR_H