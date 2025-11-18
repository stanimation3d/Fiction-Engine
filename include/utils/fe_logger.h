// include/utils/fe_logger.h

#ifndef FE_LOGGER_H
#define FE_LOGGER_H

#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>

// ----------------------------------------------------------------------
// 1. LOG SEVİYELERİ
// ----------------------------------------------------------------------

/**
 * @brief Log mesajinin önem derecesini belirtir.
 * * Daha düsük sayi = Daha yüksek önem.
 */
typedef enum fe_log_level {
    FE_LOG_LEVEL_FATAL = 0, // Hemen kilitlenmeyi gerektiren, kurtarilamaz hata.
    FE_LOG_LEVEL_ERROR = 1, // Kurtarilabilir olmayan, ancak kilitlenmeyen kritik hata.
    FE_LOG_LEVEL_WARN  = 2, // Potansiyel sorun, ama su an calismayi engellemeyen durum.
    FE_LOG_LEVEL_INFO  = 3, // Genel bilgi, sistem baslatma/kapatma vb.
    FE_LOG_LEVEL_DEBUG = 4, // Yalnizca hata ayiklama sirasinda kullanilan detayli bilgi.
    FE_LOG_LEVEL_TRACE = 5, // Bir fonksiyona girme/çikma gibi en ince detaylar.
    FE_LOG_LEVEL_COUNT
} fe_log_level_t;


// ----------------------------------------------------------------------
// 2. ANA FONKSİYON PROTOTİPLERİ
// ----------------------------------------------------------------------

/**
 * @brief Loglama sistemini baslatir ve log dosyasini açar.
 */
void fe_logger_init(void);

/**
 * @brief Loglama sistemini kapatir ve log dosyasini serbest birakir.
 */
void fe_logger_shutdown(void);

/**
 * @brief Log seviyesini ayarlar. Yalnizca bu seviyedeki veya daha düsük seviyedeki
 * * mesajlar islenir. (Örn: DEBUG ayarlanirsa, INFO ve altindakiler de islenir.)
 */
void fe_logger_set_level(fe_log_level_t level);

/**
 * @brief Ana loglama fonksiyonu (Mesajlari biçimlendirir ve çiktilara yonlendirir).
 * * Normalde makrolar araciligiyla çagrilir.
 */
void fe_logger_log_message(fe_log_level_t level, const char* file, int line, const char* format, ...);


// ----------------------------------------------------------------------
// 3. KULLANICI ARAYÜZÜ MAKRALARI (Kullanimi kolaylastirir)
// ----------------------------------------------------------------------

#define FE_LOG_FATAL(format, ...) fe_logger_log_message(FE_LOG_LEVEL_FATAL, __FILE__, __LINE__, format, ##__VA_ARGS__)
#define FE_LOG_ERROR(format, ...) fe_logger_log_message(FE_LOG_LEVEL_ERROR, __FILE__, __LINE__, format, ##__VA_ARGS__)
#define FE_LOG_WARN(format, ...)  fe_logger_log_message(FE_LOG_LEVEL_WARN,  __FILE__, __LINE__, format, ##__VA_ARGS__)
#define FE_LOG_INFO(format, ...)  fe_logger_log_message(FE_LOG_LEVEL_INFO,  __FILE__, __LINE__, format, ##__VA_ARGS__)
#define FE_LOG_DEBUG(format, ...) fe_logger_log_message(FE_LOG_LEVEL_DEBUG, __FILE__, __LINE__, format, ##__VA_ARGS__)
#define FE_LOG_TRACE(format, ...) fe_logger_log_message(FE_LOG_LEVEL_TRACE, __FILE__, __LINE__, format, ##__VA_ARGS__)


#endif // FE_LOGGER_H
