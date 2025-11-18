// src/audio/fe_audio.c

#include "audio/fe_audio.h"
#include "utils/fe_logger.h" // Loglama
#include <stdlib.h> // malloc, free
#include <string.h> // memset
#include <math.h>   // logf, powf vb.

// OpenAL baglamina (Context) ait genel degiskenler
static ALCdevice* g_audio_device = NULL;
static ALCcontext* g_audio_context = NULL;

// ----------------------------------------------------------------------
// YARDIMCI FONKSİYONLAR
// ----------------------------------------------------------------------

/**
 * @brief OpenAL hatalarini kontrol eder ve loglar.
 */
static void fe_check_al_error(const char* func_name) {
    ALenum error = alGetError();
    if (error != AL_NO_ERROR) {
        const char* error_str;
        switch (error) {
            case AL_INVALID_NAME: error_str = "AL_INVALID_NAME"; break;
            case AL_INVALID_ENUM: error_str = "AL_INVALID_ENUM"; break;
            case AL_INVALID_VALUE: error_str = "AL_INVALID_VALUE"; break;
            case AL_INVALID_OPERATION: error_str = "AL_INVALID_OPERATION"; break;
            case AL_OUT_OF_MEMORY: error_str = "AL_OUT_OF_MEMORY"; break;
            default: error_str = "Unknown AL Error"; break;
        }
        FE_LOG_ERROR("OpenAL Hatası [%s]: %s", func_name, error_str);
    }
}

// ----------------------------------------------------------------------
// 2. SİSTEM YÖNETİMİ UYGULAMALARI
// ----------------------------------------------------------------------

/**
 * Uygulama: fe_audio_init
 */
bool fe_audio_init(void) {
    // Cihazi ac (Varsayilan cihazi kullan)
    g_audio_device = alcOpenDevice(NULL);
    if (!g_audio_device) {
        FE_LOG_ERROR("fe_audio: Ses cihazi acilamadi.");
        return false;
    }

    // Baglam (Context) olustur
    g_audio_context = alcCreateContext(g_audio_device, NULL);
    if (!g_audio_context) {
        alcCloseDevice(g_audio_device);
        FE_LOG_ERROR("fe_audio: OpenAL baglami olusturulamadi.");
        return false;
    }

    // Baglami aktif yap
    if (!alcMakeContextCurrent(g_audio_context)) {
        alcDestroyContext(g_audio_context);
        alcCloseDevice(g_audio_device);
        FE_LOG_ERROR("fe_audio: OpenAL baglami aktif edilemedi.");
        return false;
    }

    // Dinleyici (Listener) varsayilan ayarları (Pozisyon: 0,0,0)
    alListener3f(AL_POSITION, 0.0f, 0.0f, 0.0f);
    alListener3f(AL_VELOCITY, 0.0f, 0.0f, 0.0f);
    
    // Yönelim (Orientation): {At (x,y,z), Up (x,y,z)}
    float orientation[] = { 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f };
    alListenerfv(AL_ORIENTATION, orientation);
    
    fe_check_al_error("fe_audio_init");
    FE_LOG_INFO("fe_audio: OpenAL başarıyla başlatıldı.");
    return true;
}

/**
 * Uygulama: fe_audio_shutdown
 */
void fe_audio_shutdown(void) {
    if (g_audio_context) {
        alcMakeContextCurrent(NULL);
        alcDestroyContext(g_audio_context);
        g_audio_context = NULL;
    }
    if (g_audio_device) {
        alcCloseDevice(g_audio_device);
        g_audio_device = NULL;
    }
    FE_LOG_INFO("fe_audio: OpenAL başarıyla kapatıldı.");
}

// ----------------------------------------------------------------------
// 3. SES YÜKLEME VE SERBEST BIRAKMA UYGULAMALARI
// ----------------------------------------------------------------------

// NOT: Bu fonksiyon, WAV/OGG dosyasından ham ses verisini okumak için
// harici bir kütüphaneye (örneğin: libsndfile veya stb_vorbis) bağımlıdır.
// Bu implementasyon, sadece AL Buffer oluşturma adımlarını gösterir.

// Örnek: Basitçe Mono, 16-bit, 44100 Hz formatında ses verisinin yüklendiğini varsayalım.
static bool fe_load_dummy_data(ALenum* format, ALsizei* freq, ALsizei* size) {
    // Gerçek uygulamada dosya okuma burada yapılır.
    
    *format = AL_FORMAT_MONO16; // Örnek format
    *freq = 44100;              // Örnek frekans
    *size = 44100 * 2;          // 1 saniyelik 16-bit Mono veri (44100 * 2 byte)

    // Ham ses verisini temsil eden basit bir tampon oluştur
    static short dummy_data[44100]; 
    // memset(dummy_data, 0, *size); // Tamponu sıfırla

    // Basit bir sinüs dalgası da üretilebilir, ancak sadece yer tutucu olarak
    // bu statik tamponu döndürelim.
    
    return true; // Başarılı say
}

/**
 * Uygulama: fe_sound_load
 */
fe_sound_t* fe_sound_load(const char* file_path) {
    fe_sound_t* sound = (fe_sound_t*)calloc(1, sizeof(fe_sound_t));
    if (!sound) return NULL;
    
    ALenum format;
    ALsizei freq, size;
    
    // Dosya okuma simülasyonu
    if (!fe_load_dummy_data(&format, &freq, &size)) {
         FE_LOG_ERROR("fe_sound_load: Dosya okuma basarisiz: %s", file_path);
         free(sound);
         return NULL;
    }
    
    // OpenAL Buffer olustur
    alGenBuffers(1, &sound->buffer_id);
    fe_check_al_error("alGenBuffers");

    // Veriyi AL Buffer'a yükle (Burada dummy_data kullanılır)
    alBufferData(sound->buffer_id, format, (const void*)/*dummy_data*/NULL, size, freq);
    fe_check_al_error("alBufferData");
    
    sound->channels = (format == AL_FORMAT_MONO8 || format == AL_FORMAT_MONO16) ? 1 : 2;
    sound->duration = (float)size / (float)(freq * sound->channels * sizeof(short)); // Yaklaşık süre
    
    FE_LOG_INFO("fe_sound_load: Ses yüklendi: %s (Duration: %.2f s)", file_path, sound->duration);
    return sound;
}

/**
 * Uygulama: fe_sound_destroy
 */
void fe_sound_destroy(fe_sound_t* sound) {
    if (sound) {
        if (sound->buffer_id) {
            alDeleteBuffers(1, &sound->buffer_id);
            fe_check_al_error("alDeleteBuffers");
        }
        free(sound);
    }
}

// ----------------------------------------------------------------------
// 4. SES KAYNAĞI YÖNETİMİ UYGULAMALARI
// ----------------------------------------------------------------------

/**
 * Uygulama: fe_source_create
 */
fe_audio_source_t* fe_source_create(fe_sound_t* sound, bool loop) {
    fe_audio_source_t* source = (fe_audio_source_t*)calloc(1, sizeof(fe_audio_source_t));
    if (!source) return NULL;
    
    source->sound = sound;

    // OpenAL Source olustur
    alGenSources(1, &source->source_id);
    fe_check_al_error("alGenSources");

    // Buffer'i Source'a ata
    alSourcei(source->source_id, AL_BUFFER, sound->buffer_id);
    
    // Varsayilan ayarlar
    alSourcef(source->source_id, AL_PITCH, 1.0f);
    alSourcef(source->source_id, AL_GAIN, 1.0f);
    alSourcei(source->source_id, AL_LOOPING, loop ? AL_TRUE : AL_FALSE);
    alSource3f(source->source_id, AL_POSITION, 0.0f, 0.0f, 0.0f);
    alSource3f(source->source_id, AL_VELOCITY, 0.0f, 0.0f, 0.0f);
    
    fe_check_al_error("fe_source_create");
    return source;
}

/**
 * Uygulama: fe_source_play
 */
void fe_source_play(fe_audio_source_t* source) {
    if (source) {
        alSourcePlay(source->source_id);
        fe_check_al_error("alSourcePlay");
    }
}

/**
 * Uygulama: fe_source_stop
 */
void fe_source_stop(fe_audio_source_t* source) {
    if (source) {
        alSourceStop(source->source_id);
        fe_check_al_error("alSourceStop");
    }
}

/**
 * Uygulama: fe_source_destroy
 */
void fe_source_destroy(fe_audio_source_t* source) {
    if (source) {
        alDeleteSources(1, &source->source_id);
        fe_check_al_error("alDeleteSources");
        free(source);
    }
}
