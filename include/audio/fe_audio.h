// include/audio/fe_audio.h

#ifndef FE_AUDIO_H
#define FE_AUDIO_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// OpenAL Kütüphanesi dahil edilir.
// Sisteme bağlı olarak OpenAL.h veya al.h/alc.h kullanılabilir.
// Genellikle platform bağımsızlığı için OpenAL/al.h kullanılır.
#include <AL/al.h>
#include <AL/alc.h>

// ----------------------------------------------------------------------
// 1. YAPILAR VE TİPLER
// ----------------------------------------------------------------------

/**
 * @brief Tek bir ses klibini (buffer) temsil eder.
 */
typedef struct fe_sound {
    ALuint buffer_id;       // OpenAL Buffer ID'si
    float duration;         // Sesin süresi (saniye)
    uint32_t channels;      // Kanal sayısı (Mono/Stereo)
    // Gelecekte dosya adı, yüklenme durumu gibi bilgiler eklenebilir.
} fe_sound_t;

/**
 * @brief Ses çalan bir örneği (source) temsil eder.
 */
typedef struct fe_audio_source {
    ALuint source_id;       // OpenAL Source ID'si
    fe_sound_t* sound;      // Kaynağın çaldığı fe_sound_t'ye işaretçi
} fe_audio_source_t;


// ----------------------------------------------------------------------
// 2. SİSTEM YÖNETİMİ
// ----------------------------------------------------------------------

/**
 * @brief OpenAL ses sistemini baslatir ve varsayilan dinleyiciyi (Listener) ayarlar.
 * @return Başarılıysa true, degilse false.
 */
bool fe_audio_init(void);

/**
 * @brief OpenAL ses sistemini kapatir ve kaynaklari serbest birakir.
 */
void fe_audio_shutdown(void);

// ----------------------------------------------------------------------
// 3. SES YÜKLEME VE SERBEST BIRAKMA
// ----------------------------------------------------------------------

/**
 * @brief Dosyadan bir ses klibi yükler ve bir fe_sound_t yapısı olusturur.
 * * NOT: Bu fonksiyonun içi, dosya yükleme (wav/ogg/mp3) için harici bir
 * * kütüphane (sndfile, stb_vorbis vb.) gerektirecektir. fe_sound_t sadece
 * * AL buffer'i doldurmakla sorumludur.
 * @param file_path Yüklenecek ses dosyasının yolu.
 * @return fe_sound_t* veya NULL.
 */
fe_sound_t* fe_sound_load(const char* file_path);

/**
 * @brief Bir ses klibini (fe_sound_t) bellekten ve OpenAL'dan serbest birakir.
 */
void fe_sound_destroy(fe_sound_t* sound);

// ----------------------------------------------------------------------
// 4. SES KAYNAĞI YÖNETİMİ (3D/Pozisyonel Ses)
// ----------------------------------------------------------------------

/**
 * @brief Yeni bir ses kaynagi olusturur.
 */
fe_audio_source_t* fe_source_create(fe_sound_t* sound, bool loop);

/**
 * @brief Belirtilen kaynagi (source) oynatir.
 */
void fe_source_play(fe_audio_source_t* source);

/**
 * @brief Belirtilen kaynagi durdurur.
 */
void fe_source_stop(fe_audio_source_t* source);

/**
 * @brief Belirtilen kaynagi yok eder.
 */
void fe_source_destroy(fe_audio_source_t* source);

#endif // FE_AUDIO_H
