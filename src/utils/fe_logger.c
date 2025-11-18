// src/utils/fe_logger.c

#include "utils/fe_logger.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h> // exit() için
#include <stdarg.h> // va_list için

// Log dosyasinin adi
#define LOG_FILE_NAME "frontend_engine.log"

// Log durum yapisi
static struct {
    FILE* log_file;
    fe_log_level_t level;
    bool is_initialized;
} g_logger_state = {0};

// Konsol Renk Kodlari (ANSI)
#ifdef _WIN32
    // Windows terminali ANSI kodlarini desteklemeyebilir (Farkli yontemler kullanilabilir), 
    // ancak modern sistemler ve WSL destekler.
    #define COLOR_RESET  "\x1b[0m"
    #define COLOR_FATAL  "\x1b[41;1m" // Beyaz arka plan kirmizi
    #define COLOR_ERROR  "\x1b[31m"   // Kirmizi
    #define COLOR_WARN   "\x1b[33m"   // Sari
    #define COLOR_INFO   "\x1b[37m"   // Beyaz
    #define COLOR_DEBUG  "\x1b[36m"   // Cyan
    #define COLOR_TRACE  "\x1b[34m"   // Mavi
#else 
    // Linux/macOS
    #define COLOR_RESET  "\x1b[0m"
    #define COLOR_FATAL  "\x1b[41;1m" 
    #define COLOR_ERROR  "\x1b[31m"   
    #define COLOR_WARN   "\x1b[33m"   
    #define COLOR_INFO   "\x1b[37m"   
    #define COLOR_DEBUG  "\x1b[36m"   
    #define COLOR_TRACE  "\x1b[34m"   
#endif


/**
 * @brief Log seviyesine göre renk kodunu dondurur.
 */
static const char* fe_logger_get_color(fe_log_level_t level) {
    switch (level) {
        case FE_LOG_LEVEL_FATAL: return COLOR_FATAL;
        case FE_LOG_LEVEL_ERROR: return COLOR_ERROR;
        case FE_LOG_LEVEL_WARN:  return COLOR_WARN;
        case FE_LOG_LEVEL_INFO:  return COLOR_INFO;
        case FE_LOG_LEVEL_DEBUG: return COLOR_DEBUG;
        case FE_LOG_LEVEL_TRACE: return COLOR_TRACE;
        default: return COLOR_RESET;
    }
}

/**
 * @brief Log seviyesine göre etiket dondurur.
 */
static const char* fe_logger_get_label(fe_log_level_t level) {
    switch (level) {
        case FE_LOG_LEVEL_FATAL: return "FATAL";
        case FE_LOG_LEVEL_ERROR: return "ERROR";
        case FE_LOG_LEVEL_WARN:  return "WARN ";
        case FE_LOG_LEVEL_INFO:  return "INFO ";
        case FE_LOG_LEVEL_DEBUG: return "DEBUG";
        case FE_LOG_LEVEL_TRACE: return "TRACE";
        default: return "?????";
    }
}

// ----------------------------------------------------------------------
// 3. YÖNETİM UYGULAMALARI
// ----------------------------------------------------------------------

/**
 * Uygulama: fe_logger_init
 */
void fe_logger_init(void) {
    if (g_logger_state.is_initialized) return;

    // Log dosyasini aç (Yeni bir dosya oluşturmak için "w")
    g_logger_state.log_file = fopen(LOG_FILE_NAME, "w");
    if (g_logger_state.log_file == NULL) {
        // Log dosyasi acilamazsa bile sistem çalışmaya devam etmelidir.
        fprintf(stderr, "HATA: Log dosyasi '%s' olusturulamadi.\n", LOG_FILE_NAME);
    }
    
    // Varsayılan log seviyesini ayarla (Geliştirme için genellikle DEBUG)
    g_logger_state.level = FE_LOG_LEVEL_DEBUG; 
    
    g_logger_state.is_initialized = true;
    
    // İlk mesaj, init fonksiyonu zaten fe_logger_log_message kullandığı için burada direkt fprintf kullanılır.
    if (g_logger_state.log_file) {
        fprintf(g_logger_state.log_file, "[%s] Logger sistemi baslatildi. Seviye: %s\n", 
                fe_logger_get_label(FE_LOG_LEVEL_INFO), fe_logger_get_label(g_logger_state.level));
        fflush(g_logger_state.log_file);
    }
}

/**
 * Uygulama: fe_logger_shutdown
 */
void fe_logger_shutdown(void) {
    if (!g_logger_state.is_initialized) return;

    if (g_logger_state.log_file) {
        // Kapanış mesajı
        fprintf(g_logger_state.log_file, "[%s] Logger sistemi kapatiliyor.\n", 
                fe_logger_get_label(FE_LOG_LEVEL_INFO));
        fclose(g_logger_state.log_file);
        g_logger_state.log_file = NULL;
    }

    g_logger_state.is_initialized = false;
}

/**
 * Uygulama: fe_logger_set_level
 */
void fe_logger_set_level(fe_log_level_t level) {
    if (level < FE_LOG_LEVEL_FATAL || level >= FE_LOG_LEVEL_COUNT) {
        // Geçersiz seviye
        return; 
    }
    g_logger_state.level = level;
}

/**
 * Uygulama: fe_logger_log_message
 */
void fe_logger_log_message(fe_log_level_t level, const char* file, int line, const char* format, ...) {
    // 1. Seviye Kontrolü
    // Mesajın seviyesi, ayarlanan seviyeden daha yüksek önem derecesinde değilse (yani sayısal olarak küçük veya eşit değilse) görmezden gel.
    if (!g_logger_state.is_initialized || level > g_logger_state.level) {
        return;
    }
    
    // 2. Zaman Damgası
    time_t timer;
    char time_buffer[26];
    struct tm* tm_info;
    
    time(&timer);
    tm_info = localtime(&timer);
    // YYYY-MM-DD HH:MM:SS formatı
    strftime(time_buffer, 26, "%Y-%m-%d %H:%M:%S", tm_info);

    // 3. Mesajı biçimlendir (Log dosyası ve Konsol için ayrı ayrı biçimlendirme)
    char buffer[4096];
    int prefix_len;

    // Log Dosyasi Biçimi: [ZAMAN] [SEVİYE] (Dosya:Satır) Mesaj
    prefix_len = snprintf(buffer, sizeof(buffer), "[%s] [%s] (%s:%d) ", 
                          time_buffer, fe_logger_get_label(level), file, line);

    // Değişken argümanları biçimlendir
    va_list args;
    va_start(args, format);
    vsnprintf(buffer + prefix_len, sizeof(buffer) - prefix_len, format, args);
    va_end(args);
    
    // 4. Konsol Çıktısı (Renkli)
    // Renk Kodu + Mesaj + Renk Sıfırlama + Yeni Satır
    fprintf(stdout, "%s%s%s\n", fe_logger_get_color(level), buffer, COLOR_RESET);
    fflush(stdout);

    // 5. Dosya Çıktısı (Renksiz)
    if (g_logger_state.log_file) {
        fprintf(g_logger_state.log_file, "%s\n", buffer);
        fflush(g_logger_state.log_file); // Hemen dosyaya yaz (özellikle kilitlenmeler için kritik)
    }
    
    // 6. Kritik Hata Kontrolü (FATAL)
    if (level == FE_LOG_LEVEL_FATAL) {
        // Kapatma ve sonlandırma işlemini çağır.
        // Motorun çökme/kilitlenme noktasıdır.
        #ifdef FE_DEBUG_BREAK
            FE_DEBUG_BREAK(); // Hata ayıklayıcı bağlıysa dur.
        #endif
        exit(EXIT_FAILURE); 
    }
}
