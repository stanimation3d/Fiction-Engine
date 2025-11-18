// include/input/fe_input.h

#ifndef FE_INPUT_H
#define FE_INPUT_H

#include <stdint.h>
#include <stdbool.h>

// ----------------------------------------------------------------------
// 1. GİRDİ KODLARI (Sanal Tuş Kodları)
// ----------------------------------------------------------------------

// Klavye Tuş Kodları (Yaygin kullanılanlar)
typedef enum fe_key_code {
    FE_KEY_UNKNOWN = 0,
    FE_KEY_W,
    FE_KEY_A,
    FE_KEY_S,
    FE_KEY_D,
    FE_KEY_SPACE,
    FE_KEY_LSHIFT,
    FE_KEY_ESCAPE,
    FE_KEY_COUNT // Toplam tuş sayisi (Diziler için kullanilir)
} fe_key_code_t;

// Fare Düğme Kodları
typedef enum fe_mouse_button_code {
    FE_MOUSE_BUTTON_LEFT = 0,
    FE_MOUSE_BUTTON_RIGHT,
    FE_MOUSE_BUTTON_MIDDLE,
    FE_MOUSE_BUTTON_COUNT // Toplam fare düğme sayisi
} fe_mouse_button_code_t;


// ----------------------------------------------------------------------
// 2. GİRİŞ DURUM YAPISI
// ----------------------------------------------------------------------

/**
 * @brief Bir tuşun veya düğmenin durumunu temsil eder.
 */
typedef struct fe_button_state {
    bool is_down; // Şu anda basili mi?
    bool was_down; // Önceki karede basili miydi?
} fe_button_state_t;

// ----------------------------------------------------------------------
// 3. GİRİŞ YÖNETİMİ VE SORGULAMA FONKSİYONLARI
// ----------------------------------------------------------------------

/**
 * @brief Giriş sistemini baslatir ve durumu sifirlar.
 */
void fe_input_init(void);

/**
 * @brief Giriş sistemini kapatir.
 */
void fe_input_shutdown(void);

/**
 * @brief Yeni kareye hazirlanir. Önceki durum (was_down) guncellenir.
 * * Her render karesinin basinda çagrilmalidir.
 */
void fe_input_begin_frame(void);

/**
 * @brief Bir klavye tuşunun su anda basili olup olmadigini sorgular.
 */
bool fe_input_is_key_down(fe_key_code_t key);

/**
 * @brief Bir klavye tuşunun bu karede (ilke kez) basili olup olmadigini sorgular.
 * * (Basılıyken tekrar basmaya gerek kalmadan, tek bir tıklama gibi)
 */
bool fe_input_is_key_pressed(fe_key_code_t key);

/**
 * @brief Bir fare dügmesinin su anda basili olup olmadigini sorgular.
 */
bool fe_input_is_mouse_button_down(fe_mouse_button_code_t button);

/**
 * @brief Fare imlecinin mevcut X koordinatini dondurur.
 */
float fe_input_get_mouse_x(void);

/**
 * @brief Fare imlecinin mevcut Y koordinatini dondurur.
 */
float fe_input_get_mouse_y(void);

/**
 * @brief Fare imlecinin son kareden bu yana X eksenindeki degisimini (delta) dondurur.
 */
float fe_input_get_mouse_delta_x(void);

/**
 * @brief Fare imlecinin son kareden bu yana Y eksenindeki degisimini (delta) dondurur.
 */
float fe_input_get_mouse_delta_y(void);


// ----------------------------------------------------------------------
// 4. OLAY İŞLEYİCİLER (Platformdan çagrilir)
// ----------------------------------------------------------------------

/**
 * @brief Klavyeden tuş basilmasi/birakilmasi olayini isler.
 * * @param key Platforma özgü tuş kodu (fe_key_code_t'ye çevrilmiş olmali).
 * @param is_down True ise basma, False ise birakma olayidir.
 */
void fe_input_on_key_event(fe_key_code_t key, bool is_down);

/**
 * @brief Fare dügmesi basilmasi/birakilmasi olayini isler.
 */
void fe_input_on_mouse_button_event(fe_mouse_button_code_t button, bool is_down);

/**
 * @brief Fare imlecinin hareket olayini isler.
 * * @param x, y Yeni mutlak koordinatlar.
 */
void fe_input_on_mouse_move(float x, float y);

#endif // FE_INPUT_H