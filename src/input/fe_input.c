// src/input/fe_input.c

#include "input/fe_input.h"
#include "utils/fe_logger.h" // Hata ayiklama için
#include <string.h> // memcpy için
#include <math.h>   // Basit matematik için

// ----------------------------------------------------------------------
// 1. GLOBAL GİRİŞ DURUMU
// ----------------------------------------------------------------------

static struct {
    // Klavye Durumu
    fe_button_state_t key_states[FE_KEY_COUNT];
    
    // Fare Durumu
    fe_button_state_t mouse_button_states[FE_MOUSE_BUTTON_COUNT];
    float mouse_x, mouse_y;         // Mutlak mevcut konum
    float prev_mouse_x, prev_mouse_y; // Önceki karedeki konum
    float delta_x, delta_y;         // Son kareden bu yana değişim
    
    bool is_initialized;
} g_input_state = {0};


// ----------------------------------------------------------------------
// 2. TEMEL YÖNETİM UYGULAMALARI
// ----------------------------------------------------------------------

/**
 * Uygulama: fe_input_init
 */
void fe_input_init(void) {
    if (g_input_state.is_initialized) return;
    
    // Tüm durumları sıfırla
    memset(g_input_state.key_states, 0, sizeof(g_input_state.key_states));
    memset(g_input_state.mouse_button_states, 0, sizeof(g_input_state.mouse_button_states));
    g_input_state.mouse_x = g_input_state.mouse_y = 0.0f;
    g_input_state.prev_mouse_x = g_input_state.prev_mouse_y = 0.0f;
    g_input_state.delta_x = g_input_state.delta_y = 0.0f;
    
    g_input_state.is_initialized = true;
    FE_LOG_DEBUG("Giris sistemi baslatildi.");
}

/**
 * Uygulama: fe_input_shutdown
 */
void fe_input_shutdown(void) {
    g_input_state.is_initialized = false;
    FE_LOG_DEBUG("Giris sistemi kapatildi.");
}

/**
 * Uygulama: fe_input_begin_frame
 */
void fe_input_begin_frame(void) {
    if (!g_input_state.is_initialized) return;

    // 1. Önceki tuş durumunu güncelle (is_down -> was_down)
    for (int i = 0; i < FE_KEY_COUNT; ++i) {
        g_input_state.key_states[i].was_down = g_input_state.key_states[i].is_down;
    }
    for (int i = 0; i < FE_MOUSE_BUTTON_COUNT; ++i) {
        g_input_state.mouse_button_states[i].was_down = g_input_state.mouse_button_states[i].is_down;
    }

    // 2. Fare delta'sini hesapla ve sıfırla
    g_input_state.delta_x = g_input_state.mouse_x - g_input_state.prev_mouse_x;
    g_input_state.delta_y = g_input_state.mouse_y - g_input_state.prev_mouse_y;
    
    // Yeni kareden sonra mevcut konumu "önceki konum" olarak ayarla
    g_input_state.prev_mouse_x = g_input_state.mouse_x;
    g_input_state.prev_mouse_y = g_input_state.mouse_y;
}


// ----------------------------------------------------------------------
// 3. OLAY İŞLEYİCİLER (PLATFORM)
// ----------------------------------------------------------------------

/**
 * Uygulama: fe_input_on_key_event
 */
void fe_input_on_key_event(fe_key_code_t key, bool is_down) {
    if (!g_input_state.is_initialized || key >= FE_KEY_COUNT) return;
    
    // Yalnızca durum değiştiğinde güncelle
    if (g_input_state.key_states[key].is_down != is_down) {
        g_input_state.key_states[key].is_down = is_down;
        // FE_LOG_TRACE("Key %d: %s", key, is_down ? "DOWN" : "UP");
    }
}

/**
 * Uygulama: fe_input_on_mouse_button_event
 */
void fe_input_on_mouse_button_event(fe_mouse_button_code_t button, bool is_down) {
    if (!g_input_state.is_initialized || button >= FE_MOUSE_BUTTON_COUNT) return;
    
    // Yalnızca durum değiştiğinde güncelle
    if (g_input_state.mouse_button_states[button].is_down != is_down) {
        g_input_state.mouse_button_states[button].is_down = is_down;
        // FE_LOG_TRACE("Mouse Btn %d: %s", button, is_down ? "DOWN" : "UP");
    }
}

/**
 * Uygulama: fe_input_on_mouse_move
 */
void fe_input_on_mouse_move(float x, float y) {
    if (!g_input_state.is_initialized) return;

    // Mutlak konumu doğrudan kaydet. Delta hesaplaması begin_frame'de yapılır.
    g_input_state.mouse_x = x;
    g_input_state.mouse_y = y;
}


// ----------------------------------------------------------------------
// 4. SORGULAMA ARAYÜZLERİ
// ----------------------------------------------------------------------

/**
 * Uygulama: fe_input_is_key_down
 */
bool fe_input_is_key_down(fe_key_code_t key) {
    if (!g_input_state.is_initialized || key >= FE_KEY_COUNT) return false;
    return g_input_state.key_states[key].is_down;
}

/**
 * Uygulama: fe_input_is_key_pressed
 * Basma (Press) = Bu karede basili OLMALI ve önceki karede basili OLMAMALI.
 */
bool fe_input_is_key_pressed(fe_key_code_t key) {
    if (!g_input_state.is_initialized || key >= FE_KEY_COUNT) return false;
    
    fe_button_state_t state = g_input_state.key_states[key];
    return state.is_down && !state.was_down;
}

/**
 * Uygulama: fe_input_is_mouse_button_down
 */
bool fe_input_is_mouse_button_down(fe_mouse_button_code_t button) {
    if (!g_input_state.is_initialized || button >= FE_MOUSE_BUTTON_COUNT) return false;
    return g_input_state.mouse_button_states[button].is_down;
}

/**
 * Uygulama: fe_input_get_mouse_x
 */
float fe_input_get_mouse_x(void) {
    return g_input_state.mouse_x;
}

/**
 * Uygulama: fe_input_get_mouse_y
 */
float fe_input_get_mouse_y(void) {
    return g_input_state.mouse_y;
}

/**
 * Uygulama: fe_input_get_mouse_delta_x
 */
float fe_input_get_mouse_delta_x(void) {
    // Delta, fe_input_begin_frame'de hesaplanır.
    return g_input_state.delta_x;
}

/**
 * Uygulama: fe_input_get_mouse_delta_y
 */
float fe_input_get_mouse_delta_y(void) {
    // Delta, fe_input_begin_frame'de hesaplanır.
    return g_input_state.delta_y;
}
